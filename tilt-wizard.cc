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
#include <ostream>

#include <cstdio>
#define DIRECTINPUT_VERSION 0x0800
#include <dinput.h>
#include <Windows.h>

BOOL
DevEnumCb(LPCDIDEVICEINSTANCE lpddi, LPVOID pvRef)
{
    std::cout << "device" << std::endl;
    return DIENUM_CONTINUE;
}

//int WINAPI WinMain (HINSTANCE hInstance, HINSTANCE hPrevInstance,
//                    LPSTR lpCmdLine, int nShowCmd)
int main(int argc, char *argv[])
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
        return 1;
    }

    // Enumerate devices
    int dummy;
    res = idi->EnumDevices(DI8DEVCLASS_GAMECTRL, DevEnumCb, &dummy, DIEDFL_ATTACHEDONLY);
    if (res != DI_OK) {
        std::cout << "Error: " << res << std::endl;
    }

    return 0;
}
