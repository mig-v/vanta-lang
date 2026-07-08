#include "codegen/chunk.h"

int Chunk::get_line_with_offset(size_t bytecodeOffset) const
{
	size_t instructionIndex = 0;

	for (const LineInfo& info : lines)
	{
		instructionIndex += info.span;
		if (bytecodeOffset < instructionIndex)
			return info.line;
	}

	// realistically should never happen
	return -1;
}