/*
 * ResourcePath.h
 *
 *  Created on: Oct 9, 2017
 *      Author: ecos
 */

#ifndef SRC_RESOURCEPATH_H_
#define SRC_RESOURCEPATH_H_

#include <string>

namespace te {
namespace usbbr {

class ResourcePath {
public:
    static std::string findIcons();
    static std::string findConfig();

    static void mkdirs(std::string path);

private:
    ResourcePath();
};

} /* namespace usbbr */
} /* namespace te */

#endif /* SRC_RESOURCEPATH_H_ */
