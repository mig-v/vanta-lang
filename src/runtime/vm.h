#pragma once

#include "codegen/chunk.h"
#include "codegen/value.h"

struct CallFrame
{
	CallFrame(Function* fn, uint8_t* ip, uint16_t frameBase) : fn(fn), ip(ip), frameBase(frameBase) {}

	Function* fn;
	uint32_t frameBase;
	uint8_t* ip;
};

class VM
{
public:
	void execute_module(Module* module);
private:
	void push_call_frame(Function* fn);
	void dispatch_loop();
	void runtime_error(const std::string& errorMsg);
	void cleanup_global_call_frame();

	void push_repeated_string(const Value& lhs, const Value& rhs);
	bool is_truthy(const Value& val);
	bool is_numeric(const Value& val);
	bool is_numerically_zero(const Value& val);
	double to_double(const Value& val);
	int64_t to_int(const Value& val);

	Module* module;
	std::vector<Value> stack;
	std::vector<CallFrame> callStack;
};