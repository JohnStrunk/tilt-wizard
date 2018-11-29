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

 #ifndef DEVICE_H
 #define DEVICE_H

#include <list>
#include <string>

#define DIRECTINPUT_VERSION 0x0800
#include <dinput.h>

class Device {
public:
    enum CalibrationMode { RAW, COOKED };

private:
    /// Pointer to the underlying DirectInput device
    LPDIRECTINPUTDEVICE8 _dev;
    /// Most recently polled state
    DIJOYSTATE _state;

    void _init(IID *guid, CalibrationMode mode);

public:
    /**
     * Create a new Device object from its GUID
     */
    Device(IID *guid, CalibrationMode mode = RAW);
    /// Create a new Device object from its GUID passed as a string
    Device(std::string guidString, CalibrationMode mode = RAW);
    ~Device();

    struct DeviceDescription {
        std::string deviceName;
        std::string guidString;
        IID guid;
    };

    typedef std::list<DeviceDescription> DescriptionList;

    static DescriptionList enumerateDevices();

    /**
     * Get the instance name for the device
     */
    std::string name() const;

    /**
     * Poll the latest state (position) of the device. This will cause all
     * associated Axis objects to get updated values.
     */
    void poll();

    void calibration(WORD axisOffset, LONG *min, LONG *center, LONG *max) const;
    void calibration(WORD axisOffset, LONG min, LONG center, LONG max);
    double deadzone(WORD axisOffset) const;
    void deadzone(WORD axisOffset, double pct);
    LONG position(WORD axisOffset) const;
    void range(WORD axisOffset, LONG min, LONG max);
    void range(WORD axisOffset, LONG *min, LONG *max) const;
    double saturation(WORD axisOffset) const;
    void saturation(WORD axisOffset, double pct);
};

 #endif // DEVICE_H
