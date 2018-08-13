#include "EMStat.h"

void
EMStat::insert(double val)
{
        double pavg = _avg;
        _avg = _momentum * pavg + (1 - _momentum) * val;

        double pvar = _var;
        _var = _momentum * pvar +
               (1 - _momentum) * (val - pavg) * (val - _avg);
}
