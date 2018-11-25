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

#ifndef EMSTAT_H
#define EMSTAT_H

#include <math.h>

/// Calculates exponential moving statistics
class EMStat {
    /// Weight of the existing statistic (0-1.0)
    double _momentum;
    /// Current average
    double _avg;
    /// Current variance
    double _var;
public:
    /// Create a new stat object using the specified momentum
    EMStat(double momentum): _momentum(momentum), _avg(0), _var(0) { }

    /// Reset the statistics to a specified value
    void set(double avg, double var) { _avg = avg; _var = var; }
    /// Insert a new data point
    void insert(double val);

    double avg() const { return _avg; }
    double var() const { return _var; }
    double stdev() const { return sqrt(var()); }
};

#endif // EMSTAT_H
