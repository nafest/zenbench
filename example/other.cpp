#include "zenbench.h"
#include <vector>
#include <algorithm>
#include <random>
#include <cmath>

// second source file to show that zenbench.h can be included included
// in multiple compilation units

class FloatVector : public zenbench::Benchmark
{
protected:
    virtual void SetUp() override
    {
        zenbench::Benchmark::SetUp();
        std::mt19937  mtrand;
        
        
        vec.resize(size);
        // put your initialization code here
        for (int i = 0; i < size; i++)
        {
            vec[i] = mtrand();
        }
    }
    
    virtual void TearDown() override
    {
        // put your tear down code here
        
        zenbench::Benchmark::TearDown();
    }
    
protected:
    std::vector<float>  vec;
    const int size = 2048;
    
};



extern "C"
int cmp_float(const void *_a, const void *_b)
{
    float a = *(reinterpret_cast<const float*>(_a));
    float b = *(reinterpret_cast<const float*>(_b));
    
    if (a > b)
      return 1;
    if (a < b)
      return -1;
     
    return 0;
}


BENCHMARK_F(FloatVector,qsort)
{
    while (ctxt.Running())
    {
        std::vector<float> workVector(size);
        std::copy(vec.begin(),vec.end(),workVector.begin());
        {
            zenbench::BenchmarkArea benchArea(ctxt);
    
            qsort(workVector.data(),size,sizeof(float),cmp_float);
        }
    }
}

BENCHMARK_F(FloatVector,qsort_no_area)
{
    while (ctxt.Running())
    {
        std::vector<float> workVector(size);
        std::copy(vec.begin(),vec.end(),workVector.begin());
    
        qsort(workVector.data(),size,sizeof(float),cmp_float);
    }
}

BENCHMARK_F(FloatVector,std_sort)
{
    while (ctxt.Running())
    {
        // create a copy of vec in each iteration, to avoid
        // sorting a presorted vector
        std::vector<float> workVector(size);
        std::copy(vec.begin(),vec.end(),workVector.begin());
        {
            // to include only a part of the ctxt.Running() loop 
            // into the benchmark, create a scope with a
            // zenbench::BenchmarkArea object at the top.
            // only operations inside the scope are measured.
            zenbench::BenchmarkArea benchArea(ctxt);
            
            // do something here in this scope
            std::sort(workVector.begin(),workVector.end());
        }
    }
}


BENCHMARK_F(FloatVector,std_sort_no_area)
{
    while (ctxt.Running())
    {
        // create a copy of vec in each iteration, to avoid
        // sorting a presorted vector
        std::vector<float> workVector(size);
        std::copy(vec.begin(),vec.end(),workVector.begin());
            
        // do something here in this scope
        std::sort(workVector.begin(),workVector.end());
    }
}




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
        return false;
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
