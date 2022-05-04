cmake_minimum_required(VERSION 3.1)

set(CONFIG_FILE build_config.h)

set(SOURCE_FILES
	HotplugHandler.cpp
	BRDongle.cpp
	
	Application.cpp
	ResourcePath.cpp
	
	main.cpp
	)

find_package(PkgConfig REQUIRED)
pkg_check_modules(GTK3 REQUIRED gtk+-3.0)

find_package(Threads REQUIRED)

find_package(libudev REQUIRED)

pkg_check_modules(LIBAPPINDICATOR QUIET appindicator3-0.1)
if(NOT LIBAPPINDICATOR_INCLUDE_DIRS)
    pkg_check_modules(LIBAPPINDICATOR QUIET ayatana-appindicator3-0.1)
    add_definitions(-DLIB_AYATANA)
endif()
if(NOT LIBAPPINDICATOR_INCLUDE_DIRS)
    message(FATAL_ERROR "appindicator library not found. Install libappindicator3-dev OR libayatana-appindicator3-dev")
endif()

include_directories(${GTK3_INCLUDE_DIRS})
link_directories(${GTK3_LIBRARY_DIRS})
add_definitions(${GTK3_CFLAGS_OTHER})

include_directories(${LIBAPPINDICATOR_INCLUDE_DIRS})
include_directories(${LIBUDEV_INCLUDE_DIRS})

configure_file(${CONFIG_FILE}.in ${CONFIG_FILE} @ONLY)
include_directories(${CMAKE_CURRENT_BINARY_DIR})

add_executable(${PROJECT_NAME} ${SOURCE_FILES})
set_property(TARGET ${PROJECT_NAME} PROPERTY CXX_STANDARD 11)
set_property(TARGET ${PROJECT_NAME} PROPERTY CXX_STANDARD_REQUIRED ON)

target_link_libraries(${PROJECT_NAME}
	${GTK3_LIBRARIES}
	${LIBAPPINDICATOR_LIBRARIES}
	Threads::Threads
	${LIBUDEV_LIBRARIES})
	
install(TARGETS ${PROJECT_NAME}	DESTINATION bin)
