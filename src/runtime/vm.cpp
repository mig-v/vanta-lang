#include <iostream>
#include <chrono>

#include "runtime/vm.h"
#include "codegen/opcode.h"
#include "utils/debug_utils.h"

void VM::execute_module(Module* module)
{
	this->stack.reserve(256);
	this->module = module;
	
	push_call_frame(module->root);

	auto start = std::chrono::steady_clock::now();
	dispatch_loop();
	auto end = std::chrono::steady_clock::now();
	auto elapsed = end - start;
	std::chrono::duration<double, std::milli> ms = elapsed;

	std::ostringstream oss;
	Utils::disassemble_value(module->globals[2], oss, 0);

	std::cout << oss.str() << std::endl;

	cleanup_global_call_frame();

	std::cout << "vm stack size after execution: " << stack.size() << std::endl;
	std::cout << ms.count() << " ms\n";
}

void VM::push_call_frame(Function* fn)
{
	callStack.emplace_back(fn, fn->chunk->code.data(), stack.size() - fn->argc);
	stack.resize(stack.size() + fn->locals);
}

void VM::push_repeated_string(const Value& lhs, const Value& rhs)
{
	std::string str = "";
	std::string initial;
	int64_t repeatCount = 0;

	// lhs is int, rhs is string
	if (lhs.kind == ValueKind::VALUE_INT)
	{
		initial = std::get<std::string>(rhs.data);
		repeatCount = std::get<int64_t>(lhs.data);
	}

	// lhs is string, rhs is int
	else
	{
		initial = std::get<std::string>(lhs.data);
		repeatCount = std::get<int64_t>(rhs.data);
	}

	for (int64_t i = 0; i < repeatCount; i++)
		str += initial;

	stack.emplace_back(str);
}

bool VM::is_truthy(const Value& val)
{
	switch (val.kind)
	{
		case ValueKind::VALUE_INT:      return std::get<int64_t>(val.data) != 0;
		case ValueKind::VALUE_FLOAT:    return std::get<double>(val.data) != 0.0;
		case ValueKind::VALUE_BOOL:     return std::get<bool>(val.data);
		case ValueKind::VALUE_STRING:   return std::get<std::string>(val.data) != "";
		case ValueKind::VALUE_FN:       return std::get<Function*>(val.data) != nullptr;
		case ValueKind::VALUE_INSTANCE: return true;
		case ValueKind::VALUE_NULL:     return false;
	}
}

bool VM::is_numeric(const Value& val)
{
	return val.kind == ValueKind::VALUE_INT || val.kind == ValueKind::VALUE_FLOAT || val.kind == ValueKind::VALUE_BOOL;
}

double VM::to_double(const Value& val)
{
	if (val.kind == ValueKind::VALUE_INT) return static_cast<double>(std::get<int64_t>(val.data));
	if (val.kind == ValueKind::VALUE_FLOAT) return std::get<double>(val.data);
	if (val.kind == ValueKind::VALUE_BOOL) return std::get<bool>(val.data) ? 1.0 : 0.0;
}

int64_t VM::to_int(const Value& val)
{
	if (val.kind == ValueKind::VALUE_INT) return std::get<int64_t>(val.data);
	if (val.kind == ValueKind::VALUE_FLOAT) return static_cast<int64_t>(std::get<double>(val.data));
	if (val.kind == ValueKind::VALUE_BOOL) return std::get<bool>(val.data) ? 1 : 0;
}

void VM::dispatch_loop()
{
	while (true)
	{
		CallFrame& frame = callStack.back();
		Opcode opcode = static_cast<Opcode>(*frame.ip++);

		// at this point, ip is either pointing to the next instruction, or the current opcodes operands if the opcode has operands
		switch (opcode)
		{
			// 2 byte operand <const_index>, pushes chunk.constants[const_index] onto the stack
			case Opcode::LOAD_CONST:
			{
				uint16_t constIndex = static_cast<uint16_t>(*frame.ip) | (static_cast<uint16_t>(*(frame.ip + 1)) << 8);
				uint8_t lo = *frame.ip;
				uint8_t hi = *(frame.ip + 1);

				std::cout << "LOAD CONST LO: " << (int)lo << " ,  HI: " << (int)hi << std::endl;
				stack.emplace_back(frame.fn->chunk->constants[constIndex]);
				frame.ip += 2;
				break;
			}
			
			// 2 byte operand <slot>, pushes module.globals[slot] onto the stack
			case Opcode::LOAD_GLOBAL:
			{
				uint16_t slot = static_cast<uint16_t>(*frame.ip) | (static_cast<uint16_t>(*(frame.ip + 1)) << 8);
				stack.emplace_back(module->globals[slot]);
				frame.ip += 2;
				break;
			}

			// 2 byte operand <slot>, pops top of stack and stores into module.globals[slot]
			case Opcode::STORE_GLOBAL:
			{
				uint16_t slot = static_cast<uint16_t>(*frame.ip) | (static_cast<uint16_t>(*(frame.ip + 1)) << 8);
				Value& val = stack.back();
				module->globals[slot] = val;
				stack.pop_back();
				frame.ip += 2;
				break;
			}

			// 2 byte operand <slot>, pushes stack[frame_base + slot] onto the stack
			case Opcode::LOAD_LOCAL:
			{
				uint16_t slot = static_cast<uint16_t>(*frame.ip) | (static_cast<uint16_t>(*(frame.ip + 1)) << 8);
				stack.emplace_back(stack[frame.frameBase + slot]);
				frame.ip += 2;
				break;
			}

			// 2 byte operand <slot>, pops top of stack and stores into stack[frame_base + slot]
			case Opcode::STORE_LOCAL:
			{
				uint16_t slot = static_cast<uint16_t>(*frame.ip) | (static_cast<uint16_t>(*(frame.ip + 1)) << 8);

				Value& val = stack.back();
				stack.pop_back();
				stack[frame.frameBase + slot] = val;

				frame.ip += 2;
				break;
			}

			// 2 byte operand <argc>, need to set up call frame with fn object
			case Opcode::CALL_FN:
			{
				// the fn object is at stack - argc - 1 because we push the fn object, then push the args in order
				uint16_t argc = static_cast<uint16_t>(*frame.ip) | (static_cast<uint16_t>(*(frame.ip + 1)) << 8);
				Value fn = stack[stack.size() - argc - 1];
				Function* fnData = std::get<Function*>(fn.data);

				if (argc != fnData->argc)
				{
					runtime_error("expected " + std::to_string(fnData->argc) + " arg(s), but got " + std::to_string(argc));
					return;
				}

				frame.ip += 2;
				push_call_frame(fnData);
				break;
			}

			// no operand, need to clean up stack and restore previous call frame
			case Opcode::RETURN:
			{
				// the return value is on the top of the stack, we need to save it temporarily while we clean up the stack
				Value returnVal = stack.back();
				stack.pop_back();

				// pop all args and any locals until we get to the frames base
				while (stack.size() > frame.frameBase)
					stack.pop_back();

				// recall, just before the args, the fn object was on the stack when we we're executing fn_call, so we pop the function here
				stack.pop_back();

				// remove the functions call frame, and restore the return value
				callStack.pop_back();
				stack.emplace_back(returnVal);
				break;
			}

			// unconditional jump, 2 byte operand <jmp_target>, jumps directly to <jmp_target>, dont need to increment ip
			case Opcode::JMP:
			{
				uint16_t jmpTarget = static_cast<uint16_t>(*frame.ip) | (static_cast<uint16_t>(*(frame.ip + 1)) << 8);
				frame.ip = &frame.fn->chunk->code[jmpTarget];
				break;
			}

			// 2 byte operand <jmp_target> peeks at the top of the stack and jumps to <jmp_target> if true, doesnt pop
			case Opcode::JMP_IF_TRUE:
			{
				uint16_t jmpTarget = static_cast<uint16_t>(*frame.ip) | (static_cast<uint16_t>(*(frame.ip + 1)) << 8);
				Value& cond = stack.back();

				// if true, set ip to jmpTarget, if not, increment ip past the operand and continue execution
				if (is_truthy(cond))
					frame.ip = &frame.fn->chunk->code[jmpTarget];
				else
					frame.ip += 2;

				break;
			}

			// 2 byte operand <jmp_target> peeks at the top of the stack and jumps to <jmp_target> if false, doesnt pop
			case Opcode::JMP_IF_FALSE:
			{
				uint16_t jmpTarget = static_cast<uint16_t>(*frame.ip) | (static_cast<uint16_t>(*(frame.ip + 1)) << 8);
				Value& cond = stack.back();

				// same logic as JMP_IF_TRUE, we just negate the is_truthy result
				if (!is_truthy(cond))
					frame.ip = &frame.fn->chunk->code[jmpTarget];
				else
					frame.ip += 2;

				break;
			}

			// no operands, pops the top of the stack
			case Opcode::POP:
			{
				stack.pop_back();
				break;
			}

			// unary ops, all work the same way, no operands, pop the top value and apply the unary operation, push result back onto stack
			case Opcode::NEG:
			{
				Value& val = stack.back();
				stack.pop_back();

				if (val.kind == ValueKind::VALUE_INT)
					stack.emplace_back(-std::get<int64_t>(val.data));
				else if (val.kind == ValueKind::VALUE_FLOAT)
					stack.emplace_back(-std::get<double>(val.data));
				else
				{
					runtime_error("invalid operand for unary '-', only ints and floats supported");
					return;
				}

				break;
			}

			case Opcode::NOT:
			{
				Value& val = stack.back();
				stack.pop_back();

				if (val.kind != ValueKind::VALUE_INT)
				{
					runtime_error("invalid operand for '~', only ints are supported");
					return;
				}

				stack.emplace_back(~std::get<int64_t>(val.data));
				break;
			}

			case Opcode::LOGICAL_NOT:
			{
				Value& val = stack.back();
				stack.pop_back();
				stack.emplace_back(!is_truthy(val));
				break;
			}

			// all binary operations below operate the same way, no operands, pops lhs and rhs off the stack, performs operation, pushes result
			case Opcode::ADD:
			{
				Value rhs = stack.back();
				stack.pop_back();

				Value lhs = stack.back();
				stack.pop_back();

				if (lhs.kind == ValueKind::VALUE_STRING && rhs.kind == ValueKind::VALUE_STRING)
				{
					stack.emplace_back(std::get<std::string>(lhs.data) + std::get<std::string>(rhs.data));
				}
				else if (is_numeric(lhs) && is_numeric(rhs))
				{
					if (lhs.kind == ValueKind::VALUE_FLOAT || rhs.kind == ValueKind::VALUE_FLOAT)
						stack.emplace_back(to_double(lhs) + to_double(rhs));
					else
						stack.emplace_back(to_int(lhs) + to_int(rhs));
				}
				else
				{
					runtime_error("invalid operand types for '+', only ints, floats, strings, and bools supported");
					return;
				}

				break;
			}

			case Opcode::SUB:
			{
				Value rhs = stack.back();
				stack.pop_back();

				Value lhs = stack.back();
				stack.pop_back();

				if (is_numeric(lhs) && is_numeric(rhs))
				{
					if (lhs.kind == ValueKind::VALUE_FLOAT || rhs.kind == ValueKind::VALUE_FLOAT)
						stack.emplace_back(to_double(lhs) - to_double(rhs));
					else
						stack.emplace_back(to_int(lhs) - to_int(rhs));
				}
				else
				{
					runtime_error("invalid operand types for '-', only ints, floats, and bools supported");
					return;
				}

				break;
			}

			case Opcode::MUL:
			{
				Value rhs = stack.back();
				stack.pop_back();

				Value lhs = stack.back();
				stack.pop_back();

				if ((lhs.kind == ValueKind::VALUE_STRING && rhs.kind == ValueKind::VALUE_INT) 
					|| (lhs.kind == ValueKind::VALUE_INT && rhs.kind == ValueKind::VALUE_STRING))
				{
					push_repeated_string(lhs, rhs);
				}
				else if (is_numeric(lhs) && is_numeric(rhs))
				{
					if (lhs.kind == ValueKind::VALUE_FLOAT || rhs.kind == ValueKind::VALUE_FLOAT)
						stack.emplace_back(to_double(lhs) * to_double(rhs));
					else
						stack.emplace_back(to_int(lhs) * to_int(rhs));
				}
				else
				{
					runtime_error("invalid operand types for '*', only ints, floats, strings, and bools supported");
					return;
				}

				break;
			}

			case Opcode::DIV:
			{
				Value rhs = stack.back();
				stack.pop_back();

				Value lhs = stack.back();
				stack.pop_back();

				if (is_numeric(lhs) && is_numeric(rhs))
				{
					// check for division by zero
					if (rhs.kind == ValueKind::VALUE_INT && std::get<int64_t>(rhs.data) == 0)
					{
						runtime_error("int division by zero");
						return;
					}
					else if (rhs.kind == ValueKind::VALUE_FLOAT && std::get<double>(rhs.data) == 0.0)
					{
						runtime_error("float division by zero");
						return;
					}
					else if (rhs.kind == ValueKind::VALUE_BOOL && !std::get<bool>(rhs.data))
					{
						runtime_error("division by zero");
						return;
					}

					if (lhs.kind == ValueKind::VALUE_FLOAT || rhs.kind == ValueKind::VALUE_FLOAT)
						stack.emplace_back(to_double(lhs) / to_double(rhs));
					else
						stack.emplace_back(to_int(lhs) / to_int(rhs));
				}
				else
				{
					runtime_error("invalid operand types for '/', only ints, floats, and bools supported");
					return;
				}

				break;
			}

			case Opcode::MOD:
			{
				Value rhs = stack.back();
				stack.pop_back();

				Value lhs = stack.back();
				stack.pop_back();

				if (is_numeric(lhs) && is_numeric(rhs))
				{
					if (lhs.kind == ValueKind::VALUE_FLOAT || rhs.kind == ValueKind::VALUE_FLOAT)
						stack.emplace_back(std::fmod(to_double(lhs), to_double(rhs)));
					else
						stack.emplace_back(to_int(lhs) % to_int(rhs));
				}
				else
				{
					runtime_error("invalid operand types for '%', only ints, floats, and bools supported");
					return;
				}

				break;
			}

			case Opcode::POW:
			{
				Value rhs = stack.back();
				stack.pop_back();

				Value lhs = stack.back();
				stack.pop_back();

				if (is_numeric(lhs) && is_numeric(rhs))
				{
					if (lhs.kind == ValueKind::VALUE_FLOAT || rhs.kind == ValueKind::VALUE_FLOAT)
						stack.emplace_back(std::pow(to_double(lhs), to_double(rhs)));
					else
						stack.emplace_back(static_cast<int64_t>(std::pow(to_int(lhs), to_int(rhs))));
				}
				else
				{
					runtime_error("invalid operand types for '**', only ints, floats, and bools supported");
					return;
				}

				break;
			}

			case Opcode::LT:
			{
				Value rhs = stack.back();
				stack.pop_back();

				Value lhs = stack.back();
				stack.pop_back();

				if (is_numeric(lhs) && is_numeric(rhs))
				{
					if (lhs.kind == ValueKind::VALUE_FLOAT || rhs.kind == ValueKind::VALUE_FLOAT)
						stack.emplace_back(to_double(lhs) < to_double(rhs));
					else
						stack.emplace_back(to_int(lhs) < to_int(rhs));
				}
				else
				{
					runtime_error("invalid operand types for '<', only ints, floats, and bools supported");
					return;
				}

				break;
			}

			case Opcode::LTE:
			{
				Value rhs = stack.back();
				stack.pop_back();

				Value lhs = stack.back();
				stack.pop_back();

				if (is_numeric(lhs) && is_numeric(rhs))
				{
					if (lhs.kind == ValueKind::VALUE_FLOAT || rhs.kind == ValueKind::VALUE_FLOAT)
						stack.emplace_back(to_double(lhs) <= to_double(rhs));
					else
						stack.emplace_back(to_int(lhs) <= to_int(rhs));
				}
				else
				{
					runtime_error("invalid operand types for '<=', only ints, floats, and bools supported");
					return;
				}

				break;
			}

			case Opcode::GT:
			{
				Value rhs = stack.back();
				stack.pop_back();

				Value lhs = stack.back();
				stack.pop_back();

				if (is_numeric(lhs) && is_numeric(rhs))
				{
					if (lhs.kind == ValueKind::VALUE_FLOAT || rhs.kind == ValueKind::VALUE_FLOAT)
						stack.emplace_back(to_double(lhs) > to_double(rhs));
					else
						stack.emplace_back(to_int(lhs) > to_int(rhs));
				}
				else
				{
					runtime_error("invalid operand types for '>', only ints, floats, and bools supported");
					return;
				}

				break;
			}

			case Opcode::GTE:
			{
				Value rhs = stack.back();
				stack.pop_back();

				Value lhs = stack.back();
				stack.pop_back();

				if (is_numeric(lhs) && is_numeric(rhs))
				{
					if (lhs.kind == ValueKind::VALUE_FLOAT || rhs.kind == ValueKind::VALUE_FLOAT)
						stack.emplace_back(to_double(lhs) >= to_double(rhs));
					else
						stack.emplace_back(to_int(lhs) >= to_int(rhs));
				}
				else
				{
					runtime_error("invalid operand types for '>=', only ints, floats, and bools supported");
					return;
				}

				break;
			}

			case Opcode::EQ:
			{
				Value rhs = stack.back();
				stack.pop_back();

				Value lhs = stack.back();
				stack.pop_back();

				if (lhs.kind != rhs.kind)
					stack.emplace_back(false);
				else
					stack.emplace_back(lhs.data == rhs.data);

				break;
			}

			case Opcode::NEQ:
			{
				Value rhs = stack.back();
				stack.pop_back();

				Value lhs = stack.back();
				stack.pop_back();

				if (lhs.kind != rhs.kind)
					stack.emplace_back(true);
				else
					stack.emplace_back(!(lhs.data == rhs.data));

				break;
			}

			case Opcode::AND:
			{
				Value rhs = stack.back();
				stack.pop_back();

				Value lhs = stack.back();
				stack.pop_back();

				// only integers are permitted in bitwise operations
				if (lhs.kind != ValueKind::VALUE_INT || rhs.kind != ValueKind::VALUE_INT)
				{
					runtime_error("invalid operand types for '&', only ints are supported");
					return;
				}

				stack.emplace_back(std::get<int64_t>(lhs.data) & std::get<int64_t>(rhs.data));
				break;
			}

			case Opcode::OR:
			{
				Value rhs = stack.back();
				stack.pop_back();

				Value lhs = stack.back();
				stack.pop_back();

				// only integers are permitted in bitwise operations
				if (lhs.kind != ValueKind::VALUE_INT || rhs.kind != ValueKind::VALUE_INT)
				{
					runtime_error("invalid operand types for '|', only ints are supported");
					return;
				}

				stack.emplace_back(std::get<int64_t>(lhs.data) | std::get<int64_t>(rhs.data));
				break;
			}

			case Opcode::XOR:
			{
				Value rhs = stack.back();
				stack.pop_back();

				Value lhs = stack.back();
				stack.pop_back();

				// only integers are permitted in bitwise operations
				if (lhs.kind != ValueKind::VALUE_INT || rhs.kind != ValueKind::VALUE_INT)
				{
					runtime_error("invalid operand types for '^', only ints are supported");
					return;
				}

				stack.emplace_back(std::get<int64_t>(lhs.data) ^ std::get<int64_t>(rhs.data));
				break;
			}

			case Opcode::L_SHIFT:
			{
				Value rhs = stack.back();
				stack.pop_back();

				Value lhs = stack.back();
				stack.pop_back();

				// only integers are permitted in bitwise operations
				if (lhs.kind != ValueKind::VALUE_INT || rhs.kind != ValueKind::VALUE_INT)
				{
					runtime_error("invalid operand types for '<<', only ints are supported");
					return;
				}

				stack.emplace_back(std::get<int64_t>(lhs.data) << std::get<int64_t>(rhs.data));
				break;
			}

			case Opcode::R_SHIFT:
			{
				Value rhs = stack.back();
				stack.pop_back();

				Value lhs = stack.back();
				stack.pop_back();

				// only integers are permitted in bitwise operations
				if (lhs.kind != ValueKind::VALUE_INT || rhs.kind != ValueKind::VALUE_INT)
				{
					runtime_error("invalid operand types for '>>', only ints are supported");
					return;
				}

				stack.emplace_back(std::get<int64_t>(lhs.data) >> std::get<int64_t>(rhs.data));
				break;
			}

			case Opcode::EXIT:
				return;
		}
	}
}

void VM::runtime_error(const std::string& errorMsg)
{
	std::cout << "Runtime Error: " << errorMsg << std::endl;
}

void VM::cleanup_global_call_frame()
{
	while (stack.size() > callStack.back().frameBase)
		stack.pop_back();

	callStack.pop_back();

}