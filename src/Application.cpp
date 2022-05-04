/*
 * MenuManager.cpp
 *
 *  Created on: Oct 6, 2017
 *      Author: ecos
 */

#include "Application.h"
#include "ResourcePath.h"

#include "build_config.h"

#include <iostream>

#include <gtk/gtk.h>
#include <libappindicator/app-indicator.h>

using namespace std;

namespace te {
namespace usbbr {

void Application::donglesChangedCallbackStatic(HotplugHandler* handler, void* data) {
    static_cast<Application*>(data)->donglesChangedCallback();
}

void Application::gtkAppActivateCallbackStatic(GtkApplication *gtkApp, gpointer user_data) {
    Application *app = static_cast<Application*>(user_data);
    app->gtkAppActivateCallback(gtkApp);
}

void Application::gtkAppActivateCallback(GtkApplication *gtkApp) {
    g_application_hold(G_APPLICATION(app_));

    gtk_icon_theme_add_resource_path(gtk_icon_theme_get_default(), ResourcePath::findIcons().c_str());

    appIndicator_ = app_indicator_new_with_path("de.technica-engineering.technicabr-tray", "technica-engineering",
            APP_INDICATOR_CATEGORY_HARDWARE, ResourcePath::findIcons().c_str());

    app_indicator_set_menu(appIndicator_, GTK_MENU(getMenu()));
}

static void setMasterActivateCallback(GtkWidget *widget, gpointer data) {
    BRDongle *dongle = reinterpret_cast<BRDongle*>(data);
    dongle->setMode(BRDongle::BrMode::BR_MODE_MASTER);
}

static void setSlaveActivateCallback(GtkWidget *widget, gpointer data) {
    BRDongle *dongle = reinterpret_cast<BRDongle*>(data);
    dongle->setMode(BRDongle::BrMode::BR_MODE_SLAVE);
}

void Application::quitActivateCallback(GtkWidget *widget, gpointer data) {
    Application *app = static_cast<Application*>(data);
    app->close();
}

Application::Application(HotplugHandler& handler) : handler_(handler), menu_(NULL), app_(NULL),
//        statusIcon_(NULL),
        appIndicator_(NULL),
        noDonglesMenuItem_(NULL), quitSeparatorMenuItem_(NULL), quitMenuItem_(NULL),
        titleMenuItem_(NULL), donglesSeparatorMenuItem_(NULL){}

Application::~Application() {
    if (menu_ != NULL) {
        handler_.removeDonglesChangedHandler(&donglesChangedCallbackStatic, this);
        gtk_widget_destroy(menu_);
    }
    close();
}

GtkWidget* Application::getMenu() {
    if (menu_ == NULL) {
        menu_ = gtk_menu_new();

        titleMenuItem_ = gtk_menu_item_new_with_label("Technica Eng. USB BroadR-Reach converters");
        gtk_widget_set_sensitive(titleMenuItem_, FALSE);
        gtk_menu_shell_append(GTK_MENU_SHELL(menu_), titleMenuItem_);
        gtk_widget_show(titleMenuItem_);

        donglesSeparatorMenuItem_ = gtk_separator_menu_item_new();
        gtk_menu_shell_append(GTK_MENU_SHELL(menu_), donglesSeparatorMenuItem_);
        gtk_widget_show(donglesSeparatorMenuItem_);

        noDonglesMenuItem_ = gtk_menu_item_new_with_label("No converters detected");
        gtk_widget_set_sensitive(noDonglesMenuItem_, FALSE);
        gtk_menu_shell_append(GTK_MENU_SHELL(menu_), noDonglesMenuItem_);

        quitSeparatorMenuItem_ = gtk_separator_menu_item_new();
        gtk_menu_shell_append(GTK_MENU_SHELL(menu_), quitSeparatorMenuItem_);
        gtk_widget_show(quitSeparatorMenuItem_);

        quitMenuItem_ = gtk_menu_item_new_with_label("Quit");
        g_signal_connect(quitMenuItem_, "activate", G_CALLBACK(&quitActivateCallback), this);
        gtk_menu_shell_append(GTK_MENU_SHELL(menu_), quitMenuItem_);
        gtk_widget_show(quitMenuItem_);

        handler_.addDonglesChangedHandler(&donglesChangedCallbackStatic, this);
        donglesChangedCallback();
    }
    return menu_;
}

void Application::donglesChangedCallback() {
    list<BRDongle>& dongles = handler_.getDongles();

    for(auto it = dongleMenuItems_.begin(); it != dongleMenuItems_.end(); ++it) {
        gtk_widget_destroy(*it);
    }
    dongleMenuItems_.clear();

    if (dongles.empty()) {
        gtk_widget_show(noDonglesMenuItem_);
        app_indicator_set_status (appIndicator_, APP_INDICATOR_STATUS_PASSIVE);
    } else {
        gtk_widget_hide(noDonglesMenuItem_);

        int position = 2;
        for(auto it = dongles.begin(); it != dongles.end(); ++it) {
            string label = it->getDongleId() + " (" + it->getHwAddrUser() + ")";
            GtkWidget *item = gtk_menu_item_new_with_label(label.c_str());
            gtk_menu_shell_insert(GTK_MENU_SHELL(menu_), item, position++);

            GtkWidget *submenu = gtk_menu_new();
            GtkWidget *modeTitle, *master, *slave;
            modeTitle = gtk_menu_item_new_with_label("BroadR-Reach Mode");
            gtk_widget_set_sensitive(modeTitle, FALSE);
            gtk_menu_shell_append(GTK_MENU_SHELL(submenu), modeTitle);

            master = gtk_radio_menu_item_new_with_label(NULL, "Master");
            g_signal_connect (master, "activate", G_CALLBACK(setMasterActivateCallback), &(*it));
            gtk_menu_shell_append(GTK_MENU_SHELL(submenu), master);
            slave = gtk_radio_menu_item_new_with_label(gtk_radio_menu_item_get_group(GTK_RADIO_MENU_ITEM(master)), "Slave");
            g_signal_connect (slave, "activate", G_CALLBACK(setSlaveActivateCallback), &(*it));
            gtk_menu_shell_append(GTK_MENU_SHELL(submenu), slave);

            BRDongle::BrMode brMode = it->getMode();
            if (brMode == BRDongle::BR_MODE_MASTER) gtk_check_menu_item_set_active (GTK_CHECK_MENU_ITEM (master), true);
            else if (brMode == BRDongle::BR_MODE_SLAVE) gtk_check_menu_item_set_active (GTK_CHECK_MENU_ITEM (slave), true);

            gtk_menu_item_set_submenu(GTK_MENU_ITEM(item), submenu);

            gtk_widget_show_all(item);

            dongleMenuItems_.push_back(item);
        }
        app_indicator_set_status (appIndicator_, APP_INDICATOR_STATUS_ACTIVE);
    }
}

int Application::run(int argc, char **argv) {
    app_ = gtk_application_new("de.technica-engineering.technicabr-tray", G_APPLICATION_FLAGS_NONE);
    g_signal_connect(app_, "activate", G_CALLBACK(&gtkAppActivateCallbackStatic), this);
    int status = g_application_run(G_APPLICATION(app_), argc, argv);
    g_object_unref(app_);
    app_ = NULL;
    return status;
}

void Application::close() {
    if (app_ != NULL) {
        g_application_release(G_APPLICATION(app_));
        g_application_quit(G_APPLICATION(app_));
    }
}

} /* namespace usbbr */
} /* namespace te */
