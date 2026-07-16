#include <iostream>
#include <chrono>

#include "runtime/vm.h"
#include "codegen/opcode.h"
#include "utils/debug_utils.h"
#include "utils/value_utils.h"
#include "runtime/built_ins.h"

const std::unordered_map<std::string, NativeMethod> VM::arrayMethods =
{
	{ "add", Builtins::array_add },
	{ "pop", Builtins::array_pop },
	{ "clear", Builtins::array_clear }
};

const std::unordered_map<std::string, NativeMethod> VM::fileMethods =
{
	{ "close", Builtins::file_close },
	{ "write", Builtins::file_write },
	{ "write_line", Builtins::file_write_line },
	{ "read", Builtins::file_read },
	{ "read_line", Builtins::file_read_line },
	{ "seek", Builtins::file_seek },
	{ "eof", Builtins::file_eof }
};

const std::unordered_map<std::string, NativeMethod> VM::dictMethods =
{
	{ "clear", Builtins::dict_clear },
	{ "contains", Builtins::dict_contains }
};

const std::unordered_map<std::string, NativeMethod> VM::strMethods =
{
	{ "is_digit", Builtins::str_is_digit },
	{ "is_alpha", Builtins::str_is_alpha },
	{ "is_alnum", Builtins::str_is_alnum }
};

VM::VM()
{
	this->stack.reserve(256);
	this->hasErrors = false;
}

void VM::initialize_module(Module* module)
{
#ifdef _DEBUG
	auto start = std::chrono::steady_clock::now();
#endif

	push_call_frame(module->root, CallFrameContext::Fn, module);
	dispatch_loop();

#ifdef _DEBUG
	auto end = std::chrono::steady_clock::now();
	auto elapsed = end - start;
	std::cout << "vm stack size after initializing: " << stack.size() << std::endl;
	std::cout << "initialize module: " << module->name << "... took " << std::chrono::duration<double, std::milli>(elapsed).count() << (" ms") << std::endl;
#endif

	cleanup_global_call_frame();
}

void VM::execute_module(Module* module)
{
#ifdef _DEBUG
	std::cout << "\n[Program Output]\n";
	auto start = std::chrono::steady_clock::now();
#endif

	push_call_frame(module->root, CallFrameContext::Fn, module);
	dispatch_loop();

#ifdef _DEBUG
	auto end = std::chrono::steady_clock::now();
	auto elapsed = end - start;
	std::cout << "vm stack size after execution: " << stack.size() << std::endl;
	std::cout << "main took... " << std::chrono::duration<double, std::milli>(elapsed).count() << (" ms") << std::endl;
#endif

	cleanup_global_call_frame();
	gc.log_stats();
}

Module* VM::create_runtime_module(CompiledModule* module)
{
	// create a runtime module object from this compiled module
	Module* runtimeModule = gc.alloc_object<Module>();
	runtimeModule->name = module->name;
	runtimeModule->globals = module->globals;
	runtimeModule->exports = module->exports;
	runtimeModule->root = module->root;
	runtimeModule->compileTimeClassDecls = module->classes;
	runtimeModule->filepath = module->filepath;
	runtimeModule->stemmedPath = module->stemmedPath;

	for (ClassDecl* decl : module->classes)
		runtimeModule->classMap[decl->name] = decl;

	this->allModules.push_back(runtimeModule);
	return runtimeModule;
}

void VM::dump_stack()
{
	std::cout << "[Stack Dump Begin]\n";
	for (const Value& val : stack)
		std::cout << Utils::to_string(val) << std::endl;
	std::cout << "[Stack Dump End]\n\n";
}

void VM::push_call_frame(Function* fn, CallFrameContext frameCtx, Module* hostModule)
{
	// normal functions
	if (frameCtx == CallFrameContext::Fn)
	{
		callStack.emplace_back(fn, fn->chunk->code.data(), stack.size() - fn->argc, frameCtx, hostModule);
		stack.resize(stack.size() + (fn->locals - fn->argc));
	}

	// methods
	else
	{
		callStack.emplace_back(fn, fn->chunk->code.data(), stack.size() - fn->argc - 1, frameCtx, hostModule);
		stack.resize(stack.size() + (fn->locals - fn->argc - 1));
	}
}

//void VM::push_method_call_frame(Function* fn)
//{
//	// we subtract an extra slot count because methods have an implicit 'this' argument that slot[0] has to point to
//	callStack.emplace_back(fn, fn->chunk->code.data(), stack.size() - fn->argc - 1);
//	stack.resize(stack.size() + (fn->locals - fn->argc - 1));
//}

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

void VM::cleanup_args(uint16_t argc)
{
	while (argc > 0)
	{
		stack.pop_back();
		argc--;
	}
}

bool VM::dispatch_builtin_method(Value& object, const std::string& methodName, uint16_t argc, const std::unordered_map<std::string, NativeMethod> methodMap)
{
	auto& method = methodMap.find(methodName);

	if (method == methodMap.end())
	{
		runtime_error("no method \"" + methodName + "\" exists for object");
		return true;
	}

	NativeFnCtx nativeFnCtx(&this->gc);
	Value* argsPtr = argc > 0 ? &stack[stack.size() - argc] : nullptr;
	ArgList argList(argsPtr, argc);
	Value result = method->second(object, argList, nativeFnCtx);

	if (nativeFnCtx.hasError)
		runtime_error(nativeFnCtx.errorMessage);

	// clean up the args, then pop 'object', and lastly push the result of the native method
	cleanup_args(argc);
	stack.pop_back();
	stack.emplace_back(result);
	return nativeFnCtx.hasError;
}

void VM::array_load(Array* arrPtr, const Value& index)
{
	// only allow integers to be used as an index
	if (index.kind != ValueKind::VALUE_INT)
	{
		runtime_error("cannot index with non-integer type");
		return;
	}

	int64_t indexVal = std::get<int64_t>(index.data);
	if (indexVal >= arrPtr->arr.size())
	{
		runtime_error("out of range index: " + std::to_string(indexVal) + " on array");
		return;
	}

	stack.push_back(arrPtr->arr[indexVal]);
}

void VM::array_store(Array* arrPtr, const Value& index, const Value& val)
{
	if (index.kind != ValueKind::VALUE_INT)
	{
		runtime_error("cannot index with non-integer type");
		return;
	}

	int64_t indexVal = std::get<int64_t>(index.data);
	if (indexVal >= arrPtr->arr.size())
	{
		runtime_error("out of range index: " + std::to_string(indexVal) + " on array");
		return;
	}

	arrPtr->arr[indexVal] = val;
}

void VM::dict_load(Dict* dictPtr, const Value& key)
{
	// if the key is not found in the dictionary, a runtime error occurs
	const auto& val = dictPtr->dict.find(key);
	if (val == dictPtr->dict.end())
	{
		runtime_error("key not found in dict");
		return;
	}

	stack.push_back(val->second);
}

void VM::dict_store(Dict* dictPtr, const Value& key, const Value& val)
{
	dictPtr->dict[key] = val;
}

void VM::string_load(const std::string& str, const Value& index)
{
	if (index.kind != ValueKind::VALUE_INT)
	{
		runtime_error("cannot index with non-integer type");
		return;
	}

	int64_t indexVal = std::get<int64_t>(index.data);
	if (indexVal >= str.size())
	{
		runtime_error("out of range index: " + std::to_string(indexVal) + " on string");
		return;
	}

	stack.push_back(std::string(1, str[indexVal]));
}

void VM::dispatch_loop()
{
	while (!this->hasErrors)
	{	
		CallFrame& frame = callStack.back();

		if (gc.should_collect())
			gc.collect(stack, this->allModules);
			//gc.collect(stack, frame.hostModule->globals);

		Opcode opcode = static_cast<Opcode>(*frame.ip++);

		// at this point, ip is either pointing to the next instruction, or the current opcodes operands if the opcode has operands
		switch (opcode)
		{
			// 2 byte operand <const_index>, pushes chunk.constants[const_index] onto the stack
			case Opcode::LOAD_CONST:
			{
				uint16_t constIndex = static_cast<uint16_t>(*frame.ip) | (static_cast<uint16_t>(*(frame.ip + 1)) << 8);
				stack.emplace_back(frame.fn->chunk->constants[constIndex]);
				frame.ip += 2;
				break;
			}
			
			// 2 byte operand <slot>, pushes module.globals[slot] onto the stack
			case Opcode::LOAD_GLOBAL:
			{
				uint16_t slot = static_cast<uint16_t>(*frame.ip) | (static_cast<uint16_t>(*(frame.ip + 1)) << 8);
				stack.emplace_back(frame.hostModule->globals[slot]);
				frame.ip += 2;
				break;
			}

			// 2 byte operand <slot>, pops top of stack and stores into module.globals[slot]
			case Opcode::STORE_GLOBAL:
			{
				uint16_t slot = static_cast<uint16_t>(*frame.ip) | (static_cast<uint16_t>(*(frame.ip + 1)) << 8);
				Value val = stack.back();
				frame.hostModule->globals[slot] = val;
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
				Value val = stack.back();
				stack.pop_back();
				stack[frame.frameBase + slot] = val;

				frame.ip += 2;
				break;
			}

			// 2 byte operand <fieldIndex>, '$this' is at the top of the stack, push the value stored in the instances
			// corresponding fieldIndex onto the stack
			case Opcode::LOAD_FIELD:
			{
				uint16_t fieldIndex = static_cast<uint16_t>(*frame.ip) | (static_cast<uint16_t>(*(frame.ip + 1)) << 8);
				const std::string& fieldName = std::get<std::string>(frame.fn->chunk->constants[fieldIndex].data);

				Value val = stack.back();
				stack.pop_back();

				// check to see if we're loading a value from an instance
				if (val.kind == ValueKind::VALUE_INSTANCE)
				{
					Instance* instance = std::get<Instance*>(val.data);

					// load a field from instance
					if (instance->classDecl->fields.find(fieldName) != instance->classDecl->fields.end())
					{
						stack.push_back(instance->fields[instance->classDecl->fields[fieldName]]);
					}

					// load a method from instance, need to alloc a BoundMethod object for this
					else if (instance->classDecl->methods.find(fieldName) != instance->classDecl->methods.end())
					{
						if (fieldName == instance->classDecl->name)
						{
							runtime_error("cannot bind a classes constructor");
							return;
						}
						BoundMethod* boundMethod = gc.alloc_object<BoundMethod>();
						
						boundMethod->object = instance;
						boundMethod->method = instance->classDecl->methods[fieldName];
						stack.push_back(boundMethod);
					}

					// no field or method with 'fieldName'
					else
					{
						runtime_error("undefined field \"" + fieldName + "\" on class \"" + instance->classDecl->name + "\"");
						return;
					}
				}

				// check to see if we're loading a value from a module
				else if (val.kind == ValueKind::VALUE_MODULE)
				{
					Module* importedModule = std::get<Module*>(val.data);
					uint16_t exportSlot = importedModule->exports[fieldName];
					stack.emplace_back(importedModule->globals[exportSlot]);
				}

				frame.ip += 2;
				break;
			}

			// 2 byte operand <fieldIndex>, '$this' is at the top of the stack, and the val being stored is TOS - 1
			// store val in the corresponding fieldIndex on instance
			case Opcode::STORE_FIELD:
			{
				uint16_t fieldIndex = static_cast<uint16_t>(*frame.ip) | (static_cast<uint16_t>(*(frame.ip + 1)) << 8);
				const std::string& fieldName = std::get<std::string>(frame.fn->chunk->constants[fieldIndex].data);
				Value instanceVal = stack.back();
				stack.pop_back();

				Value val = stack.back();
				stack.pop_back();

				Instance* instance = std::get<Instance*>(instanceVal.data);
				auto it = instance->classDecl->fields.find(fieldName);
				if (it == instance->classDecl->fields.end())
				{
					runtime_error("undefined field \"" + fieldName + "\"");
					return;
				}

				instance->fields[instance->classDecl->fields[fieldName]] = val;
				frame.ip += 2;
				break;
			}

			// 2 byte operand <argc>, need to set up call frame with fn object
			case Opcode::CALL_FN:
			{
				// the fn object is at stack - argc - 1 because we push the fn object, then push the args in order
				uint16_t argc = static_cast<uint16_t>(*frame.ip) | (static_cast<uint16_t>(*(frame.ip + 1)) << 8);
				Value fn = stack[stack.size() - argc - 1];

				// handle normal function calls
				if (fn.kind == ValueKind::VALUE_FN)
				{
					Function* fnData = std::get<Function*>(fn.data);
					if (argc != fnData->argc)
					{
						runtime_error("expected " + std::to_string(fnData->argc) + " arg(s), but got " + std::to_string(argc));
						return;
					}

					frame.ip += 2;
					push_call_frame(fnData, CallFrameContext::Fn, frame.hostModule);
				}

				// handle native function calls
				else if (fn.kind == ValueKind::VALUE_NATIVE_FN)
				{
					// set up the error ctx and arg list, then call the native function
					NativeFnCtx nativeFnCtx(&this->gc);
					Value* argsPtr = argc > 0 ? &stack[stack.size() - argc] : nullptr;
					ArgList argList(argsPtr, argc);
					Value result = std::get<NativeFn>(fn.data)(argList, nativeFnCtx);

					// clean up the stack, no return opcode will be used, so we need to manually clean up the stack here
					// there's also no locals in native functions, so we can directly use argc to see how many times we need to pop
					while (argc > 0)
					{
						stack.pop_back();
						argc--;
					}

					// pop the native function object since stack layout is fn_object, arg1, arg2, etc.
					stack.pop_back();

					// check if the native function had any errors, if so fire a runtime error with the provided message
					if (nativeFnCtx.hasError)
					{
						runtime_error(nativeFnCtx.errorMessage);
						return;
					}

					// lastly, place the return value of the native function onto the stack and point to the next instruction
					stack.emplace_back(result);
					frame.ip += 2;
				}

				else if (fn.kind == ValueKind::VALUE_BOUND_METHOD)
				{
					BoundMethod* fnData = std::get<BoundMethod*>(fn.data);
					if (argc != fnData->method->argc)
					{
						runtime_error("expected " + std::to_string(fnData->method->argc) + " arg(s), but got " + std::to_string(argc));
						return;
					}

					frame.ip += 2;

					// methods expect the instance to be at stack.size - argc - 1, but BoundMethod is sitting there, so we need to overwrite
					// the bound method with the actual instance here
					stack[stack.size() - argc - 1] = fnData->object;
					push_call_frame(fnData->method, CallFrameContext::Method, fnData->object->hostModule);
				}

				// error object not callable
				else
				{
					runtime_error("object is not callable, type: " + Utils::type_name(fn));
					return;
				}

				break;
			}

			// 2 operands <methodIndex> <argc>, methodIndex is an index into the chunks constants table
			case Opcode::CALL_METHOD:
			{
				uint16_t methodIndex = static_cast<uint16_t>(*frame.ip) | (static_cast<uint16_t>(*(frame.ip + 1)) << 8);
				frame.ip += 2;

				uint16_t argc = static_cast<uint16_t>(*frame.ip) | (static_cast<uint16_t>(*(frame.ip + 1)) << 8);
				frame.ip += 2;

				const std::string& methodName = std::get<std::string>(frame.fn->chunk->constants[methodIndex].data);

				// need to get the object associated with this method
				Value object = stack[stack.size() - argc - 1];

				switch (object.kind)
				{
					case ValueKind::VALUE_INSTANCE:
					{
						Instance* instance = std::get<Instance*>(object.data);

						// first, check if methodName resolves to one of instance's fields, if it does, the instance
						// may be storing a bound method, or a raw fn
						auto& fieldIt = instance->classDecl->fields.find(methodName);
						if (fieldIt != instance->classDecl->fields.end())
						{
							const Value& fieldVal = instance->fields[fieldIt->second];
							if (fieldVal.kind == ValueKind::VALUE_BOUND_METHOD)
							{
								BoundMethod* boundMethod = std::get<BoundMethod*>(fieldVal.data);
								if (argc != boundMethod->method->argc)
								{
									runtime_error("expected " + std::to_string(boundMethod->method->argc) + " arg(s), but got " + std::to_string(argc));
									return;
								}
								
								stack[stack.size() - argc - 1] = boundMethod->object;
								push_call_frame(boundMethod->method, CallFrameContext::Method, boundMethod->object->hostModule);
							}
							else if (fieldVal.kind == ValueKind::VALUE_FN)
							{
								Function* fn = std::get<Function*>(fieldVal.data);
								if (argc != fn->argc)
								{
									runtime_error("expected " + std::to_string(fn->argc) + " arg(s), but got " + std::to_string(argc));
									return;
								}

								push_call_frame(fn, CallFrameContext::Fn, frame.hostModule);
							}
						}
						else
						{
							// then need to verify the instance has a method named 'methodName'
							auto method = instance->classDecl->methods.find(methodName);
							if (method == instance->classDecl->methods.end())
							{
								runtime_error("no class method \"" + methodName + "\" exists for object");
								return;
							}

							// then verify arg counts match
							if (method->second->argc != argc)
							{
								runtime_error("class method \"" + methodName + "\" expects " + std::to_string(method->second->argc) + " args but got " + std::to_string(argc));
								return;
							}

							// then push call frame
							if (instance->classDecl->name == methodName)
								push_call_frame(method->second, CallFrameContext::Constructor, instance->hostModule);
							else
								push_call_frame(method->second, CallFrameContext::Method, instance->hostModule);
						}

						break;
					}

					case ValueKind::VALUE_ARR:
					{
						// dispatch functions returns true if there was an error
						if (dispatch_builtin_method(object, methodName, argc, arrayMethods))
							return;

						break;
					}

					case ValueKind::VALUE_FILE:
					{
						if (dispatch_builtin_method(object, methodName, argc, fileMethods))
							return;

						break;
					}

					case ValueKind::VALUE_DICT:
					{
						if (dispatch_builtin_method(object, methodName, argc, dictMethods))
							return;

						break;
					}

					case ValueKind::VALUE_STRING:
					{
						if (dispatch_builtin_method(object, methodName, argc, strMethods))
							return;

						break;
					}

					// handle any module function calls like math.sin()
					case ValueKind::VALUE_MODULE:
					{
						Module* runtimeModule = std::get<Module*>(object.data);
						auto& moduleFn = runtimeModule->exports.find(methodName);

						if (moduleFn == runtimeModule->exports.end())
						{
							runtime_error("no fn \"" + methodName + "\" found in module \"" + runtimeModule->name + "\"");
							return;
						}

						Value exportedSymbol = runtimeModule->globals[moduleFn->second];
						if (exportedSymbol.kind != ValueKind::VALUE_FN)
						{
							runtime_error("\"" + methodName + "\" not callable from module \"" + runtimeModule->name + "\"");
							return;
						}

						Function* fn = std::get<Function*>(exportedSymbol.data);
						if (argc != fn->argc)
						{
							runtime_error("module fn\"" + methodName + "\" expects " + std::to_string(fn->argc) + " args but got " + std::to_string(argc));
							return;
						}

						push_call_frame(fn, CallFrameContext::Fn, runtimeModule);
						break;
					}
					default:
					{
						runtime_error("cannot call method on non-object");
						return;
					}
				}

				break;
			}

			// no operand, need to clean up stack and restore previous call frame
			case Opcode::RETURN:
			{
				// the return value is on the top of the stack, we need to save it temporarily while we clean up the stack
				Value returnVal;

				// constructors implicitly return 'this' which is stored at the frames base
				if (frame.frameCtx == CallFrameContext::Constructor)
				{
					returnVal = stack[frame.frameBase];
				}

				// otherwise, pop the return value off of the stack
				else
				{
					returnVal = stack.back();
					stack.pop_back();
				}

				// pop all args and any locals until we get to the frames base
				while (stack.size() > frame.frameBase)
					stack.pop_back();

				#ifdef _DEBUG
					assert_stack_invariant(frame, frame.fn->name, "RETURN");
				#endif

				// for normal functions, the fn object is on the stack just before the args (1 spot below the frame base), so we pop it off
				// after popping all the args
				if (frame.frameCtx == CallFrameContext::Fn)
					stack.pop_back();

				// remove the functions call frame, and restore the return value
				callStack.pop_back();
				stack.push_back(returnVal);
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
				if (Utils::is_truthy(cond))
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
				if (!Utils::is_truthy(cond))
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
				stack.emplace_back(!Utils::is_truthy(val));
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
					stack.push_back(std::get<std::string>(lhs.data) + std::get<std::string>(rhs.data));
				}
				else if (Utils::is_numeric(lhs) && Utils::is_numeric(rhs))
				{
					if (lhs.kind == ValueKind::VALUE_FLOAT || rhs.kind == ValueKind::VALUE_FLOAT)
						stack.push_back(Utils::to_double(lhs) + Utils::to_double(rhs));
					else
						stack.push_back(Utils::to_int(lhs) + Utils::to_int(rhs));
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

				if (Utils::is_numeric(lhs) && Utils::is_numeric(rhs))
				{
					if (lhs.kind == ValueKind::VALUE_FLOAT || rhs.kind == ValueKind::VALUE_FLOAT)
						stack.emplace_back(Utils::to_double(lhs) - Utils::to_double(rhs));
					else
						stack.emplace_back(Utils::to_int(lhs) - Utils::to_int(rhs));
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
				else if (Utils::is_numeric(lhs) && Utils::is_numeric(rhs))
				{
					if (lhs.kind == ValueKind::VALUE_FLOAT || rhs.kind == ValueKind::VALUE_FLOAT)
						stack.emplace_back(Utils::to_double(lhs) * Utils::to_double(rhs));
					else
						stack.emplace_back(Utils::to_int(lhs) * Utils::to_int(rhs));
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

				if (Utils::is_numeric(lhs) && Utils::is_numeric(rhs))
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
						stack.emplace_back(Utils::to_double(lhs) / Utils::to_double(rhs));
					else
						stack.emplace_back(Utils::to_int(lhs) / Utils::to_int(rhs));
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

				if (Utils::is_numeric(lhs) && Utils::is_numeric(rhs))
				{
					if (lhs.kind == ValueKind::VALUE_FLOAT || rhs.kind == ValueKind::VALUE_FLOAT)
						stack.emplace_back(std::fmod(Utils::to_double(lhs), Utils::to_double(rhs)));
					else
						stack.emplace_back(Utils::to_int(lhs) % Utils::to_int(rhs));
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

				if (Utils::is_numeric(lhs) && Utils::is_numeric(rhs))
				{
					if (lhs.kind == ValueKind::VALUE_FLOAT || rhs.kind == ValueKind::VALUE_FLOAT)
						stack.emplace_back(std::pow(Utils::to_double(lhs), Utils::to_double(rhs)));
					else
						stack.emplace_back(static_cast<int64_t>(std::pow(Utils::to_int(lhs), Utils::to_int(rhs))));
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

				if (Utils::is_numeric(lhs) && Utils::is_numeric(rhs))
				{
					if (lhs.kind == ValueKind::VALUE_FLOAT || rhs.kind == ValueKind::VALUE_FLOAT)
						stack.emplace_back(Utils::to_double(lhs) < Utils::to_double(rhs));
					else
						stack.emplace_back(Utils::to_int(lhs) < Utils::to_int(rhs));
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

				if (Utils::is_numeric(lhs) && Utils::is_numeric(rhs))
				{
					if (lhs.kind == ValueKind::VALUE_FLOAT || rhs.kind == ValueKind::VALUE_FLOAT)
						stack.emplace_back(Utils::to_double(lhs) <= Utils::to_double(rhs));
					else
						stack.emplace_back(Utils::to_int(lhs) <= Utils::to_int(rhs));
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

				if (Utils::is_numeric(lhs) && Utils::is_numeric(rhs))
				{
					if (lhs.kind == ValueKind::VALUE_FLOAT || rhs.kind == ValueKind::VALUE_FLOAT)
						stack.emplace_back(Utils::to_double(lhs) > Utils::to_double(rhs));
					else
						stack.emplace_back(Utils::to_int(lhs) > Utils::to_int(rhs));
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

				if (Utils::is_numeric(lhs) && Utils::is_numeric(rhs))
				{
					if (lhs.kind == ValueKind::VALUE_FLOAT || rhs.kind == ValueKind::VALUE_FLOAT)
						stack.emplace_back(Utils::to_double(lhs) >= Utils::to_double(rhs));
					else
						stack.emplace_back(Utils::to_int(lhs) >= Utils::to_int(rhs));
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

			case Opcode::MAKE_ARR:
			{
				uint16_t elementCount = static_cast<uint16_t>(*frame.ip) | (static_cast<uint16_t>(*(frame.ip + 1)) << 8);
				
				Array* arrObj = gc.alloc_object<Array>();
				arrObj->arr.resize(elementCount);
				
				// elements are pushed left to right, so when popping, we need to assign them back to front to preserve the original order
				for (int i = elementCount - 1; i >= 0; i--)
				{
					arrObj->arr[i] = stack.back();
					stack.pop_back();
				}

				stack.emplace_back(arrObj);
				frame.ip += 2;
				break;
			}

			// 2 byte operand <pair_count>, pop off 2n elements from the stack and create new dict object
			case Opcode::MAKE_DICT:
			{
				uint16_t pairCount = static_cast<uint16_t>(*frame.ip) | (static_cast<uint16_t>(*(frame.ip + 1)) << 8);

				Dict* dictObj = gc.alloc_object<Dict>();

				// pairs are pushed as key : val, key : val, etc. so we need to pop in opposite order val, key
				for (int i = 0; i < pairCount; i++)
				{
					Value val = stack.back();
					stack.pop_back();

					Value key = stack.back();
					stack.pop_back();

					dictObj->dict[key] = val;
				}

				stack.emplace_back(dictObj);
				frame.ip += 2;
				break;
			}

			// no operands, obj and index are on the stack, need to push the obj[index] onto the stack
			case Opcode::INDEX_LOAD:
			{
				// obj pushed first, then index
				Value index = stack.back();
				stack.pop_back();

				Value obj = stack.back();
				stack.pop_back();

				if (obj.kind == ValueKind::VALUE_ARR)
					array_load(std::get<Array*>(obj.data), index);
				else if (obj.kind == ValueKind::VALUE_DICT)
					dict_load(std::get<Dict*>(obj.data), index);
				else if (obj.kind == ValueKind::VALUE_STRING)
					string_load(std::get<std::string>(obj.data), index);
				else
				{
					runtime_error("cannot index into non-indexable object");
					return;
				}
	
				break;
			}

			// no operands, value to store, obj, and index, are on the stack in that order
			case Opcode::INDEX_STORE:
			{
				Value index = stack.back();
				stack.pop_back();

				Value obj = stack.back();
				stack.pop_back();

				Value val = stack.back();
				stack.pop_back();

				if (obj.kind == ValueKind::VALUE_ARR)
					array_store(std::get<Array*>(obj.data), index, val);
				else if (obj.kind == ValueKind::VALUE_DICT)
					dict_store(std::get<Dict*>(obj.data), index, val);
				else if (obj.kind == ValueKind::VALUE_STRING)
				{
					runtime_error("cannot write directly to string by index");
					return;
				}
				else
				{
					runtime_error("cannot index into non-indexable object");
					return;
				}

				break;
			}

			// no operands, iter, end, and step pushed onto the stack in that order
			case Opcode::FOR_ITER_RANGE:
			{
				Value step = stack.back();
				stack.pop_back();

				Value end = stack.back();
				stack.pop_back();

				Value iter = stack.back();
				stack.pop_back();

				// make sure iter, end, and step are all integers, if not issue a runtime error
				if (iter.kind != ValueKind::VALUE_INT || end.kind != ValueKind::VALUE_INT || step.kind != ValueKind::VALUE_INT)
				{
					runtime_error("for loop range values must be integers");
					return;
				}

				// the condition checked changes depending on if step is positive >= 0, or negative < 0
				int64_t stepVal = std::get<int64_t>(step.data);
				int64_t iterVal = std::get<int64_t>(iter.data);
				int64_t endVal = std::get<int64_t>(end.data);

				if (stepVal >= 0)
					stack.emplace_back(Value(iterVal < endVal));
				else
					stack.emplace_back(Value(iterVal > endVal));

				break;
			}

			// 2 byte operand <class_index>, store module.classes[class_index] in newly created instance and push instance on the stack
			case Opcode::MAKE_INSTANCE:
			{
				Instance* instance = gc.alloc_object<Instance>();
				uint16_t classIndex = static_cast<uint16_t>(*frame.ip) | (static_cast<uint16_t>(*(frame.ip + 1)) << 8);
				instance->classDecl = frame.hostModule->compileTimeClassDecls[classIndex];
				instance->fields.resize(instance->classDecl->fields.size());
				instance->hostModule = frame.hostModule;
				stack.emplace_back(instance);
				frame.ip += 2;
				break;
			}

			// 2 byte operand <name_index> resolves to the name of the class as a string in the constants table
			// top of stack should be module object, then use constants[nameIndex] to get the string of the field we're loading
			case Opcode::MAKE_MODULE_INSTANCE:
			{
				uint16_t nameIndex = static_cast<uint16_t>(*frame.ip) | (static_cast<uint16_t>(*(frame.ip + 1)) << 8);
				frame.ip += 2;

				// the module object is just before the args
				Value obj = stack.back();
				stack.pop_back();

				if (obj.kind != ValueKind::VALUE_MODULE)
				{
					runtime_error("cannot make module instance from non-module object");
					return;
				}

				Module* module = std::get<Module*>(obj.data);
				const std::string& className = std::get<std::string>(frame.fn->chunk->constants[nameIndex].data);
				auto it = module->classMap.find(className);

				if (it == module->classMap.end())
				{
					runtime_error("module \"" + module->name + "\" has no class \"" + className + "\"");
					return;
				}

				ClassDecl* decl = it->second;
				Instance* instance = gc.alloc_object<Instance>();
				instance->classDecl = decl;
				instance->fields.resize(instance->classDecl->fields.size());
				instance->hostModule = module;
				stack.push_back(instance);

				break;
			}

			case Opcode::EXIT:
				return;
		}
	}
}

void VM::runtime_error(const std::string& errorMsg)
{
	CallFrame& frame = callStack.back();
	int line = frame.fn->chunk->get_line_with_offset(frame.ip - frame.fn->chunk->code.data());
	std::cout << "[Runtime Error] line " << line << ": file \"" << frame.hostModule->stemmedPath << ".va\" " << errorMsg << "\n";
	print_stack_trace();
	this->hasErrors = true;
}

void VM::print_stack_trace()
{
	std::cout << "[Stack Trace]\n";
	for (auto it = callStack.rbegin(); it != callStack.rend(); it++)
	{
		CallFrame& frame = *it;
		int line = frame.fn->chunk->get_line_with_offset(frame.ip - frame.fn->chunk->code.data());
		std::cout << "line " << line << ": file \"" << frame.hostModule->stemmedPath << ".va\" in " << frame.fn->name << "()" << std::endl;
	}
}

void VM::cleanup_global_call_frame()
{
	while (stack.size() > callStack.back().frameBase)
		stack.pop_back();

	callStack.pop_back();
}

// debug function definitions
#ifdef _DEBUG

void VM::assert_stack_invariant(const CallFrame& frame, const std::string& fnIdentifier, const std::string& ctx)
{
	if (stack.size() != frame.frameBase) {
		std::cout << "<ASSERT_ERROR> stack mismatch for fn (" << fnIdentifier << ") in ctx <"
			<< ctx << ">: expected " << frame.frameBase
			<< " got " << stack.size() << std::endl;
	}
}

#endif