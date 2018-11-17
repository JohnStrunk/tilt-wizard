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
    wchar_t guid[100];
    assert(0 != StringFromGUID2(lpddi->guidInstance, guid, 100));
    std::wcout << guid << "    " << lpddi->tszInstanceName << std::endl;
    return DIENUM_CONTINUE;
}

int
enumerateDevices()
{
    HINSTANCE hInst = GetModuleHandle(0);
    if (!hInst) {
        std::cout << "Failed to get module handle" << std::endl;
        return 1;
    }

    LPDIRECTINPUT8 idi = 0;
    HRESULT res = DirectInput8Create(hInst, DIRECTINPUT_VERSION,
                                     IID_IDirectInput8, (LPVOID*)&idi, 0);
    if (res != DI_OK || !idi) {
        std::cout << "Failed to get interface to DirectInput" << std::endl;
        return res;
    }

    // Enumerate devices
    std::cout << std::setw(42) << std::left << "Device GUID"
               << "Name" << std::endl;
    std::cout << std::setw(70) << std::setfill('=') << "=" << std::endl;
    int dummy;
    res = idi->EnumDevices(DI8DEVCLASS_GAMECTRL, devEnumCb, &dummy,
                           DIEDFL_ATTACHEDONLY);
    if (res != DI_OK) {
        std::cout << "Error enumerating devices: " << res << std::endl;
    }

    return res;
}


void
usage(std::string pname)
{
    std::cout << pname << " [device_uuid]" << std::endl
              << std::endl
              << "device_uuid  uuid of device to auto-calibrate" << std::endl;
}


int main(int argc, char *argv[])
{
    // No arguments, so just enumerate the DirectInput devices we find.
    if (argc < 2) {
        return enumerateDevices();
    }

    if (argc == 2) {
        std::string devGuid(argv[1]);
        std::cout << "Looking for device: " << devGuid << std::endl;
        return 0;
    }

    usage(argv[0]);
    return 1;
}
