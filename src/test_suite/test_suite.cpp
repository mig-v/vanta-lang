#include <iostream>

#include "test_suite/test_suite.h"

void TestSuite::run_suite(const std::string& suiteName)
{
    int passed = 0, failed = 0;

    std::cout << "*** " << suiteName << " ***\n";

    for (const auto& test : suite)
    {
        if (test.fn()) { passed++; std::cout << test.name << " ... passed\n"; }
        else { failed++; std::cout << test.name << " ... failed\n"; }
    }

    std::cout << passed << "/" << failed + passed << " tests passed\n\n";
}