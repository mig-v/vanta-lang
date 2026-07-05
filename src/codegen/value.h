#pragma once

#include <string>
#include <variant>
#include <vector>
#include <memory>
#include <unordered_map>

// forward declare chunk since Functions need to store their relevant chunks of bytecode which is defined in chunk.h
struct Chunk;
struct ClassDecl;
struct Instance;
struct Value;
struct ArgList;

struct NativeFnError
{
	NativeFnError() : hasError(false) {}

	std::string errorMessage;
	bool hasError;

	void error(const std::string& msg)
	{
		hasError = true;
		errorMessage = msg;
	}
};

struct GCObject
{
	bool marked = false;
	GCObject* next = nullptr;
};

struct Function
{
	std::string name;
	uint16_t argc;
	uint16_t locals;
	Chunk* chunk;
};

struct Array : GCObject
{
	std::vector<Value> arr;
};

struct Module : GCObject
{
	std::string name;                                     // name of module, reflects any alias that is passed in an import statement
	std::vector<Value> globals;	                          // array of module level globals
	std::vector<ClassDecl*> compileTimeClassDecls;		  // vector of compile time class decls that can be used in certain scenarios
	std::unordered_map<std::string, uint16_t> exports;	  // mapping of identifiers to slots that store those identifiers in 'globals'
	std::unordered_map<std::string, ClassDecl*> classMap; // mapping of string names to class declarations in the module
	Function* root;
};

// native functions are just function pointers to raw c++ functions that take in an arg list and return a value
using NativeFn = Value(*)(ArgList args, NativeFnError& ctx);
using NativeMethod = Value(*)(Value& object, ArgList args, NativeFnError& ctx);

using ValueData = std::variant
<
	std::monostate,
	int64_t,
	double,
	bool,
	std::string,
	Function*,
	Instance*,
	Array*,
	Module*,
	NativeFn
>;

enum class ValueKind
{
	VALUE_INT,
	VALUE_FLOAT,
	VALUE_STRING,
	VALUE_BOOL,
	VALUE_NULL,
	VALUE_ARR,
	VALUE_FN,
	VALUE_INSTANCE,
	VALUE_MODULE,
	VALUE_NATIVE_FN
};

struct Value
{
	Value() : kind(ValueKind::VALUE_NULL), data(std::in_place_type<std::monostate>) {}
	Value(std::monostate) : kind(ValueKind::VALUE_NULL), data(std::in_place_type<std::monostate>) {}
	Value(int64_t val) : kind(ValueKind::VALUE_INT), data(std::in_place_type<int64_t>, val) {}
	Value(double val) : kind(ValueKind::VALUE_FLOAT), data(std::in_place_type<double>, val) {}
	Value(bool val) : kind(ValueKind::VALUE_BOOL), data(std::in_place_type<bool>, val) {}
	Value(const std::string& val) : kind(ValueKind::VALUE_STRING), data(std::in_place_type<std::string>, val) {}
	Value(Function* val) : kind(ValueKind::VALUE_FN), data(std::in_place_type<Function*>, val) {}
	Value(NativeFn val) : kind(ValueKind::VALUE_NATIVE_FN), data(std::in_place_type<NativeFn>, val) {}
	Value(Instance* val) : kind(ValueKind::VALUE_INSTANCE), data(std::in_place_type<Instance*>, val) {}
	Value(Array* val) : kind(ValueKind::VALUE_ARR), data(std::in_place_type<Array*>, val) {}
	Value(Module* val) : kind(ValueKind::VALUE_MODULE), data(std::in_place_type<Module*>, val) {}
	
	Value(ValueKind kind, ValueData data) : kind(kind), data(data) {}

	ValueKind kind;
	ValueData data;
};

struct Instance : GCObject
{
	ClassDecl* classDecl;
	Module* hostModule;
	std::vector<Value> fields;
};

struct ArgList
{
	ArgList(Value* args, uint16_t argc) : args(args), argc(argc) {}

	Value* args;
	uint16_t argc;

	inline Value* begin() const { return args; }
	inline Value* end()   const { return args + argc; }
	inline Value& operator[](uint16_t i) { return args[i]; }
	inline uint16_t size() const { return argc; }
};