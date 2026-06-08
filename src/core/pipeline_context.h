#pragma once

#include "core/memory_arena.h"
#include "core/diagnostic.h"

struct PipelineContext
{
	MemoryArena arena;
	DiagnosticReporter reporter;
};