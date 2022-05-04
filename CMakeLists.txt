cmake_minimum_required(VERSION 3.1)
cmake_policy(VERSION 3.1)
project(technicabr-tray VERSION 1.0.1 LANGUAGES CXX C)

set (CMAKE_MODULE_PATH "${CMAKE_CURRENT_SOURCE_DIR}/cmake" ${CMAKE_MODULE_PATH}) 

set(XDG_DATA_DIR "/usr/share" CACHE PATH "Path to the XDG_DATA_DIR specified by freedesktop.org")
set(XDG_CONFIG_DIR "/etc/xdg" CACHE PATH "Path to the XDG_CONFIG_DIR specified by freedesktop.org")


set(XDG_ICONS_PATH "${XDG_DATA_DIR}/icons")
set(XDG_APPLICATIONS_PATH "${XDG_DATA_DIR}/applications")

set(XDG_AUTOSTART_PATH "${XDG_CONFIG_DIR}/autostart")

add_subdirectory(resource)
add_subdirectory(src)

# uninstall target
configure_file(
    "${CMAKE_CURRENT_SOURCE_DIR}/cmake/cmake_uninstall.cmake.in"
    "${CMAKE_CURRENT_BINARY_DIR}/cmake/cmake_uninstall.cmake"
    IMMEDIATE @ONLY)

add_custom_target(uninstall
    COMMAND ${CMAKE_COMMAND} -P ${CMAKE_CURRENT_BINARY_DIR}/cmake/cmake_uninstall.cmake)
