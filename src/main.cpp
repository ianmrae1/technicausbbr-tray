/*
 * main.cpp
 *
 *  Created on: Sep 5, 2017
 *      Author: ecos
 */

#include <thread>
#include <chrono>
#include <vector>
#include <list>

#include "Application.h"
#include "HotplugHandler.h"

using namespace te::usbbr;


int main(int argc, char **argv) {
    HotplugHandler handler;
    handler.start();

    Application app(handler);
    int status = app.run(argc, argv);

    handler.stop();

    return status;
}
