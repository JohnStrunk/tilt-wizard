#include <iostream>

#include "EMStat.h"

int
main(int argc, char *argv[])
{
        std::cout << "Hello" << std::endl;

        EMStat s(0.95);
        s.set(0, 0);
        for (unsigned i=0; i<20; ++i) {
                std::cout << s.avg() << " "
                          << s.var() << " "
                          << s.stdev() << std::endl;
                s.insert(0);
        }
}
