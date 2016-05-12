#include "gtest/gtest.h"
#include "zenbench.h"
 
#include <thread>

namespace zenbench
{
    
TEST(Context, CorrectInitialState)
{
    zenbench::Context  ctxt(std::chrono::milliseconds(100));
    
    EXPECT_EQ(std::chrono::milliseconds(100), ctxt.duration);
    EXPECT_EQ(zenbench::ContextState::Idle, ctxt.state);
}

TEST(Context, CorrectStateAfterFirstRun)
{
    zenbench::Context  ctxt(std::chrono::milliseconds(100));
    
    EXPECT_EQ(true, ctxt.Running());
    
    EXPECT_EQ(std::chrono::milliseconds(100), ctxt.duration);
    EXPECT_EQ(zenbench::ContextState::Running, ctxt.state);
    EXPECT_EQ(1, ctxt.iterations);
}

TEST(Context, MeasuresCorrectTime)
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
    
    // accept 5% inaccuracy (for the sake of stability)
    EXPECT_GT(1250, std::abs(ctxt.TimePerIteration(overhead)-25000));
}

TEST(Context, DetectsBenchmarkArea)
{
    zenbench::Context  ctxt(std::chrono::milliseconds(1));
    
    while (ctxt.Running())
    {
        zenbench::BenchmarkArea ba(ctxt);
    }
    
    EXPECT_EQ(zenbench::ContextState::AreaBench, ctxt.state);
}

}  // namespace zenbench