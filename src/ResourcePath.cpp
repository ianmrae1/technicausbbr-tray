/*
 * ResourcePath.cpp
 *
 *  Created on: Oct 9, 2017
 *      Author: ecos
 */

#include "ResourcePath.h"
#include "build_config.h"

#include <cstdlib>
#include <cstring>

#include <dirent.h>
#include <unistd.h>
#include <libgen.h>
#include <sys/types.h>
#include <sys/stat.h>


using namespace std;

namespace te {
namespace usbbr {

ResourcePath::ResourcePath() {}

std::string ResourcePath::findIcons() {
    const char* teIconPath = XDG_ICONS_PATH "/hicolor/16x16/apps/technica-engineering.png";
    if (access(teIconPath, R_OK) != -1) {
        string path (XDG_ICONS_PATH);
        return path;
    }

    char exePath[PATH_MAX];
    ssize_t count = readlink("/proc/self/exe", exePath, PATH_MAX);

    if (count != -1) {
        string localPath (dirname(exePath));
        localPath += "/../resource/icons/technica-engineering";
        return localPath;
    }

    return "../resource/icons/technica-engineering";
}

std::string ResourcePath::findConfig() {
    string path;
    char *globalPath;
    if ((globalPath = getenv("XDG_CONFIG_HOME")) != NULL) {
        path = string(globalPath);
    } else if ((globalPath = getenv("HOME")) != NULL) {
        path = string(globalPath) + "/.config";
    } else {
        return "";
    }
    path += "/" PROJECT_NAME;

    return path;
}

void ResourcePath::mkdirs(std::string path) {
    char cpath[PATH_MAX];
    memcpy(cpath, path.c_str(), path.size() + 1);
    for (int i = 1; i < path.size(); i++) {
        if (cpath[i] == '/') {
            cpath[i] = 0;
            mkdir(cpath, S_IRWXU | S_IRGRP | S_IXGRP);
            cpath[i] = '/';
        }
    }
}

} /* namespace usbbr */
} /* namespace te */
