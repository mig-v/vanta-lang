#pragma once

#include <vector>
#include <unordered_map>

#include "codegen/value.h"

struct ClassDecl
{
	ClassDecl(const std::string& name) : name(name) {}

	std::string name;
	std::unordered_map<std::string, uint16_t> fields;	// map field name to slot
	std::unordered_map<std::string, Function*> methods; // map method name to function object
};

struct LineInfo
{
	LineInfo(int line, int span) : line(line), span(span) {}

	int line; // the source line an opcode points to
	int span; // the number of opcodes / operands this single line spans
};

struct Chunk
{
	std::vector<uint8_t> code;
	std::vector<Value> constants; // constants are any compile time first class values such as functions, literals, etc.
	std::vector<LineInfo> lines;  // line tracking info for printing runtime errors

	int get_line_with_offset(size_t bytecodeOffset) const;
};

struct CompiledModule
{
	std::string name;			// name of the module itself, or the alias provided by an import. e.g. math or 'm' in the case -> [import math as m]
	std::string filepath;		// actual filepath of the file compiled, e.g. src/renderer/renderer.va
	std::vector<Value> globals;
	std::vector<ClassDecl*> classes;
	std::unordered_map<std::string, uint16_t> moduleMap;
	std::unordered_map<std::string, uint16_t> exports;	// table of identifiers -> slot mapping of global symbols in a file
	Function* root; // main entry of the module
};