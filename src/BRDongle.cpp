/*
 * BRDongle.cpp
 *
 *  Created on: Sep 5, 2017
 *      Author: ecos
 */

#include "BRDongle.h"
#include "ResourcePath.h"

#include <cstring>
#include <iostream>
#include <fstream>
#include <sstream>

#include <unistd.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <sys/ioctl.h>

#include <technicabr.h>

using namespace std;

namespace te {
namespace usbbr {

BRDongle::BRDongle() : mode_(BR_MODE_UNKNOWN), socketFd_(-1) {}

BRDongle::BRDongle(std::string dongleId) : dongleId_(dongleId), mode_(BR_MODE_UNKNOWN) {
    socketFd_ = socket(AF_UNIX,SOCK_DGRAM,0);
    fetchHwAddr();
    fetchMode();

    loadConfig();

    cout << "Detected TE BroadR-Reach USB dongle \"" << dongleId << "\" (MAC: " << hwAddrUser_ << ")" << endl;
}

BRDongle::BRDongle(const BRDongle& other) :
        dongleId_(other.dongleId_), mode_(other.mode_), hwAddr_(other.hwAddr_), hwAddrUser_(other.hwAddrUser_) {
    if (other.socketFd_ == -1) {
        socketFd_ = -1;
    } else {
        socketFd_ = socket(AF_UNIX,SOCK_DGRAM,0);
    }
}

BRDongle::BRDongle(BRDongle&& other) :
        dongleId_(std::move(other.dongleId_)), hwAddr_(std::move(other.hwAddr_)),
        hwAddrUser_(std::move(other.hwAddrUser_)), mode_(other.mode_), socketFd_(other.socketFd_) {
    other.socketFd_ = -1;
}


BRDongle& BRDongle::operator=(const BRDongle& other) {
    dongleId_ = std::move(other.dongleId_);
    hwAddr_ = other.hwAddr_;
    hwAddrUser_ = other.hwAddrUser_;
    mode_ = other.mode_;
    if (other.socketFd_ == -1) {
        socketFd_ = -1;
    } else {
        socketFd_ = socket(AF_UNIX,SOCK_DGRAM,0);
    }

    return *this;
}


BRDongle& BRDongle::operator=(BRDongle&& other) {
    dongleId_ = std::move(other.dongleId_);
    hwAddr_ = std::move(other.hwAddr_);
    hwAddrUser_ = std::move(other.hwAddrUser_);
    mode_ = other.mode_;
    socketFd_ = other.socketFd_;

    other.socketFd_ = -1;

    return *this;
}

BRDongle::~BRDongle() {
    if (socketFd_ > 0) {
        close(socketFd_);
    }
}

bool BRDongle::setUpIoctl(struct ifreq &ifr) {
    if (socketFd_ < 0) return false;

    if (dongleId_.size() >= sizeof(ifr.ifr_name)) {
        cerr << "Dongle ID \"" << dongleId_ << "\" is too long. Cannot set up IOCTL!" << endl;
        return false;
    }
    memcpy(ifr.ifr_name, dongleId_.c_str(), dongleId_.size());
    ifr.ifr_name[dongleId_.size()] = 0;

    return true;
}

bool BRDongle::performIoctl(int command, struct ifreq &ifr) {
    if (ioctl(socketFd_,command,&ifr) == -1) {
        cerr << "Error performing IOCTL for dongle \"" << dongleId_ << "\": " << strerror(errno) << endl;
        return false;
    }
    return true;
}

bool BRDongle::setMode(BrMode mode) {
    struct ifreq ifr;
    setUpIoctl(ifr);

    switch (mode) {
    case BR_MODE_MASTER:
        ifr.ifr_flags = SIOCBASET1_FLAGS_MASTER;
        break;
    case BR_MODE_SLAVE:
        ifr.ifr_flags = SIOCBASET1_FLAGS_SLAVE;
        break;
    default:
        return false;
    }
    if(!performIoctl(SIOCBASET1, ifr)) return false;
    mode_ = mode;
    storeConfig();
    return true;
}

void BRDongle::fetchHwAddr() {
    struct ifreq ifr;
    setUpIoctl(ifr);
    if (!performIoctl(SIOCGIFHWADDR, ifr)) return;
    char addr[3 * 6];
    for (int i = 0; i < 6; i++) {
        sprintf(addr + i * 3, "%.2X", (unsigned char)ifr.ifr_hwaddr.sa_data[i]);
        addr[i*3 + 2] = i == 5 ? 0 : ':';
    }
    hwAddrUser_ = string (addr);

    for (int i = 0; i < 6; i++) {
        sprintf(addr + i * 2, "%.2x", (unsigned char)ifr.ifr_hwaddr.sa_data[i]);
    }
    hwAddr_ = string (addr);
}

void BRDongle::fetchMode() {
    struct ifreq ifr;
    setUpIoctl(ifr);
    ifr.ifr_flags = 0;
    if (!performIoctl(SIOCBASET1, ifr)) return;
    if (ifr.ifr_flags & SIOCBASET1_FLAGS_MASTER) mode_ = BR_MODE_MASTER;
    else if (ifr.ifr_flags & SIOCBASET1_FLAGS_SLAVE) mode_ = BR_MODE_SLAVE;
}

void BRDongle::loadConfig() {
    string path (getConfigPath());
    if (path == "") return;
    ifstream ifs (path);
    string line, param;
    while (getline(ifs, line)) {
        istringstream iss(line);
        iss >> param;
        if (iss.eof()) continue;
        if (param == "mode") {
            int mode;
            iss >> mode;
            setMode(static_cast<BrMode>(mode));
        }
    }
}
void BRDongle::storeConfig() {
    string path (getConfigPath());
    if (path == "") return;
    ofstream ofs (path);
    if (!ofs.is_open()) return;
    ofs << "mode " << static_cast<int>(mode_) << endl;
}
std::string BRDongle::getConfigPath() {
    if (hwAddr_ == "") return "";
    string globalPath = ResourcePath::findConfig();
    if (globalPath == "") return "";
    globalPath += "/dongles/";
    ResourcePath::mkdirs(globalPath);
    return globalPath + hwAddr_;
}

} /* namespace usbbr */
} /* namespace te */
