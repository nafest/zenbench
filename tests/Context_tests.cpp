#include "gtest/gtest.h"
#include "zenbench.h"
 
#include <thread>

namespace zenbench
{
    
    
class FakeClock
{
public:
    static std::chrono::high_resolution_clock::time_point now()
    {
        static std::chrono::high_resolution_clock::time_point tp = std::chrono::high_resolution_clock::now();
        tp += std::chrono::nanoseconds(100);
        return tp;
    }
    
    typedef std::chrono::high_resolution_clock::time_point time_point;
};
 
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

using TestContext = BasicContext<FakeClock>;

TEST(Context, MeasuresCorrectTime)
{
    TestContext  ctxt(std::chrono::milliseconds(1));
    
    while (ctxt.Running())
    {}
    
    // the fake clock measures exactly 100ns per iteration
    EXPECT_EQ(100, ctxt.TimePerIteration());
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