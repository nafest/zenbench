#include "zenbench.h"
#include <vector>
#include <algorithm>
#include <cmath>

// second source file to show, that zenbench.h can be included included
// in multiple compilation units

class FloatVector : public zenbench::Benchmark
{
protected:
    virtual void SetUp() override
    {
        zenbench::Benchmark::SetUp();
        
        
        vec.resize(1000);
        // put your initialization code here
        for (int i = 0; i < 1000; i++)
        {
            vec[i] = pow(-1,i)*i;
        }
    }
    
    virtual void TearDown() override
    {
        // put your tear down code here
        
        zenbench::Benchmark::TearDown();
    }
    
protected:
    std::vector<float>  vec;
    
};


BENCHMARK_F(FloatVector,std_sort)
{
    // do something here
    while (ctxt.Running())
    {
        // do something here
        std::sort(vec.begin(),vec.end());
    }
}

extern "C"
int cmp_float(const void *_a, const void *_b)
{
    const float* a = reinterpret_cast<const float*>(a);
    const float* b = reinterpret_cast<const float*>(b);
    
    if (a > b)
      return 1;
    if (a < b)
      return -1;
      
    return 0;
}

BENCHMARK_F(FloatVector,qsort)
{
    // do something here
    while (ctxt.Running())
    {
        // do something here
        qsort(vec.data(),1000,sizeof(float),cmp_float);
    }

}

