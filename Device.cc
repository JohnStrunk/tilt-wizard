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

#include "Device.h"
#include <cassert>

Device::Device(std::string guidString)
{
    IID guid;
    std::wstring wGuid;
    wGuid.assign(guidString.begin(), guidString.end());
    HRESULT res = IIDFromString(wGuid.c_str(), &guid);
    assert("Error parsing device GUID" && SUCCEEDED(res));

    _init(&guid);
}

Device::Device(IID *guid)
{
    _init(guid);
}

void
Device::_init(IID *guid)
{
    assert("GUID should not be null" && guid);

    HINSTANCE hInst = GetModuleHandle(0);
    assert("Unable to get module handle" && hInst);

    LPDIRECTINPUT8 di8Int = 0;
    HRESULT res = DirectInput8Create(hInst, DIRECTINPUT_VERSION,
                                     IID_IDirectInput8, (LPVOID*)&di8Int,
                                     0);
    assert("Failed to get interface to DirectInput" && SUCCEEDED(res));

    res = di8Int->CreateDevice(*guid, &_dev, 0);
    assert("Unable to get handle for device" && SUCCEEDED(res));

    di8Int->Release();

    // Configure the device
    res = _dev->SetCooperativeLevel(0, DISCL_BACKGROUND | DISCL_NONEXCLUSIVE);
    assert("Failed setting device cooperation level" && SUCCEEDED(res));
    res = _dev->SetDataFormat(&c_dfDIJoystick);
    assert("Failed setting data format" && SUCCEEDED(res));
    // Set to retrieve RAW data from device
    DIPROPDWORD dipdw;
    dipdw.diph.dwSize = sizeof(dipdw);
    dipdw.diph.dwHeaderSize = sizeof(dipdw.diph);
    dipdw.diph.dwObj = 0;
    dipdw.diph.dwHow = DIPH_DEVICE;
    dipdw.dwData = DIPROPCALIBRATIONMODE_RAW;
    res = _dev->SetProperty(DIPROP_CALIBRATIONMODE, &dipdw.diph);
    assert("Failed setting raw data mode" && SUCCEEDED(res));

    // Open the device so we can read
    res = _dev->Acquire();
    assert("Failed acquiring device" && SUCCEEDED(res));
}

Device::~Device()
{
    _dev->Unacquire();
    _dev->Release(); // Needed?
}

std::string
Device::name() const
{
    DIDEVICEINSTANCE di;
    di.dwSize = sizeof(di);
    HRESULT res = _dev->GetDeviceInfo(&di);
    assert("Failed getting device info" && SUCCEEDED(res));
    std::string name(di.tszInstanceName);
    return name;
}

void
Device::poll()
{
    HRESULT res = _dev->Poll();
    assert("Failed polling device" && SUCCEEDED(res));
    res = _dev->GetDeviceState(sizeof(_state), &_state);
    assert("Failed getting current device position" && SUCCEEDED(res));
}

void
Device::calibration(WORD axisOffset, LONG *min, LONG *center, LONG *max) const
{
    DIPROPCAL dical;
    dical.diph.dwSize = sizeof(dical);
    dical.diph.dwHeaderSize = sizeof(dical.diph);
    dical.diph.dwObj = axisOffset;
    dical.diph.dwHow = DIPH_BYOFFSET;
    HRESULT res = _dev->GetProperty(DIPROP_CALIBRATION, &dical.diph);
    assert("Unable to get calibration" && SUCCEEDED(res));
    *min = dical.lMin;
    *center = dical.lCenter;
    *max = dical.lMax;
}

void
Device::calibration(WORD axisOffset, LONG min, LONG center, LONG max)
{
    DIPROPCAL dical;
    dical.diph.dwSize = sizeof(dical);
    dical.diph.dwHeaderSize = sizeof(dical.diph);
    dical.diph.dwObj = axisOffset;
    dical.diph.dwHow = DIPH_BYOFFSET;
    dical.lMin = min;
    dical.lCenter = center;
    dical.lMax = max;
    HRESULT res = _dev->SetProperty(DIPROP_CALIBRATION, &dical.diph);
    assert("Unable to set calibration" && SUCCEEDED(res));

/*
    LPDIRECTINPUTJOYCONFIG idijc = 0;
    res = di8Dev->QueryInterface(IID_IDirectInputJoyConfig,(LPVOID*)&idijc);
    if (FAILED(res)) fatalError(L"Unable to get JoyConfig", res);
    res = idijc->Acquire();
    if (FAILED(res)) fatalError(L"JoyConfig acquire", res);
    res = idijc->SendNotify();
    if (FAILED(res)) fatalError(L"JoyConfig SendNotify", res);
    res = idijc->Release();
    if (FAILED(res)) fatalError(L"JoyConfig Release", res);
*/
}

LONG
Device::position(WORD axisOffset) const
{
    switch (axisOffset) {
    case DIJOFS_X:
        return _state.lX;
    case DIJOFS_Y:
        return _state.lY;
    default:
        assert("Unsupported axis" && 0);
    }

    // not reached
    return 0;
}

double
Device::deadzone(WORD axisOffset) const
{
    DIPROPDWORD dipdw;
    dipdw.diph.dwSize = sizeof(dipdw);
    dipdw.diph.dwHeaderSize = sizeof(dipdw.diph);
    dipdw.diph.dwObj = axisOffset;
    dipdw.diph.dwHow = DIPH_BYOFFSET;
    HRESULT res = _dev->GetProperty(DIPROP_DEADZONE, &dipdw.diph);
    assert("Unable to get deadzone" && SUCCEEDED(res));
    return dipdw.dwData/100.0;
}

void
Device::deadzone(WORD axisOffset, double pct)
{
    assert(0.0 <= pct && pct <= 100.0);
    DIPROPDWORD dipdw;
    dipdw.diph.dwSize = sizeof(dipdw);
    dipdw.diph.dwHeaderSize = sizeof(dipdw.diph);
    dipdw.diph.dwObj = axisOffset;
    dipdw.diph.dwHow = DIPH_BYOFFSET;
    dipdw.dwData = (DWORD)(pct*100.0);
    HRESULT res = _dev->SetProperty(DIPROP_DEADZONE, &dipdw.diph);
    assert("Unable to set deadzone" && SUCCEEDED(res));
}
