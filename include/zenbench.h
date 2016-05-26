// Copyright (c) 2016 Stefan Winkler
// License: MIT License (for full license see LICENSE)

#ifndef _ZENBENCH_H
#define _ZENBENCH_H

#ifdef _WIN32
#include <windows.h>
#endif

#include <algorithm>
#include <chrono>
#include <vector>
#include <iostream>
#include <iomanip>
#include <sstream>
#include <regex>

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

#ifdef _WIN32
enum class Color
{
    White = 0,
    Green = FOREGROUND_GREEN | FOREGROUND_INTENSITY,
    Yellow = FOREGROUND_RED | FOREGROUND_GREEN | FOREGROUND_INTENSITY,
    Cyan = FOREGROUND_GREEN | FOREGROUND_BLUE | FOREGROUND_INTENSITY
};
#else
enum class Color
{
    White = 0,
    Green = 32,
    Yellow = 33,
    Cyan = 36
};
#endif

class ConsoleModifier
{
public:
    ConsoleModifier(Color c) : code(c) {}
    friend std::ostream&
    operator<<(std::ostream& os, const ConsoleModifier& cm)
    {
#ifdef _WIN32
        SetConsoleTextAttribute(GetStdHandle(STD_OUTPUT_HANDLE),static_cast<WORD>(cm.code));
        return os;
#else
        return os << "\033[" << static_cast<int>(cm.code) << "m";
#endif
    }  
  
private:
    Color  code;
};

class Benchmark 
{
private:
    static std::string FilterArgument(int argc, const char* argv[])
    {
        // iterate over all arguments and find --zenbench_filter=
        std::regex filterRegex {"--zenbench_filter=(.*)"};
        std::smatch baseMatch;
        
        for (int i = 0; i < argc; i++)
        {
            std::string argument { argv[i] };
            if (std::regex_match(argument, baseMatch, filterRegex))
            {
                if (baseMatch.size() == 2)
                {
                    std::ssub_match subMatch = baseMatch[1];
                    std::string filter = subMatch.str();
                    /* replace all occurences of * with .* */
                    filter = std::regex_replace(filter, std::regex("\\*"), ".*");
                    
                    return filter;
                }
            }
        }
        
        return std::string(".*");
    }

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
    
    static void RunAllBenchmarks(int argc = 0, const char* argv[] = nullptr)
    {
        auto maxLength = MaxNameLength();
        auto nanoLength = std::string("nanoseconds").length();
        auto itLength = std::string("iterations").length();
        
        auto filter = FilterArgument(argc, argv);
        
        std::regex filterRegex;
        try 
        {
          filterRegex = std::regex(filter);
        }
        catch (std::regex_error)
        {
            std::cout << "Invalid filter: " << filter << std::endl;
            exit(1);
        }
        
        Context overheadCtxt(std::chrono::seconds(1));
        EmptyBenchMark(overheadCtxt);
        
        std::cout << std::left << std::setw(maxLength) << "name" << "  " << "nanoseconds  iterations" <<  std::endl;
        std::string dashes;
        dashes.insert(0,maxLength + 2 + nanoLength + 2 + itLength,'-');
        std::cout << dashes << std::endl; 
        
        for (auto benchmark : List())
        {
            std::smatch baseMatch;
            if (!std::regex_match(benchmark->Name(), baseMatch, filterRegex))
              continue;
              
            Context  ctxt(std::chrono::seconds(1));
            benchmark->SetUp();
            benchmark->RunBenchmark(ctxt);
            benchmark->TearDown();
            std::ostringstream ost;
            int64_t timePerIteration = ctxt.TimePerIteration(overheadCtxt.TimePerIteration());
            timePerIteration = std::max<int64_t>(timePerIteration,0ll);
            ost << timePerIteration;
            auto nanoStr = ost.str();

            std::cout << ConsoleModifier(Color::Green);
            std::cout << std::left << std::setw(maxLength);
            std::cout << benchmark->Name();
            std::cout << "  ";
           
            std::cout << ConsoleModifier(Color::Yellow);
            std::cout << std::right << std::setw(nanoLength) << nanoStr;

            std::ostringstream itstrstr;
            itstrstr << ctxt.Iterations();
            auto itStr = itstrstr.str();
            
            std::cout << "  ";
            std::cout << ConsoleModifier(Color::Cyan);
            std::cout << std::setw(itLength) << itStr << std::endl;
            std::cout << "\033[0m";
        }
    }
    
    static size_t MaxNameLength()
    {
        size_t length = 0;
        for (const auto& benchmark : List())
        {
            length = benchmark->Name().length() > length ? benchmark->Name().length() : length;
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

#ifdef FRIEND_TEST
    FRIEND_TEST(Benchmark, FilterArgumentWithNoArg);
    FRIEND_TEST(Benchmark, FilterArgumentWithSimpleFilter);
    FRIEND_TEST(Benchmark, FilterArgumentWithWildcards);
#endif
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

#define ZENBENCH_MAIN \
int main(int argc, const char* argv[]) \
{ \
    zenbench::Benchmark::RunAllBenchmarks(argc, argv); \
}


#endif
