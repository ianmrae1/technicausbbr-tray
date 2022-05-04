/*
 * HotplugHandler.h
 *
 *  Created on: Sep 5, 2017
 *      Author: ecos
 */

#ifndef SRC_BRCONVERTER_UTIL_GTK_HOTPLUGHANDLER_H_
#define SRC_BRCONVERTER_UTIL_GTK_HOTPLUGHANDLER_H_

#include <vector>
#include <thread>
#include <condition_variable>
#include <mutex>
#include <list>
#include <utility>

#include <libudev.h>

#include "BRDongle.h"

namespace te {
namespace usbbr {

class HotplugHandler {
public:
    typedef void (*DonglesChangedCallback)(HotplugHandler* handler, void *data);

    HotplugHandler();
    virtual ~HotplugHandler();

    void start();
    void stop();

    bool isRunning() const {
        return isRunning_;
    }

    std::list<BRDongle>& getDongles() {
        return dongles_;
    }

    void addDonglesChangedHandler(DonglesChangedCallback callback, void *data);
    void removeDonglesChangedHandler(DonglesChangedCallback callback, void *data);

private:
    bool isRunning_;
    std::thread pollingThread_;
    bool pollingThreadShouldExit_;
    std::condition_variable pollingThreadWaitCond_;
    std::mutex pollingThreadWaitMutex_;

    std::mutex donglesMutex_;
    std::list<BRDongle> dongles_;

    std::mutex donglesChangedHandlersMutex_;
    std::list<std::pair<DonglesChangedCallback, void *> > donglesChangedHandlers_;

    void pollingThreadMain();
    void pollExisting(struct udev *udev);
    void pollNew(struct udev_monitor *monitor, int udevMonitorFd);

    void tryAddDongle(std::string dongleId);
    void tryRemoveDongle(std::string dongleId);
};

} /* namespace usbbr */
} /* namespace te */

#endif /* SRC_BRCONVERTER_UTIL_GTK_HOTPLUGHANDLER_H_ */
