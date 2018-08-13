#ifndef EMSTAT_H
#define EMSTAT_H

#include <math.h>

class EMStat {
        double _momentum;
        double _avg;
        double _var;
public:
        EMStat(double momentum): _momentum(momentum), _avg(0), _var(0) { }

        void set(double avg, double var) { _avg = avg; _var = var; }
        void insert(double val);

        double avg() const { return _avg; }
        double var() const { return _var; }
        double stdev() const { return sqrt(var()); }
};

#endif // EMSTAT_H
