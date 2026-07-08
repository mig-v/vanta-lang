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

	bool dispatch_array_method(Value& object, const std::string& methodName, uint16_t argc);

	// debug function declarations
	#ifdef _DEBUG
	void assert_stack_invariant(const CallFrame& frame, const std::string& fnIdentifier, const std::string& ctx);
	#endif

	GarbageCollector gc;
	std::vector<Value> stack;
	std::vector<CallFrame> callStack;
	bool hasErrors;

	static const std::unordered_map<std::string, NativeMethod> arrayMethods;
};