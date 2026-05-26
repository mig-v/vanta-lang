#pragma once

#include <vector>

#include "test_suite/test.h"

class TestSuite
{
public:
	void run_suite(const std::string& suiteName);

protected:
	std::vector<Test> suite;
};