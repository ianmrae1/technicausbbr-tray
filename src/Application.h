/*
 * MenuManager.h
 *
 *  Created on: Oct 6, 2017
 *      Author: ecos
 */

#ifndef SRC_APPLICATION_H_
#define SRC_APPLICATION_H_

#include <list>
#include <gtk/gtk.h>
#include <libappindicator/app-indicator.h>

#include "HotplugHandler.h"

namespace te {
namespace usbbr {

class Application {
public:
    Application();
    Application(HotplugHandler& handler);
    virtual ~Application();

    Application(const Application& other) = delete;
    Application& operator=(const Application& other) = delete;

    int run(int argc, char **argv);

    void close();
private:
    GtkApplication *app_;
    //GtkStatusIcon *statusIcon_;
    AppIndicator *appIndicator_;

    HotplugHandler& handler_;

    GtkWidget* menu_;
    std::list<GtkWidget*> dongleMenuItems_;
    GtkWidget *titleMenuItem_, *donglesSeparatorMenuItem_, *noDonglesMenuItem_, *quitSeparatorMenuItem_, *quitMenuItem_;

    GtkWidget* getMenu();

    static void donglesChangedCallbackStatic(HotplugHandler* handler, void* data);
    void donglesChangedCallback();

    static void gtkAppActivateCallbackStatic(GtkApplication *app, gpointer user_data);
    void gtkAppActivateCallback(GtkApplication *app);

    static void quitActivateCallback(GtkWidget *widget, gpointer data);
};

} /* namespace usbbr */
} /* namespace te */

#endif /* SRC_APPLICATION_H_ */
