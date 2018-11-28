/*
 * Tilt-wizard
 * Copyright (C) 2018  John D. Strunk
 *
 * This program is free software: you can redistribute it and/or modify
 * it under the terms of the GNU Affero General Public License as published
 * by the Free Software Foundation, either version 3 of the License, or
 * (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU Affero General Public License for more details.
 *
 * You should have received a copy of the GNU Affero General Public License
 * along with this program.  If not, see <http://www.gnu.org/licenses/>.
 */

#include <iostream>
#include <string>
#include <getopt.h>
#include <iomanip>
#include <unistd.h>

#include "Device.h"

#define _TEXTIFY1(x) #x
#define TEXTIFY(x) _TEXTIFY1(x)

void
readDevice(std::string guidString)
{
    std::cout << "Looking for device: " << guidString << std::endl;
    Device dev(guidString);
    Device cooked(guidString, Device::COOKED);
    std::cout << "Device name: " << dev.name() << std::endl;

    unsigned fwidth = 6;
    std::cout << std::right
              << std::setw(2*fwidth) << "cooked"
              << std::setw(fwidth) << "raw"
              << std::setw(fwidth) << "rmin"
              << std::setw(fwidth) << "min"
              << std::setw(fwidth) << "ctr"
              << std::setw(fwidth) << "max"
              << std::setw(fwidth) << "rmax"
              << std::setw(fwidth) << "DZ%"
              << std::setw(fwidth) << "SAT%"
              << std::setw(2*fwidth) << "cooked"
              << std::setw(fwidth) << "raw"
              << std::setw(fwidth) << "rmin"
              << std::setw(fwidth) << "min"
              << std::setw(fwidth) << "ctr"
              << std::setw(fwidth) << "max"
              << std::setw(fwidth) << "rmax"
              << std::setw(fwidth) << "DZ%"
              << std::setw(fwidth) << "SAT%"
              << std::endl;

    while (true) {
        dev.poll();
        cooked.poll();

        LONG xMin, xCtr, xMax;
        dev.calibration(DIJOFS_X, &xMin, &xCtr, &xMax);
        LONG xrMin, xrMax;
        dev.range(DIJOFS_X, &xrMin, &xrMax);

        LONG yMin, yCtr, yMax;
        dev.calibration(DIJOFS_Y, &yMin, &yCtr, &yMax);
        LONG yrMin, yrMax;
        dev.range(DIJOFS_Y, &yrMin, &yrMax);

        std::cout << std::setprecision(0) << std::fixed;
        std::cout << std::setw(fwidth) << "x: "
                  << std::setw(fwidth) << cooked.position(DIJOFS_X)
                  << std::setw(fwidth) << dev.position(DIJOFS_X)
                  << std::setw(fwidth) << xrMin
                  << std::setw(fwidth) << xMin
                  << std::setw(fwidth) << xCtr
                  << std::setw(fwidth) << xMax
                  << std::setw(fwidth) << xrMax
                  << std::setw(fwidth) << dev.deadzone(DIJOFS_X)
                  << std::setw(fwidth) << dev.saturation(DIJOFS_X)
                  << std::setw(fwidth) << "y: "
                  << std::setw(fwidth) << cooked.position(DIJOFS_Y)
                  << std::setw(fwidth) << dev.position(DIJOFS_Y)
                  << std::setw(fwidth) << yrMin
                  << std::setw(fwidth) << yMin
                  << std::setw(fwidth) << yCtr
                  << std::setw(fwidth) << yMax
                  << std::setw(fwidth) << yrMax
                  << std::setw(fwidth) << dev.deadzone(DIJOFS_Y)
                  << std::setw(fwidth) << dev.saturation(DIJOFS_Y)
                  << "\r";
        usleep(100000);
    }
}


static void
usage(std::string pname)
{
    std::cout << std::endl << "Usage:" << std::endl
              << "  " << pname.c_str() << " -h|--help" << std::endl
              << "  " << pname.c_str() << " -d|--device device_guid" << std::endl
              << std::endl
              << "  -d, --device device_uuid   read from specified device"
              << std::endl
              << "  -h, --help                 this help message"
              << std::endl;
}


 static void
 printVersion()
 {
     std::cout << "direader - https://github.com/JohnStrunk/tilt-wizard"
               << std::endl
               << "License: AGPL v3 or later" << std::endl
               << "Version: " << TEXTIFY(GIT_VERSION) << std::endl
               << "Date: " << TEXTIFY(GIT_DATE) << std::endl << std::endl;
 }


 int main(int argc, char *argv[])
 {
     printVersion();

     std::string devGuid;
     char opt;
     struct option longopts[] = {
         {"device", required_argument, 0, 'd'},
         {"help", no_argument, 0, 'h'},
         {0, 0, 0, 0}
     };
     while ((opt = getopt_long(argc, argv, "d:h", longopts, 0)) != -1) {
         switch (opt) {
         case 'd':
             devGuid = optarg;
             break;
         case 'h':
             usage(argv[0]);
             exit(EXIT_SUCCESS);
             break;
         default: /* '?' */
             usage(argv[0]);
             exit(EXIT_FAILURE);
         }
     }

     if (!devGuid.empty()) {
         readDevice(devGuid);
         return 0;
     } else {
         std::cout << "Must specify one of -d or -h" << std::endl;
         usage(argv[0]);
         exit(EXIT_FAILURE);
     }
 }
