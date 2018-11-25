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

#include <string>

#include <dinput.h>

class Device {
    /// Pointer to the underlying DirectInput device
    LPDIRECTINPUTDEVICE8 _dev;
    /// Most recently polled state
    DIJOYSTATE _state;


public:
    /**
     * Create a new Device object from its GUID
     */
    Device(IID *guid);
    ~Device();

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
    LONG position(WORD axisOffset) const;
    double deadzone(WORD axisOffset) const;
    void deadzone(WORD axisOffset, double pct);
};

 #endif // DEVICE_H
