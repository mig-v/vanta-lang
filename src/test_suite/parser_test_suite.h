#pragma once

#include "test_suite/test.h"
#include "test_suite/test_suite.h"

class ParserTestSuite : public TestSuite
{
public:
	ParserTestSuite();
	void regenerate_ast_snapshots();

protected:
	void internal_entry() override;
private:
	bool test_literals();
};