// Copyright (c) 2016 Stefan Winkler
// License: MIT License (for full license see LICENSE)

#ifndef _ZENBENCH_H
#define _ZENBENCH_H

#include <algorithm>
#include <chrono>
#include <vector>
#include <iostream>
#include <iomanip>
#include <sstream>

namespace zenbench
{

using BenchmarkList = std::vector<class Benchmark*>;

enum class ContextState
{
    Idle,
    Running,
    AreaBench
};

template <typename Clock>
class BasicContext
{
public:
    BasicContext(std::chrono::nanoseconds duration) : state(ContextState::Idle), duration(duration)
    {}
    
    // Keeps the benchmark running for the duration given in the constructor.
    // Return true as long as there is time left. 
    bool Running()
    {
        if (state == ContextState::AreaBench)
        {
            if (runTime > duration)
            {
                return false;
            }
            iterations++;
            return true;
        }
        if (state == ContextState::Idle)
        {
            state = ContextState::Running;
            iterations = 1;
            start = Clock::now();
            return true;
        }
        auto now = Clock::now();
        runTime = now - start;
        if (runTime >= duration)
        {
            return false;
        }
        
        iterations++;
        return true;
    }

protected:    
    int64_t TimePerIteration(int64_t overhead = 0) const
    {
        auto  perIt = runTime.count()/iterations;
        
        if (state != ContextState::AreaBench)
        {
            perIt -= overhead;
        }
        
        return perIt;
    }

    int64_t Iterations() const
    {
        return iterations;
    }
    
    void BeginArea()
    {
        if (state != ContextState::AreaBench)
        {
            // reset everything set by Running()
            iterations = 1;
            runTime = std::chrono::nanoseconds::zero();  
            state = ContextState::AreaBench; 
        }
        start = Clock::now();
    }
    
    void EndArea()
    {
        auto now = Clock::now();
        runTime += (now - start);
    }
    
private:
    ContextState                                     state;
    int64_t                                          iterations;
    typename Clock::time_point                       start;
    std::chrono::nanoseconds                         duration;
    std::chrono::nanoseconds                         runTime;
    
    friend class BenchmarkArea;
    friend class Benchmark;
#ifdef FRIEND_TEST
    FRIEND_TEST(Context, MeasuresCorrectTime);
    FRIEND_TEST(Context, DetectsBenchmarkArea);
    FRIEND_TEST(Context, CorrectInitialState);
    FRIEND_TEST(Context, CorrectStateAfterFirstRun);
#endif
};

using Context = BasicContext<std::chrono::high_resolution_clock>;


class BenchmarkArea
{
public:
    BenchmarkArea(Context& ctxt) : ctxt(ctxt)
    {
        ctxt.BeginArea();
    }
    
    ~BenchmarkArea()
    {
        ctxt.EndArea();
    }    

private:
    Context&  ctxt;
};

class Benchmark 
{
public:
    static BenchmarkList& List()
    {
        // currently elements in list are never destroyed, but as
        // the lifetime of list is the lifetime of the process, we
        // don't bother using unique_ptrs 
        static BenchmarkList  list;
        return list;
    }
    
    // empty benchmark to measure the overhead of ctxt.Running()
    static void EmptyBenchMark(Context& ctxt)
    {
        while (ctxt.Running());
    }
        
    static void RunAllBenchmarks()
    {
        auto maxLength = MaxNameLength();
        auto nanoLength = std::string("nanoseconds").length();
        auto itLength = std::string("iterations").length();
        
        Context overheadCtxt(std::chrono::seconds(1));
        EmptyBenchMark(overheadCtxt);
        
        std::cout << std::left << std::setw(maxLength) << "name" << "  " << "nanoseconds  iterations" <<  std::endl;
        std::string dashes;
        dashes.insert(0,maxLength + 2 + nanoLength + 2 + itLength,'-');
        std::cout << dashes << std::endl; 
        
        for (auto benchmark : List())
        {
            Context  ctxt(std::chrono::seconds(1));
            benchmark->SetUp();
            benchmark->RunBenchmark(ctxt);
            benchmark->TearDown();
            std::ostringstream ost;
            int64_t timePerIteration = ctxt.TimePerIteration(overheadCtxt.TimePerIteration());
            timePerIteration = std::max<int64_t>(timePerIteration,0ll);
            ost << timePerIteration;
            auto nanoStr = ost.str();
            std::cout << std::left << std::setw(maxLength) << benchmark->Name() << "  ";
            std::cout << std::right << std::setw(nanoLength) << nanoStr;
            std::ostringstream itstrstr;
            itstrstr << ctxt.Iterations();
            auto itStr = itstrstr.str();
            
            std::cout << "  " << std::setw(itLength) << itStr << std::endl;
        }
    }
    
    static size_t MaxNameLength()
    {
        size_t length = 0;
        for (const auto& benchmark : List())
        {
            length = std::max(length,benchmark->Name().length());
        }
        return length;
    }
        
    const std::string& Name() const
    {
        return name;
    }
    
    std::string& Name()
    {
        return name;
    }
    
protected:
    virtual void RunBenchmark(Context& ctxt) {};
    virtual void SetUp() {}
    virtual void TearDown() {}
    
protected:
    std::string name;
};

#define _ZB_CONCATX(A,B) A ## B
#define _ZB_CONCAT(A,B) _ZB_CONCATX(A,B)
#define _ZB_STRX(X) #X
#define _ZB_STR(X) _ZB_STRX(X)

#define BENCHMARK(NAME) \
class NAME : public zenbench::Benchmark \
{ \
public: \
    static class _init \
    { \
    public: \
        _init() \
        { \
            zenbench::Benchmark *bench = new NAME(); \
            bench->Name() = #NAME; \
            NAME::List().push_back(bench); \
        } \
    } _initializer; \
    \
protected: \
    void RunBenchmark(zenbench::Context& ctxt) override; \
}; \
NAME::_init NAME::_initializer; \
\
void NAME::RunBenchmark(zenbench::Context& ctxt)

#define BENCHMARK_F(FIXTURE,NAME) \
class _ZB_CONCAT(FIXTURE,NAME) : public FIXTURE \
{ \
public: \
    static class _init \
    { \
    public: \
        _init() \
        { \
            zenbench::Benchmark *bench = new _ZB_CONCAT(FIXTURE,NAME)(); \
            bench->Name() = _ZB_STR(FIXTURE) "." _ZB_STR(NAME); \
            _ZB_CONCAT(FIXTURE,NAME)::List().push_back(bench); \
        } \
    } _initializer; \
    \
protected: \
    void RunBenchmark(zenbench::Context& ctxt) override; \
}; \
_ZB_CONCAT(FIXTURE,NAME)::_init _ZB_CONCAT(FIXTURE,NAME)::_initializer; \
\
void _ZB_CONCAT(FIXTURE,NAME)::RunBenchmark(zenbench::Context& ctxt)

}  // namespace zenbench

#endif
