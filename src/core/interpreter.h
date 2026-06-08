#pragma once

#include "core/pipeline_context.h"

class Interpreter
{
public:
	Interpreter() = default;

	void run();
	void run_tests();
private:
	PipelineContext ctx;
};