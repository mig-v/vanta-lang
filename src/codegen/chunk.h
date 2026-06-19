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

struct Chunk
{
	std::vector<uint8_t> code;
	std::vector<Value> constants; // constants are any compile time first class values such as functions, literals, etc.
};

struct Module
{
	std::string name;
	std::string filepath;
	std::vector<Value> globals;
	std::vector<ClassDecl*> classes;
	Function* root; // main entry of the module
};