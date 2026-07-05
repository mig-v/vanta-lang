#pragma once

#include "core/memory_arena.h"
#include "core/diagnostic.h"

struct PipelineContext
{
	MemoryArena astArena;			// used solely for ASTNodes since we can free them after they're no longer needed
	MemoryArena compilerArena;		// used for more general purpose / long living objects like Functions, ClassDecls, and Modules
	DiagnosticReporter reporter;	// used to report any compilation errors
};