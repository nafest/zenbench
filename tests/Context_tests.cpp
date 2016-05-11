#include "gtest/gtest.h"
#include "zenbench.h"
 
#include <thread>

namespace zenbench
{

TEST(Context,MeasuresCorrectTime)
{
    zenbench::Context  overheadCtxt(std::chrono::milliseconds(100));
    while(overheadCtxt.Running());
    auto overhead = overheadCtxt.TimePerIteration(); 
    
    zenbench::Context  ctxt(std::chrono::milliseconds(1));
    
    while (ctxt.Running())
    {
        // active waiting for 25 mikroseconds;
        auto start = std::chrono::high_resolution_clock::now();
        while (std::chrono::high_resolution_clock::now() - start < std::chrono::microseconds(25));
    }
    
    // accept 2% inaccuracy
    EXPECT_GT(500,std::abs(ctxt.TimePerIteration(overhead)-25000));
}

TEST(Context,DetectsBenchmarkArea)
{
    zenbench::Context  ctxt(std::chrono::milliseconds(1));
    
    while (ctxt.Running())
    {
        zenbench::BenchmarkArea ba(ctxt);
    }
    
    EXPECT_TRUE(ctxt.useArea);
}

}  // namespace zenbench