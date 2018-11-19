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
#include <Windows.h>

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
usage(std::string pname)
{
    std::wcout << std::endl
               << pname.c_str() << " [device_uuid]" << std::endl
               << "  Note: run w/o arguments to scan available devices"
               << std::endl << std::endl
               << "  device_uuid - uuid of device to auto-calibrate"
               << std::endl;
}


static void
printVersion()
{
    std::wcout << "tilt-wizard - https://github.com/JohnStrunk/tilt-wizard"
               << std::endl
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
    std::wcout << "Looking for device: " << devGuid << std::endl;
    IID guid;
    res = IIDFromString(devGuid.c_str(), &guid);
    if (FAILED(res)) fatalError(L"Error parsing device GUID", res);

    LPDIRECTINPUTDEVICE8 di8Dev = 0;
    res = di8Interface->CreateDevice(guid, &di8Dev, 0);
    if (FAILED(res)) fatalError(L"Unable to get handle for device", res);

    DIDEVICEINSTANCE di;
    di.dwSize = sizeof(di);
    res = di8Dev->GetDeviceInfo(&di);
    if (FAILED(res)) fatalError(L"Failed getting device info", res);
    std::wcout << "Device name: " << di.tszInstanceName << std::endl;

    res = di8Dev->SetCooperativeLevel(0, DISCL_BACKGROUND | DISCL_NONEXCLUSIVE);
    if (FAILED(res)) fatalError(L"Failed setting device cooperation level", res);
    res = di8Dev->SetDataFormat(&c_dfDIJoystick);
    if (FAILED(res)) fatalError(L"Failed setting data format", res);

    // retrieve RAW data
    DIPROPDWORD dipdw;
    dipdw.diph.dwSize = sizeof(dipdw);
    dipdw.diph.dwHeaderSize = sizeof(dipdw.diph);
    dipdw.diph.dwObj = 0;
    dipdw.diph.dwHow = DIPH_DEVICE;
    dipdw.dwData = DIPROPCALIBRATIONMODE_RAW;
    //dipdw.dwData = DIPROPCALIBRATIONMODE_COOKED;
    res = di8Dev->SetProperty(DIPROP_CALIBRATIONMODE, &dipdw.diph);
    if (FAILED(res)) fatalError(L"Failed setting calibration mode", res);

    res = di8Dev->Acquire();
    if (FAILED(res)) fatalError(L"Failed acquiring device", res);

    // retrieve the calibration points
    // https://docs.microsoft.com/en-us/previous-versions/windows/desktop/ee419009(v%3dvs.85)
    // Calibration points are no longer supported.
    /*
    DIPROPCPOINTS dipcp;
    dipcp.diph.dwSize = sizeof(dipcp);
    dipcp.diph.dwHeaderSize = sizeof(dipcp.diph);
    dipcp.diph.dwObj = DIJOFS_X;
    dipcp.diph.dwHow = DIPH_BYOFFSET;
    res = di8Dev->GetProperty(DIPROP_CPOINTS, &dipcp.diph);
    if (FAILED(res)) {
        std::wcout << L"Failed getting calibration points " << res << std::endl;
    } else {
        for (unsigned i=0; i<dipcp.dwCPointsNum; ++i) {
            std::wcout << "point " << i << ": "
                       << dipcp.cp[i].lP << " " << dipcp.cp[0].dwLog
                       << std::endl;
        }
    }
    */

    // Retrieve calibration
    DIPROPCAL dical;
    dical.diph.dwSize = sizeof(dical);
    dical.diph.dwHeaderSize = sizeof(dical.diph);
    dical.diph.dwObj = DIJOFS_X;  // x-axis
    dical.diph.dwHow = DIPH_BYOFFSET;
    res = di8Dev->GetProperty(DIPROP_CALIBRATION, &dical.diph);
    if (FAILED(res)) {
        std::wcout << L"Failed getting calibration data " << res << std::endl;
    } else {
        std::wcout << "min: " << dical.lMin
                   << "  center: " << dical.lCenter
                   << "  max: " << dical.lMax
                   << std::endl;
    }

    dipdw.diph.dwObj = DIJOFS_X;
    dipdw.diph.dwHow = DIPH_BYOFFSET;
    res = di8Dev->GetProperty(DIPROP_DEADZONE, &dipdw.diph);
    if (FAILED(res)) fatalError(L"Failed getting deadzone", res);
    std::wcout << "deadzone: " << dipdw.dwData << std::endl;

    sleep(10);

    while (true) {
        res = di8Dev->Poll();
        if (FAILED(res)) fatalError(L"Failed polling current value", res);
        DIJOYSTATE state;
        res = di8Dev->GetDeviceState(sizeof(state), &state);
        if (FAILED(res)) fatalError(L"Failed getting current value", res);
        std::wcout << "x:" << state.lX << " y:" << state.lY << std::endl;
        usleep(100000);
    }
    res = di8Dev->Unacquire();
    if (FAILED(res)) fatalError(L"Failed unacquiring device", res);

    return 0;
}
