#pragma once

#include <vector>
#include <unordered_map>
#include <unordered_set>

#include "codegen/value.h"

#include "parser/ast.h"

struct ClassDecl
{
	ClassDecl(const std::string& name) : name(name), hasConstructor(false), constructorArgc(0) {}

	std::string name;
	std::unordered_map<std::string, uint16_t> fields;	// map field name to slot
	std::unordered_map<std::string, Function*> methods; // map method name to function object

	bool hasConstructor;
	int constructorArgc;
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
	std::string stemmedPath;	// stemmed path, always true to the source module name while CompiledModule::name can contain an alias
	std::vector<Value> globals;
	std::vector<ClassDecl*> classes;
	std::vector<ASTEnumDecl*> enums;
	std::unordered_map<std::string, uint16_t> moduleMap;
	std::unordered_map<std::string, CompiledModule*> directImports;
	std::unordered_map<std::string, uint16_t> exports;	// table of identifiers -> slot mapping of global symbols in a file
	Function* root; // main entry of the module

	inline ASTEnumDecl* find_enum_with_name(const std::string& name) const
	{
		for (ASTEnumDecl* decl : enums)
		{
			if (name == decl->identifier)
				return decl;
		}

		return nullptr;
	}
};