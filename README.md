# zenbench - a minimal micro benchmark library
   * Header - only
   * Solely depends on a C++ 11 capable compiler

[![Build Status](https://travis-ci.org/nafest/zenbench.svg?branch=master)](https://travis-ci.org/nafest/zenbench) [![License](https://img.shields.io/badge/license-MIT-blue.svg)](http://opensource.org/licenses/MIT)

### basic usage
All benchmarks must be created and registered with the `BENCHMARK(name)` or the `BENCHMARK_F(fixture,name)` macro:

    BENCHMARK(isPrimeSimple_101)
    {
        while (ctxt.Running())
        {
            // put the code to benchmark here
            isPrimeSimple(101);
        }
    }
    
These macros register a benchmark in a global list and obtain a `zenbench::Context& ctxt` parameter which iterates
over the code to benchmark for a given amount of time.

To execute all benchmarks and print the results to stdout call `zenbench::Benchmark::RunAllBenchmarks()` (in `main`, e.g.).

    int main(int argc, const char* argv[])
    {
        zenbench::Benchmark::RunAllBenchmarks();   
    }
    
### fixture classes
Fixture classes must derive from `zenbench::Benchmark` and may overwrite `SetUp`and `TearDown`   
    
    class Fixture : public zenbench::Benchmark
    {
    protected:
        virtual void SetUp() override
        {
            zenbench::Benchmark::SetUp();
        
            // put your initialization code here
        }
    
        virtual void TearDown() override
        {
            // put your tear down code here
        
            zenbench::Benchmark::TearDown();
        }
    };


