/*
 * BRDongle.h
 *
 *  Created on: Sep 5, 2017
 *      Author: ecos
 */

#ifndef SRC_BRCONVERTER_UTIL_GTK_BRDONGLE_H_
#define SRC_BRCONVERTER_UTIL_GTK_BRDONGLE_H_

#include <stdint.h>
#include <string>
#include <net/if.h>

namespace te {
namespace usbbr {

class BRDongle {
public:
    enum BrMode {
        BR_MODE_UNKNOWN,
        BR_MODE_MASTER,
        BR_MODE_SLAVE,
    };

    BRDongle();
    BRDongle(std::string dongleId);
    BRDongle(const BRDongle& other);
    BRDongle(BRDongle&& other);
    BRDongle& operator=(const BRDongle& other);
    BRDongle& operator=(BRDongle&& other);

    virtual ~BRDongle();

    bool setMode(BrMode mode);
    BrMode getMode() const {
        return mode_;
    }

    const std::string& getDongleId() const {
        return dongleId_;
    }

    const std::string& getHwAddrUser() const {
        return hwAddrUser_;
    }

    const std::string& getHwAddr() const {
        return hwAddr_;
    }

    bool isValid() const {
        return dongleId_ != "";
    }

private:
    std::string dongleId_, hwAddrUser_, hwAddr_;
    BrMode mode_;
    int socketFd_;

    bool setUpIoctl(struct ifreq &ifr);
    bool performIoctl(int command, struct ifreq &ifr);

    void fetchHwAddr();
    void fetchMode();

    void loadConfig();
    void storeConfig();
    std::string getConfigPath();
};

} /* namespace usbbr */
} /* namespace te */

#endif /* SRC_BRCONVERTER_UTIL_GTK_BRDONGLE_H_ */
