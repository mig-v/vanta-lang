#pragma once

#include "test_suite/test.h"
#include "test_suite/test_suite.h"

class LexerTestSuite : public TestSuite
{
public:
	LexerTestSuite();

protected:
	void internal_entry() override;
private:
	bool test_literals();
	bool test_keywords();
	bool test_identifiers();
	bool test_assignment();
	bool test_comparison();
	bool test_arithmetic();
	bool test_delimiters();
	bool test_bitwise();
};