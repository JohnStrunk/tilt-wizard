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
