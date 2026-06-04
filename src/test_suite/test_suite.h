#pragma once

#include <vector>

#include "test_suite/test.h"

class TestSuite
{
public:
	void run_suite(const std::string& suiteName);

protected:
	virtual void internal_entry() = 0;
	std::vector<Test> suite;
	int passed;
	int failed;
};