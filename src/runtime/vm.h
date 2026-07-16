#pragma once

#include "codegen/chunk.h"
#include "codegen/value.h"
#include "runtime/gc.h"

enum class CallFrameContext
{
	Fn,
	Method,
	Constructor
};

struct CallFrame
{
	CallFrame(Function* fn, uint8_t* ip, uint16_t frameBase, CallFrameContext frameCtx, Module* hostModule) 
		: fn(fn), ip(ip), frameBase(frameBase), frameCtx(frameCtx), hostModule(hostModule) {}

	Function* fn;
	uint32_t frameBase;
	Module* hostModule;
	uint8_t* ip;
	CallFrameContext frameCtx;
};

class VM
{
public:
	VM();

	void initialize_module(Module* module);
	void execute_module(Module* module);
	Module* create_runtime_module(CompiledModule* module);

	inline bool has_errors() const { return hasErrors; }

private:
	void push_call_frame(Function* fn, CallFrameContext frameCtx, Module* hostModule);
	void dispatch_loop();
	void runtime_error(const std::string& errorMsg);
	void print_stack_trace();
	void cleanup_global_call_frame();
	void push_repeated_string(const Value& lhs, const Value& rhs);
	void cleanup_args(uint16_t argc);
	void dump_stack();

	bool dispatch_builtin_method(Value& object, const std::string& methodName, uint16_t argc, const std::unordered_map<std::string, NativeMethod> methodMap);

	void array_load(Array* arrPtr, const Value& index);
	void array_store(Array* arrPtr, const Value& index, const Value& val);
	void dict_load(Dict* dictPtr, const Value& key);
	void dict_store(Dict* dictPtr, const Value& key, const Value& val);
	void string_load(const std::string& str, const Value& index);

	// debug function declarations
	#ifdef _DEBUG
	void assert_stack_invariant(const CallFrame& frame, const std::string& fnIdentifier, const std::string& ctx);
	#endif

	GarbageCollector gc;
	std::vector<Value> stack;
	std::vector<CallFrame> callStack;
	std::vector<Module*> allModules;
	bool hasErrors;

	// built in methods
	static const std::unordered_map<std::string, NativeMethod> arrayMethods;
	static const std::unordered_map<std::string, NativeMethod> fileMethods;
	static const std::unordered_map<std::string, NativeMethod> dictMethods;
	static const std::unordered_map<std::string, NativeMethod> strMethods;
};