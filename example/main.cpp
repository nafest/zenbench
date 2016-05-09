#include "zenbench.h"
#include <iostream>
#include <cmath>


bool isPrimeSimple(int n)
{
    if (n < 0)
    {
        n = -n;
    }
    
    if (n < 3)
    {
        return true;
    }
    
    for (int d = 2; d < n; d++)
    {
        if (n % d == 0)
        {
            return false;
        }
    }
    
    return true;
}

bool isPrimeOpt(int n)
{
    if (n < 0)
    {
        n = -n;
    }
    
    if (n < 3)
    {
        return true;
    }
    
    if (n % 2 == 0)
    {
        return true;
    }
    
    for (int d = 3; d < sqrt(n); d += 2)
    {
        if (n % d == 0)
        {
            return false;
        }
    }
    
    return true;
}

BENCHMARK(isPrimeSimple_101)
{
    while (ctxt.Running())
    {
        isPrimeSimple(101);
    }
}

BENCHMARK(isPrimeOpt_101)
{
    while (ctxt.Running())
    {
        isPrimeOpt(101);
    }
}

BENCHMARK(empty)
{
    while (ctxt.Running())
    {
        // do nothing
    }
}

int main(int argc, const char* argv[])
{
    zenbench::Benchmark::RunAllBenchmarks();   
}