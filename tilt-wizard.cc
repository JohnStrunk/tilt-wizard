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

#define DIRECTINPUT_VERSION 0x0800
#include <dinput.h>
#include <Windows.h>

BOOL
devEnumCb(LPCDIDEVICEINSTANCE lpddi, LPVOID pvRef)
{
    const unsigned G_LEN = 100;
    wchar_t guid[G_LEN];
    assert(0 != StringFromGUID2(lpddi->guidInstance, guid, G_LEN));
    std::wcout << guid << "    " << lpddi->tszInstanceName << std::endl;
    return DIENUM_CONTINUE;
}

int
enumerateDevices()
{
    HINSTANCE hInst = GetModuleHandle(0);
    if (!hInst) {
        std::wcout << "Failed to get module handle" << std::endl;
        return 1;
    }

    LPDIRECTINPUT8 idi = 0;
    HRESULT res = DirectInput8Create(hInst, DIRECTINPUT_VERSION,
                                     IID_IDirectInput8, (LPVOID*)&idi, 0);
    if (res != DI_OK || !idi) {
        std::wcout << "Failed to get interface to DirectInput" << std::endl;
        return res;
    }

    // Enumerate devices
    std::wcout << std::setw(42) << std::left << "Device GUID"
               << "Name" << std::endl;
    std::wcout << std::setw(70) << std::setfill(L'=') << "=" << std::endl;
    int dummy;
    res = idi->EnumDevices(DI8DEVCLASS_GAMECTRL, devEnumCb, &dummy,
                           DIEDFL_ATTACHEDONLY);
    if (res != DI_OK) {
        std::wcout << "Error enumerating devices: " << res << std::endl;
    }

    return res;
}


void
usage(std::string pname)
{
    std::wcout << std::endl
               << pname.c_str() << " [device_uuid]" << std::endl
               << "  Note: run w/o arguments to scan available devices"
               << std::endl << std::endl
               << "  device_uuid - uuid of device to auto-calibrate"
               << std::endl;
}


int main(int argc, char *argv[])
{
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
    HRESULT res = IIDFromString(devGuid.c_str(), &guid);
    if (res != S_OK) {
        std::wcout << "Error parsing device GUID" << std::endl;
        return 1;
    }

    return 0;
}
