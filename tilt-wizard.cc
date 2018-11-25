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

HINSTANCE hInst = 0;
LPDIRECTINPUT8 di8Interface = 0;

static void
fatalError(std::wstring message, HRESULT res) {
    _com_error e(res);
    std::wcout << message << L": " << e.ErrorMessage()
               << L" (" << res << ")" << std::endl;
    abort();
}


BOOL
devEnumCb(LPCDIDEVICEINSTANCE lpddi, LPVOID pvRef)
{
    const unsigned G_LEN = 100;
    wchar_t guid[G_LEN];
    assert(0 != StringFromGUID2(lpddi->guidInstance, guid, G_LEN));
    std::wcout << guid << "    " << lpddi->tszInstanceName << std::endl;
    return DIENUM_CONTINUE;
}

static int
enumerateDevices()
{
    // Enumerate devices
    std::wcout << std::setw(42) << std::left << "Device GUID"
               << "Name" << std::endl;
    std::wcout << std::setw(70) << std::setfill(L'=') << "=" << std::endl;
    int dummy;
    //HRESULT res = di8Interface->EnumDevices(DI8DEVCLASS_ALL, devEnumCb,
    HRESULT res = di8Interface->EnumDevices(DI8DEVCLASS_GAMECTRL, devEnumCb,
                                            &dummy, DIEDFL_ATTACHEDONLY);
    if (FAILED(res)) fatalError(L"Error enumerating devices", res);

    return res;
}

static void
calibrateDevice(std::wstring guidString)
{
    // Turn string into a GUID
    std::wcout << "Looking for device: " << guidString << std::endl;
    IID guid;
    HRESULT res = IIDFromString(guidString.c_str(), &guid);
    if (FAILED(res)) fatalError(L"Error parsing device GUID", res);

    Device dev(&guid);
    std::cout << "Device name: " << dev.name() << std::endl;

    // Grab values once to initialize the stats
    dev.poll();
    sleep(1);  // Let the device start sending data

    /*
     * Create stats objects
     * We take 10 samples/sec, and the new data has weight 1%, so each
     * second is ~10% of the statistic
     */
    EMStat xStats(0.99), yStats(0.99);
    dev.poll();
    xStats.set(dev.position(DIJOFS_X), 100*100);
    yStats.set(dev.position(DIJOFS_Y), 100*100);

    std::cout << std::right
              << std::setw(13) << "current"
              << std::setw(7) << "avg"
              << std::setw(7) << "stdev"
              << std::setw(13) << "current"
              << std::setw(7) << "avg"
              << std::setw(7) << "stdev"
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

        // Update calibration 1/sec
        if (i % 10 == 0) {
            dev.deadzone(DIJOFS_X, std::min(4.0*xStats.stdev(), 100.0));
            dev.calibration(DIJOFS_X, xStats.avg() - 100,
                                      xStats.avg(),
                                      xStats.avg() + 100);
            dev.deadzone(DIJOFS_Y, std::min(4.0*yStats.stdev(), 100.0));
            dev.calibration(DIJOFS_Y, yStats.avg() - 100,
                                      yStats.avg(),
                                      yStats.avg() + 100);
        }

        // Print stats
        std::wcout << std::setprecision(0) << std::fixed;
        std::wcout << "\r    x: " << std::setw(6) << x
                   << " " << std::setw(6) << xStats.avg()
                   << " " << std::setw(6) << xStats.stdev()
                   << "    y: " << std::setw(6) << y
                   << " " << std::setw(6) << yStats.avg()
                   << " " << std::setw(6) << yStats.stdev();
        // Pause 0.1s between samples
        usleep(100000);
    }
}


static void
usage(std::string pname)
{
    std::wcout << std::endl
               << pname.c_str() << " [device_uuid]" << std::endl
               << "  Note: run without arguments to scan available devices"
               << std::endl << std::endl
               << "  device_uuid - uuid of device to auto-calibrate"
               << std::endl;
}


static void
printVersion()
{
    std::wcout << "tilt-wizard - https://github.com/JohnStrunk/tilt-wizard"
               << std::endl
               << "License: AGPL v3 or later" << std::endl
               << "Version: " << TEXTIFY(GIT_VERSION) << std::endl
               << "Date: " << TEXTIFY(GIT_DATE) << std::endl << std::endl;
}


int main(int argc, char *argv[])
{
    printVersion();

    // Initialize module handle instance
    hInst = GetModuleHandle(0);
    if (!hInst) {
        std::wcout << "Failed to get module handle" << std::endl;
        return 1;
    }

    // Initialize DirectInput
    HRESULT res = DirectInput8Create(hInst, DIRECTINPUT_VERSION,
                                     IID_IDirectInput8, (LPVOID*)&di8Interface,
                                     0);
    if (FAILED(res)) fatalError(L"Failed to get interface to DirectInput", res);

    // No arguments, so just enumerate the DirectInput devices we find.
    if (argc < 2) {
        return enumerateDevices();
    }

    // Too many arguments
    if (argc > 2) {
        usage(argv[0]);
        return 1;
    }

    std::string arg(argv[1]);
    std::transform(arg.begin(), arg.end(), arg.begin(), ::toupper);

    // Print help text if needed
    if (0 == arg.compare("/H") ||
        0 == arg.compare("-H") ||
        0 == arg.compare("/HELP") ||
        0 == arg.compare("-HELP") ||
        0 == arg.compare("--HELP")) {
        usage(argv[0]);
        return 0;
    }

    std::wstring devGuid;
    devGuid.assign(arg.begin(), arg.end());
    calibrateDevice(devGuid);
    return 0;
}
