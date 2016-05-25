#include "zenbench.h"
#include <iostream>
#include <cmath>


bool isPrimeOpt(int n);
bool isPrimeSimple(int n);


BENCHMARK(isPrimeOpt_101)
{
    while (ctxt.Running())
    {
        bool ip = isPrimeOpt(101*101);
    }
}

BENCHMARK(isPrimeSimple_101)
{
    while (ctxt.Running())
    {
        bool ip = isPrimeSimple(101*101);
    }
}

BENCHMARK(empty)
{
    while (ctxt.Running())
    {
        // do nothing
    }
}

ZENBENCH_MAIN