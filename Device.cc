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

// Docs: https://docs.microsoft.com/en-us/previous-versions/windows/desktop/ee417929(v%3dvs.85)

#include "Device.h"
#include "TWError.h"

BOOL
devEnumCb(LPCDIDEVICEINSTANCE lpddi, LPVOID voidDev)
{
    Device::DescriptionList *devices =
        reinterpret_cast<Device::DescriptionList*>(voidDev);
    const unsigned G_LEN = 100;
    wchar_t guid[G_LEN];
    if (0 == StringFromGUID2(lpddi->guidInstance, guid, G_LEN))
        throw TWError("Unable to convert GUID to string");
    std::wstring wgstring(guid);
    Device::DeviceDescription desc;
    desc.deviceName = lpddi->tszInstanceName;
    desc.guidString.assign(wgstring.begin(), wgstring.end());
    memcpy(&desc.guid, &lpddi->guidInstance, sizeof(desc.guid));
    devices->push_back(desc);
    return DIENUM_CONTINUE;
}

Device::DescriptionList
Device::enumerateDevices()
{
    // Initialize module handle instance
    HINSTANCE hInst = GetModuleHandle(0);
    if (!hInst) throw TWError("Failed to get module handle");

    // Initialize DirectInput
    LPDIRECTINPUT8 di8Interface = 0;
    HRESULT res = DirectInput8Create(hInst, DIRECTINPUT_VERSION,
                                     IID_IDirectInput8, (LPVOID*)&di8Interface,
                                     0);
    if (FAILED(res)) throw TWError("Failed to get interface to DirectInput", res);

    DescriptionList devices;
    res = di8Interface->EnumDevices(DI8DEVCLASS_GAMECTRL, devEnumCb,
                                    &devices, DIEDFL_ATTACHEDONLY);
    if (FAILED(res)) throw TWError("Error enumerating devices", res);

    return devices;
}


Device::Device(std::string guidString, CalibrationMode mode)
{
    IID guid;
    std::wstring wGuid;
    wGuid.assign(guidString.begin(), guidString.end());
    HRESULT res = IIDFromString(wGuid.c_str(), &guid);
    if (FAILED(res)) throw TWError("Error parsing device GUID", res);

    _init(&guid, mode);
}

Device::Device(IID *guid, CalibrationMode mode)
{
    _init(guid, mode);
}

void
Device::_init(IID *guid, CalibrationMode mode)
{
    if (!guid) throw TWError("GUID should not be null");

    HINSTANCE hInst = GetModuleHandle(0);
    if (!hInst) throw TWError("Unable to get module handle");

    LPDIRECTINPUT8 di8Int = 0;
    HRESULT res = DirectInput8Create(hInst, DIRECTINPUT_VERSION,
                                     IID_IDirectInput8, (LPVOID*)&di8Int,
                                     0);
    if (FAILED(res)) throw TWError("Failed to get interface to DirectInput", res);

    res = di8Int->QueryInterface(IID_IDirectInputJoyConfig8, (LPVOID*)&_dijc8);
    if (FAILED(res)) throw TWError("Failed to get JoyConfig8", res);
    res = _dijc8->SetCooperativeLevel(GetConsoleWindow(), DISCL_BACKGROUND | DISCL_EXCLUSIVE);
    if (FAILED(res)) throw TWError("SetCooperativeLevel", res);
    res = _dijc8->Acquire();
    if (FAILED(res)) throw TWError("Acquire", res);
    res = _dijc8->SendNotify();
    if (FAILED(res)) throw TWError("SendNotify", res);

    res = di8Int->CreateDevice(*guid, &_dev, 0);
    if (FAILED(res)) throw TWError("Unable to get handle for device", res);

    di8Int->Release();

    // Configure the device
    res = _dev->SetCooperativeLevel(0, DISCL_BACKGROUND | DISCL_NONEXCLUSIVE);
    if (FAILED(res)) throw TWError("Failed setting device cooperation level", res);
    res = _dev->SetDataFormat(&c_dfDIJoystick);
    if (FAILED(res)) throw TWError("Failed setting data format", res);
    // Set to retrieve RAW data from device
    DIPROPDWORD dipdw;
    dipdw.diph.dwSize = sizeof(dipdw);
    dipdw.diph.dwHeaderSize = sizeof(dipdw.diph);
    dipdw.diph.dwObj = 0;
    dipdw.diph.dwHow = DIPH_DEVICE;
    if (mode == RAW) {
        dipdw.dwData = DIPROPCALIBRATIONMODE_RAW;
    } else {
        dipdw.dwData = DIPROPCALIBRATIONMODE_COOKED;
    }
    res = _dev->SetProperty(DIPROP_CALIBRATIONMODE, &dipdw.diph);
    if (FAILED(res)) throw TWError("Failed setting calibration mode", res);

    // Open the device so we can read
    res = _dev->Acquire();
    if (FAILED(res)) throw TWError("Failed acquiring device", res);
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
    if (FAILED(res)) throw TWError("Failed getting device info", res);
    std::string name(di.tszInstanceName);
    return name;
}

void
Device::poll()
{
    HRESULT res = _dev->Poll();
    if (FAILED(res)) throw TWError("Failed polling device", res);
    res = _dev->GetDeviceState(sizeof(_state), &_state);
    if (FAILED(res)) throw TWError("Failed getting current device position", res);
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
    if (FAILED(res)) throw TWError("Unable to get calibration", res);
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
    if (FAILED(res)) throw TWError("Unable to set calibration", res);

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
        throw TWError("Unsupported axis");
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
    if (FAILED(res)) throw TWError("Unable to get deadzone", res);
    return dipdw.dwData/100.0;
}

void
Device::deadzone(WORD axisOffset, double pct)
{
    if (0.0 > pct || pct > 100.0)
        throw TWError("Deadzone must be between 0 and 100");
    DIPROPDWORD dipdw;
    dipdw.diph.dwSize = sizeof(dipdw);
    dipdw.diph.dwHeaderSize = sizeof(dipdw.diph);
    dipdw.diph.dwObj = axisOffset;
    dipdw.diph.dwHow = DIPH_BYOFFSET;
    dipdw.dwData = (DWORD)(pct*100.0);
    HRESULT res = _dev->SetProperty(DIPROP_DEADZONE, &dipdw.diph);
    if (FAILED(res)) throw TWError("Unable to set deadzone", res);
}

void
Device::range(WORD axisOffset, LONG min, LONG max)
{
    if (min > max) throw TWError("min must be <= max");
    DIPROPRANGE r;
    r.diph.dwSize = sizeof(r);
    r.diph.dwHeaderSize = sizeof(r.diph);
    r.diph.dwObj = axisOffset;
    r.diph.dwHow = DIPH_BYOFFSET;
    r.lMin = min;
    r.lMax = max;
    HRESULT res = _dev->SetProperty(DIPROP_RANGE, &r.diph);
    if (FAILED(res)) throw TWError("Unable to set axis range", res);
}

void
Device::range(WORD axisOffset, LONG *min, LONG *max) const
{
    DIPROPRANGE r;
    r.diph.dwSize = sizeof(r);
    r.diph.dwHeaderSize = sizeof(r.diph);
    r.diph.dwObj = axisOffset;
    r.diph.dwHow = DIPH_BYOFFSET;
    HRESULT res = _dev->GetProperty(DIPROP_RANGE, &r.diph);
    if (FAILED(res)) throw TWError("Unable to get axis range", res);
    *min = r.lMin;
    *max = r.lMax;
}

double
Device::saturation(WORD axisOffset) const
{
    DIPROPDWORD dipdw;
    dipdw.diph.dwSize = sizeof(dipdw);
    dipdw.diph.dwHeaderSize = sizeof(dipdw.diph);
    dipdw.diph.dwObj = axisOffset;
    dipdw.diph.dwHow = DIPH_BYOFFSET;
    HRESULT res = _dev->GetProperty(DIPROP_SATURATION, &dipdw.diph);
    if (FAILED(res)) throw TWError("Unable to get axis saturation", res);
    return dipdw.dwData/100.0;
}

void
Device::saturation(WORD axisOffset, double pct)
{
    if (0.0 > pct || pct > 100.0)
        throw TWError("Saturation must be between 0 and 100");
    DIPROPDWORD dipdw;
    dipdw.diph.dwSize = sizeof(dipdw);
    dipdw.diph.dwHeaderSize = sizeof(dipdw.diph);
    dipdw.diph.dwObj = axisOffset;
    dipdw.diph.dwHow = DIPH_BYOFFSET;
    dipdw.dwData = (DWORD)(pct*100.0);
    HRESULT res = _dev->SetProperty(DIPROP_SATURATION, &dipdw.diph);
    if (FAILED(res)) throw TWError("Unable to set axis saturation", res);
}
