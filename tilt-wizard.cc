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

#include <algorithm>
#include <cassert>
#include <cstdio>
#include <iomanip>
#include <iostream>
#include <ostream>
#include <string>
#include <getopt.h>
#include <unistd.h>

#define DIRECTINPUT_VERSION 0x0800
#include <comdef.h>
#include <dinput.h>
//#include <dinputd.h>
#include <Windows.h>

#include "Device.h"
#include "EMStat.h"

#ifndef GIT_VERSION
#define GIT_VERSION (unknown)
#endif

#ifndef GIT_DATE
#define GIT_DATE (unknown)
#endif

#define _TEXTIFY1(x) #x
#define TEXTIFY(x) _TEXTIFY1(x)

static const int DEFAULT_RANGE = 100;
static const double DEFAULT_MOMENTUM = 0.95;

static void
fatalError(std::string message, HRESULT res) {
    _com_error e(res);
    std::cout << message << ": " << e.ErrorMessage()
              << " (" << res << ")" << std::endl;
    abort();
}


BOOL
devEnumCb(LPCDIDEVICEINSTANCE lpddi, LPVOID pvRef)
{
    const unsigned G_LEN = 100;
    wchar_t guid[G_LEN];
    assert(0 != StringFromGUID2(lpddi->guidInstance, guid, G_LEN));
    std::wstring wgstring(guid);
    std::string gstring;
    gstring.assign(wgstring.begin(), wgstring.end());
    std::cout << gstring << "    " << lpddi->tszInstanceName << std::endl;
    return DIENUM_CONTINUE;
}

static void
enumerateDevices()
{
    // Initialize module handle instance
    HINSTANCE hInst = GetModuleHandle(0);
    if (!hInst) {
        std::cout << "Failed to get module handle" << std::endl;
        abort();
    }

    // Initialize DirectInput
    LPDIRECTINPUT8 di8Interface = 0;
    HRESULT res = DirectInput8Create(hInst, DIRECTINPUT_VERSION,
                                     IID_IDirectInput8, (LPVOID*)&di8Interface,
                                     0);
    if (FAILED(res)) fatalError("Failed to get interface to DirectInput", res);

    // Enumerate devices
    std::cout << std::setw(42) << std::left << "Device GUID"
               << "Name" << std::endl;
    std::cout << std::setw(70) << std::setfill('=') << "=" << std::endl;
    int dummy;
    //HRESULT res = di8Interface->EnumDevices(DI8DEVCLASS_ALL, devEnumCb,
    res = di8Interface->EnumDevices(DI8DEVCLASS_GAMECTRL, devEnumCb,
                                    &dummy, DIEDFL_ATTACHEDONLY);
    if (FAILED(res)) fatalError("Error enumerating devices", res);
}

static void
calibrateDevice(std::string guidString, int axisRange, double momentum)
{
    std::cout << "Looking for device: " << guidString << std::endl;
    Device dev(guidString);
    std::cout << "Device name: " << dev.name() << std::endl;

    // Grab values once to initialize the stats
    dev.poll();
    sleep(1);  // Let the device start sending data

    /*
     * Create stats objects
     * We take 10 samples/sec, and the new data has weight 1%, so each
     * second is ~10% of the statistic
     */
    EMStat xStats(momentum), yStats(momentum);
    dev.poll();
    xStats.set(dev.position(DIJOFS_X), 100*100);
    yStats.set(dev.position(DIJOFS_Y), 100*100);

    std::cout << std::right
              << std::setw(13) << "current"
              << std::setw(7) << "avg"
              << std::setw(7) << "DZ%"
              << std::setw(13) << "current"
              << std::setw(7) << "avg"
              << std::setw(7) << "DZ%"
              << std::endl;

    for (unsigned i=1; ;++i) {
        dev.poll();

        LONG x = dev.position(DIJOFS_X);
        // Reject extreme values (nudges); ensure we always accept something
        if (abs(x - xStats.avg()) < std::max(2.0*xStats.stdev(), 10.0))
            xStats.insert(x);

        LONG y = dev.position(DIJOFS_Y);
        if (abs(y - yStats.avg()) < std::max(2.0*yStats.stdev(), 10.0))
            yStats.insert(y);

        double xDeadzone = std::min(4.0*xStats.stdev()*100.0/axisRange, 100.0);
        double yDeadzone = std::min(4.0*yStats.stdev()*100.0/axisRange, 100.0);

        // Update calibration 1/sec
        if (i % 10 == 0) {
            dev.deadzone(DIJOFS_X, xDeadzone);
            dev.calibration(DIJOFS_X, xStats.avg() - axisRange,
                                      xStats.avg(),
                                      xStats.avg() + axisRange);
            dev.deadzone(DIJOFS_Y, yDeadzone);
            dev.calibration(DIJOFS_Y, yStats.avg() - axisRange,
                                      yStats.avg(),
                                      yStats.avg() + axisRange);
        }

        // Print stats
        std::cout << std::setprecision(0) << std::fixed;
        std::cout << "\r    x: " << std::setw(6) << x
                  << " " << std::setw(6) << xStats.avg()
                  << " " << std::setw(6) << xDeadzone
                  << "    y: " << std::setw(6) << y
                  << " " << std::setw(6) << yStats.avg()
                  << " " << std::setw(6) << yDeadzone;
        // Pause 0.1s between samples
        usleep(100000);
    }
}


static void
usage(std::string pname)
{
    std::cout << std::endl << "Usage:" << std::endl
              << "  " << pname.c_str() << " -h|--help" << std::endl
              << "  " << pname.c_str() << " -l|--list" << std::endl
              << "  " << pname.c_str() << " -d|--device device_guid  [options...]" << std::endl
              << std::endl
              << "  -d, --device device_uuid   auto-calibrate specified device"
              << std::endl
              << "  -h, --help                 this help message"
              << std::endl
              << "  -l, --list                 list available devices"
              << std::endl
              << "  -m, --momentum             EMA momentum (0 - 1.0)"
              << " (default: " << DEFAULT_MOMENTUM << ")"
              << std::endl
              << "  -r, --range                axis range, center to max"
              << " (default: " << DEFAULT_RANGE << ")"
              << std::endl;
}


static void
printVersion()
{
    std::cout << "tilt-wizard - https://github.com/JohnStrunk/tilt-wizard"
              << std::endl
              << "License: AGPL v3 or later" << std::endl
              << "Version: " << TEXTIFY(GIT_VERSION) << std::endl
              << "Date: " << TEXTIFY(GIT_DATE) << std::endl << std::endl;
}


int main(int argc, char *argv[])
{
    printVersion();

    std::string devGuid;
    int range = DEFAULT_RANGE;
    double momentum = DEFAULT_MOMENTUM;
    char opt;
    struct option longopts[] = {
        {"device", required_argument, 0, 'd'},
        {"help", no_argument, 0, 'h'},
        {"list", no_argument, 0, 'l'},
        {"momentum", required_argument, 0, 'm'},
        {"range", required_argument, 0, 'r'},
        {0, 0, 0, 0}
    };
    while ((opt = getopt_long(argc, argv, "d:hlm:r:", longopts, 0)) != -1) {
        switch (opt) {
        case 'd':
            devGuid = optarg;
            break;
        case 'h':
            usage(argv[0]);
            exit(EXIT_SUCCESS);
            break;
        case 'l':
            enumerateDevices();
            exit(EXIT_SUCCESS);
            break;
        case 'm':
            momentum = atof(optarg);
            break;
        case 'r':
            range = atoi(optarg);
            break;
        default: /* '?' */
            usage(argv[0]);
            exit(EXIT_FAILURE);
        }
    }

    if (momentum < 0 || 1.0 < momentum) {
        std::cout << "Momentum must be in the range 0 - 1.0" << std::endl;
        usage(argv[0]);
        exit(EXIT_FAILURE);
    }

    if (range <= 0) {
        std::cout << "Axis range must be > 0" << std::endl;
        usage(argv[0]);
        exit(EXIT_FAILURE);
    }

    if (!devGuid.empty()) {
        calibrateDevice(devGuid, range, momentum);
        return 0;
    } else {
        std::cout << "Must specify one of -d, -h or -l" << std::endl;
        usage(argv[0]);
        exit(EXIT_FAILURE);
    }
}
