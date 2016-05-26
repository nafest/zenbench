#include "gtest/gtest.h"
#include "zenbench.h"

namespace zenbench
{

TEST(Benchmark, FilterArgumentWithNoArg)
{
    std::string filter = zenbench::Benchmark::FilterArgument(0,nullptr);
    
    EXPECT_EQ(".*", filter);
}

TEST(Benchmark, FilterArgumentWithSimpleFilter)
{
    const char* argv[1] = {"--zenbench_filter=foo"};
    std::string filter = zenbench::Benchmark::FilterArgument(1,argv);
    
    EXPECT_EQ("foo", filter);
}

TEST(Benchmark, FilterArgumentWithWildcards)
{
    const char* argv[1] = {"--zenbench_filter=*foo*"};
    std::string filter = zenbench::Benchmark::FilterArgument(1,argv);
    
    EXPECT_EQ(".*foo.*", filter);
}

}