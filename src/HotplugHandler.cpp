/*
 * HotplugHandler.cpp
 *
 *  Created on: Sep 5, 2017
 *      Author: ecos
 */

#include "HotplugHandler.h"

#include <chrono>
#include <iostream>

#include <cstring>

#include <unistd.h>
#include <libudev.h>

using namespace std;

namespace te {
namespace usbbr {

HotplugHandler::HotplugHandler() : isRunning_(false), pollingThreadShouldExit_(false) {

}

HotplugHandler::~HotplugHandler() {
    stop();
}

void HotplugHandler::start() {
    if (isRunning_) return;
    pollingThreadShouldExit_ = false;
    isRunning_ = true;
    pollingThread_ = thread(&HotplugHandler::pollingThreadMain, this);
}

void HotplugHandler::stop() {
    if (!isRunning_) return;

    {
        unique_lock<mutex> lock(pollingThreadWaitMutex_);
        pollingThreadShouldExit_ = true;
        pollingThreadWaitCond_.notify_one();
    }
    if (pollingThread_.joinable()) {
        pollingThread_.join();
    }

    dongles_.clear();

    isRunning_ = false;
}

void HotplugHandler::pollingThreadMain() {
    struct udev *udev;
    struct udev_monitor *udevMonitor;
    int udevMonitorFd;

    /* create udev object */
    udev = udev_new();
    if (!udev) {
        cerr << "Can't create udev" << endl;
        return;
    }

    udevMonitor = udev_monitor_new_from_netlink(udev, "udev");
    udev_monitor_filter_add_match_subsystem_devtype(udevMonitor, "net", NULL);
    udev_monitor_enable_receiving(udevMonitor);

    // Poll for existing devices *after* enabling the monitor to avoid race conditions
    pollExisting(udev);

    udevMonitorFd = udev_monitor_get_fd(udevMonitor);
    while (!pollingThreadShouldExit_) {
        pollNew(udevMonitor, udevMonitorFd);

        unique_lock<mutex> lock(pollingThreadWaitMutex_);
        pollingThreadWaitCond_.wait_for(lock, chrono::milliseconds(500), [this]() { return pollingThreadShouldExit_; });
    }

    udev_unref(udev);
}

void HotplugHandler::pollExisting(struct udev *udev) {
    struct udev_enumerate *udevEnum = udev_enumerate_new(udev);
    if (!udevEnum) {
        cerr << "Cannot create udev enumerate context." << endl;
        return;
    }

    udev_enumerate_add_match_subsystem(udevEnum, "net");
    udev_enumerate_scan_devices(udevEnum);

    for(struct udev_list_entry *device = udev_enumerate_get_list_entry(udevEnum);
            device != NULL; device = udev_list_entry_get_next(device)) {
        const char *path, *devName, *driverName;
        struct udev_device *dev, *parent;

        path = udev_list_entry_get_name(device);
        dev = udev_device_new_from_syspath(udev, path);

        devName = udev_device_get_sysname(dev);
        string devNameSting (devName);
        parent = udev_device_get_parent(dev);
        if (parent) {
            driverName = udev_device_get_driver(parent);
            if (strcmp(driverName, "technicabr") == 0) {
                tryAddDongle(devNameSting);
            }
        }

        udev_device_unref(dev);
    }

    udev_enumerate_unref(udevEnum);
}

void HotplugHandler::pollNew(struct udev_monitor *monitor, int udevMonitorFd) {
    struct udev_device *dev, *parent;
    fd_set fds;
    struct timeval tv;
    int ret;

    FD_ZERO(&fds);
    FD_SET(udevMonitorFd, &fds);
    tv.tv_sec = 0;
    tv.tv_usec = 0;

    ret = select(udevMonitorFd+1, &fds, NULL, NULL, &tv);
    if (ret > 0 && FD_ISSET(udevMonitorFd, &fds)) {
        const char *devName, *driverName, *action;
        dev = udev_monitor_receive_device(monitor);
        if (dev) {
            devName = udev_device_get_sysname(dev);
            string devNameSting (devName);
            action = udev_device_get_action(dev);
            if (strcmp(action, "add") == 0) {
                parent = udev_device_get_parent(dev);
                if (parent) {
                    driverName = udev_device_get_driver(parent);
                    if (strcmp(driverName, "technicabr") == 0) {
                        tryAddDongle(devNameSting);
                    }
                }
            } else if (strcmp(action, "remove") == 0) {
                tryRemoveDongle(devNameSting);
            }
            /* free dev */
            udev_device_unref(dev);
        }
    }
}

void HotplugHandler::tryAddDongle(std::string dongleId) {
    lock_guard<std::mutex> lock (donglesMutex_);
    bool exists = false;
    for (auto it = dongles_.begin(); it != dongles_.end(); ++it) {
        if (it->getDongleId() == dongleId) {
            exists = true;
            break;
        }
    }
    if (!exists) {
        dongles_.push_back(BRDongle(dongleId));

        lock_guard<mutex> lock (donglesChangedHandlersMutex_);
        for (auto it = donglesChangedHandlers_.begin(); it != donglesChangedHandlers_.end(); ++it) {
            (*it->first)(this, it->second);
        }
    }
}
void HotplugHandler::tryRemoveDongle(std::string dongleId) {
    bool hasChanged = false;

    lock_guard<std::mutex> lock (donglesMutex_);
    for (auto it = dongles_.begin(); it != dongles_.end(); ++it) {
        if (it->getDongleId() == dongleId) {
            dongles_.erase(it);
            hasChanged = true;
            break;
        }
    }
    if (hasChanged) {
        lock_guard<mutex> lock (donglesChangedHandlersMutex_);
        for (auto it = donglesChangedHandlers_.begin(); it != donglesChangedHandlers_.end(); ++it) {
            (*it->first)(this, it->second);
        }
    }
}

void HotplugHandler::addDonglesChangedHandler(DonglesChangedCallback callback, void *data) {
    lock_guard<mutex> lock (donglesChangedHandlersMutex_);
    for (auto it = donglesChangedHandlers_.begin(); it != donglesChangedHandlers_.end(); ++it) {
        if (it->first == callback && it->second == data) return;
    }
    donglesChangedHandlers_.push_back(std::pair<DonglesChangedCallback, void*>(callback, data));
}

void HotplugHandler::removeDonglesChangedHandler(DonglesChangedCallback callback, void *data) {
    lock_guard<mutex> lock (donglesChangedHandlersMutex_);
    for (auto it = donglesChangedHandlers_.begin(); it != donglesChangedHandlers_.end(); ++it) {
        if (it->first == callback && it->second == data) {
            donglesChangedHandlers_.erase(it);
            return;
        }
    }
}

} /* namespace usbbr */
} /* namespace te */
