#include "codegen/codegen.h"

#include <iostream>

Codegen::Codegen()
{
	this->currentFn = nullptr;
	this->module = nullptr;
	this->ctx = nullptr;
	this->currentClass = nullptr;
	this->classDepth = 0;
}

Module* Codegen::compile(const std::vector<ASTNode*>& ast, PipelineContext* ctx)
{
	this->ctx = ctx;

	module = ctx->arena.alloc<Module>();

	module->name = "temp_module_name";
	module->filepath = "temp_module_filepath";
	module->root = make_fn("__main__", 0, 0);

	currentFn = module->root;

	for (ASTNode* node : ast)
		compile_node(node);

	// resize the modules globals array to account for all globals defined, the global scope will be the last scope remaining when compilation is done
	module->globals.resize(env.current_scope_slot_count());
	emit_opcode(Opcode::EXIT);
	return module;
}

void Codegen::emit_opcode(Opcode opcode)
{
	Chunk* curr = get_current_chunk();
	curr->code.push_back(static_cast<uint8_t>(opcode));
}

uint16_t Codegen::emit_operand(uint16_t operand)
{
	Chunk* curr = get_current_chunk();
	uint16_t address = curr->code.size();
	curr->code.push_back(static_cast<uint8_t>(operand & 0xFF));			// write low byte
	curr->code.push_back(static_cast<uint8_t>(operand >> 8) & 0xFF);	// write high byte
	return address;
}

void Codegen::emit_store_lhs(ASTNode* lhs)
{
	// regular identifier, can store directly into its slot
	if (lhs->kind == ASTKind::AST_IDENTIFIER)
	{
		const ASTIdentifier& data = std::get<ASTIdentifier>(lhs->data);

		EnvEntry entry = env.resolve_entry(data.identifier);
		if (entry.scope == -1 && entry.slot == -1)
		{
			ctx->reporter.submit_diagnostic(Diagnostic{ Phase::Codegen, "undefined identifier \"" + data.identifier + "\"",lhs ->line, lhs->column });
			return;
		}

		emit_store_for_entry(entry);
	}

	// writing to an array slot -> arr[i] = val
	else if (lhs->kind == ASTKind::AST_ARRAY_ACCESS)
	{
		const ASTArrayAccess& data = std::get<ASTArrayAccess>(lhs->data);
		compile_node(data.arr);
		compile_node(data.index);
		emit_opcode(Opcode::ARRAY_STORE);
	}

	// writing to a field, foo.x = val
	else if (lhs->kind == ASTKind::AST_FIELD_ACCESS)
	{
		const ASTFieldAccess& data = std::get<ASTFieldAccess>(lhs->data);
		compile_node(data.object);
		uint16_t fieldIndex = add_constant_to_chunk(Value(ValueKind::VALUE_STRING, ValueData(std::in_place_type<std::string>, data.field)));
		emit_opcode(Opcode::STORE_FIELD);
		emit_operand(fieldIndex);
	}
}

void Codegen::emit_load_lhs(ASTNode* lhs)
{
	if (lhs->kind == ASTKind::AST_IDENTIFIER)
	{
		const ASTIdentifier& data = std::get<ASTIdentifier>(lhs->data);

		EnvEntry entry = env.resolve_entry(data.identifier);
		if (entry.scope == -1 && entry.slot == -1)
		{
			ctx->reporter.submit_diagnostic(Diagnostic{ Phase::Codegen, "undefined identifier \"" + data.identifier + "\"",lhs->line, lhs->column });
			return;
		}

		emit_load_for_entry(entry);
	}
	else if (lhs->kind == ASTKind::AST_ARRAY_ACCESS)
	{
		const ASTArrayAccess& data = std::get<ASTArrayAccess>(lhs->data);
		compile_node(data.arr);
		compile_node(data.index);
		emit_opcode(Opcode::ARRAY_LOAD);
	}

	// load the value of foo.x by pushing it onto the stack
	else if (lhs->kind == ASTKind::AST_FIELD_ACCESS)
	{
		const ASTFieldAccess& data = std::get<ASTFieldAccess>(lhs->data);
		compile_node(data.object);
		uint16_t fieldIndex = add_constant_to_chunk(Value(ValueKind::VALUE_STRING, ValueData(std::in_place_type<std::string>, data.field)));
		emit_opcode(Opcode::LOAD_FIELD);
		emit_operand(fieldIndex);
	}
}

void Codegen::emit_store_for_entry(EnvEntry entry)
{
	if (entry.scope == 0)
	{
		emit_opcode(Opcode::STORE_GLOBAL);
		emit_operand(static_cast<uint16_t>(entry.slot));
	}
	else
	{
		emit_opcode(Opcode::STORE_LOCAL);
		emit_operand(static_cast<uint16_t>(entry.slot));
	}
}

void Codegen::emit_load_for_entry(EnvEntry entry)
{
	if (entry.scope == 0)
	{
		emit_opcode(Opcode::LOAD_GLOBAL);
		emit_operand(static_cast<uint16_t>(entry.slot));
	}
	else
	{
		emit_opcode(Opcode::LOAD_LOCAL);
		emit_operand(static_cast<uint16_t>(entry.slot));
	}
}

void Codegen::emit_compound_assignment_op(TokenKind op)
{
	switch (op)
	{
		case TokenKind::TOKEN_PLUS_EQUALS:   emit_opcode(Opcode::ADD); break;
		case TokenKind::TOKEN_MINUS_EQUALS:  emit_opcode(Opcode::SUB); break;
		case TokenKind::TOKEN_TIMES_EQUALS:  emit_opcode(Opcode::MUL); break;
		case TokenKind::TOKEN_DIVIDE_EQUALS: emit_opcode(Opcode::DIV); break;
	}
}

void Codegen::emit_binary_op(TokenKind op)
{
	// binary expr includes: 
	// comparison ops: <, <=, >, >=
	// equality ops:  ==, !=
	// bitwise ops: &, |, ^, <<, >>
	// arithmetic ops: +, -, *, /, %, **
	switch (op)
	{
		case TokenKind::TOKEN_PLUS:            emit_opcode(Opcode::ADD); break;
		case TokenKind::TOKEN_MINUS:           emit_opcode(Opcode::SUB); break;
		case TokenKind::TOKEN_ASTERISK:        emit_opcode(Opcode::MUL); break;
		case TokenKind::TOKEN_FORWARD_SLASH:   emit_opcode(Opcode::DIV); break;
		case TokenKind::TOKEN_MODULO:          emit_opcode(Opcode::MOD); break;
		case TokenKind::TOKEN_POWER:           emit_opcode(Opcode::POW); break;

		case TokenKind::TOKEN_LT:              emit_opcode(Opcode::LT); break;
		case TokenKind::TOKEN_LTE:             emit_opcode(Opcode::LTE); break;
		case TokenKind::TOKEN_GT:              emit_opcode(Opcode::GT); break;
		case TokenKind::TOKEN_GTE:             emit_opcode(Opcode::GTE); break;

		case TokenKind::TOKEN_EQUALITY:        emit_opcode(Opcode::EQ); break;
		case TokenKind::TOKEN_NOT_EQUALITY:    emit_opcode(Opcode::NEQ); break;

		case TokenKind::TOKEN_BITWISE_AND:     emit_opcode(Opcode::AND); break;
		case TokenKind::TOKEN_BITWISE_OR:      emit_opcode(Opcode::OR); break;
		case TokenKind::TOKEN_BITWISE_XOR:     emit_opcode(Opcode::XOR); break;
		case TokenKind::TOKEN_BITWISE_L_SHIFT: emit_opcode(Opcode::L_SHIFT); break;
		case TokenKind::TOKEN_BITWISE_R_SHIFT: emit_opcode(Opcode::R_SHIFT); break;
	}
}

void Codegen::emit_unary_op(TokenKind op)
{
	switch (op)
	{
		case TokenKind::TOKEN_BITWISE_NOT: emit_opcode(Opcode::NOT); break;
		case TokenKind::TOKEN_NOT: emit_opcode(Opcode::LOGICAL_NOT); break;
		case TokenKind::TOKEN_MINUS: emit_opcode(Opcode::NEG); break;
	}
}

void Codegen::emit_iterable_condition_check(IterContext& iterCtx)
{
	// handle range based condition check
	if (std::holds_alternative<RangeIterContext>(iterCtx))
	{
		// load i, load end, cmp lt --> i < end (exclusive)
		RangeIterContext& range = std::get<RangeIterContext>(iterCtx);
		emit_opcode(Opcode::LOAD_LOCAL);
		emit_operand(range.iterSlot);
		emit_opcode(Opcode::LOAD_LOCAL);
		emit_operand(range.endSlot);
		emit_opcode(Opcode::LT);
		emit_opcode(Opcode::JMP_IF_FALSE);
		range.endJmpPatch = emit_operand(0xFFFF);
		emit_opcode(Opcode::POP);
	}

	// todo: handle collection iter condition check
	else if (std::holds_alternative<CollectionIterContext>(iterCtx))
	{

	}

}

void Codegen::emit_iterable_increment(IterContext& iterCtx)
{
	// handle range based condition check
	if (std::holds_alternative<RangeIterContext>(iterCtx))
	{
		// check step, if provided
		//		- then ::= load iter, load step, add
		//		- else ::= load iter, load const 1, add
		RangeIterContext& range = std::get<RangeIterContext>(iterCtx);

		// we can set the most recent loops start address to the increment bytecode
		loopStack.back().startAddress = get_current_chunk()->code.size();

		// always need to load iter
		emit_opcode(Opcode::LOAD_LOCAL);
		emit_operand(range.iterSlot);

		// step is provided
		if (range.stepSlot != -1)
		{
			emit_opcode(Opcode::LOAD_LOCAL);
			emit_operand(range.stepSlot);
		}

		// step is not provided, increment by default value of 1
		else
		{
			uint16_t constIndex = add_constant_to_chunk(Value(ValueKind::VALUE_INT, ValueData(std::in_place_type<int64_t>, 1)));
			emit_opcode(Opcode::LOAD_CONST);
			emit_operand(constIndex);
		}

		// always add after iter and (default or provided) step is loaded
		emit_opcode(Opcode::ADD);
		emit_opcode(Opcode::STORE_LOCAL);
		emit_operand(range.iterSlot);

		emit_opcode(Opcode::JMP);
		emit_operand(range.conditionAddress);

		// this is the end of the for loop (the only place where iterables can occur), so we can backpatch the end jmp right here
		patch_jump(range.endJmpPatch);
		emit_opcode(Opcode::POP);
	}

	// todo: handle collection iter increment
	else if (std::holds_alternative<CollectionIterContext>(iterCtx))
	{

	}
}

void Codegen::emit_function(ASTNode* node)
{
	const ASTFnDecl& data = std::get<ASTFnDecl>(node->data);

	EnvEntry entry = env.resolve_entry(data.identifier);
	if (entry.slot != -1 && entry.scope == env.get_scope_depth())
		ctx->reporter.submit_diagnostic({ Phase::Codegen, "duplicate identifier '" + data.identifier + "'", node->line, node->column });

	// add the function into the environment in the parent scope
	int fnSlot = alloc_slot(data.identifier);

	// we dont know the amount of locals in the function yet, we need to compile the body of the function first, then get
	// the slot index of the functions scope
	currentFn = make_fn(data.identifier, data.params.size(), 0);

	env.new_scope(ScopeKind::FnLevel);

	for (const std::string& param : data.params)
		alloc_slot(param);

	compile_node(data.body);

	Opcode lastOpcode = static_cast<Opcode>(currentFn->chunk->code.back());
	if (lastOpcode != Opcode::RETURN)
	{
		int nullSlot = add_constant_to_chunk(Value(NullValue{}));
		emit_opcode(Opcode::LOAD_CONST);
		emit_operand(nullSlot);
		emit_opcode(Opcode::RETURN);
	}

	env.end_scope();
	Value fnData = Value(ValueKind::VALUE_FN, ValueData(std::in_place_type<Function*>, currentFn));
	currentFn = module->root;

	// check for global scope so we can add this function to the modules globals
	if (env.get_scope_depth() == 0 && classDepth == 0)
		add_global_at_slot(fnData, static_cast<uint16_t>(fnSlot));
}

void Codegen::emit_method(ASTNode* node)
{
	// methods are a bit different than functions
	// 1. we dont need to emit an entry / register the function with the environment
	// 2. before we add entries for the params, we need to reserve the first slot for 'this'
	// 3. instead of storing the function in the global scope, we add it to the classDecl's methods table

	const ASTFnDecl& data = std::get<ASTFnDecl>(node->data);
	currentFn = make_fn(data.identifier, data.params.size(), 0);
	env.new_scope(ScopeKind::FnLevel);

	alloc_slot("$this");
	for (const std::string& param : data.params)
		alloc_slot(param);

	compile_node(data.body);
	currentFn->locals = env.current_scope_slot_count() - data.params.size() - 1; // extra -1 to account for hidden 'this' parameter
	env.end_scope();

	currentClass->methods[data.identifier] = currentFn;
	currentFn = module->root;
}

void Codegen::emit_field_decl(ASTNode* node)
{
	const ASTVarDecl& data = std::get<ASTVarDecl>(node->data);

	uint16_t slot = static_cast<uint16_t>(currentClass->fields.size());
	currentClass->fields[data.identifier] = slot;
}

void Codegen::emit_var_decl(ASTNode* node)
{
	const ASTVarDecl& data = std::get<ASTVarDecl>(node->data);

	// check for duplicate variable names, we need to check two things:
	// if the slot returned is valid AND the scope is the same as our current scope, we have a duplicate declaration
	// this allows variable shadowing by allowing deeper scopes to declare variables with the same identifiers as outer scopes
	EnvEntry entry = env.resolve_entry(data.identifier);
	if (entry.slot != -1 && entry.scope == env.get_scope_depth())
		ctx->reporter.submit_diagnostic({ Phase::Codegen, "duplicate variable declaration '" + data.identifier + "'", node->line, node->column });

	// compile the initializer first which will push the initialization value onto the stack
	compile_node(data.initializer);

	// add the variable to the environment and get its slot index
	int slot = alloc_slot(data.identifier);

	// scope depth is 0, we need to emit the STORE_GLOBAL opcodes
	if (env.get_scope_depth() == 0)
	{
		emit_opcode(Opcode::STORE_GLOBAL);
		emit_operand(static_cast<uint16_t>(slot));
	}
	else
	{
		emit_opcode(Opcode::STORE_LOCAL);
		emit_operand(static_cast<uint16_t>(slot));
	}
}

void Codegen::patch_loop_context(LoopContext& loopCtx)
{
	// each offset is an address into the current chunks bytecode where we need to fill in a temp jump value
	// break jumps always jump to the end address of the loop, which is chunk.code.size(), and continue jumps
	// to the start of the loop, which is contained within the loop context
	Chunk* curr = get_current_chunk();

	uint16_t endAddress = static_cast<uint16_t>(curr->code.size());
	for (uint16_t address : loopCtx.breakJumps)
	{
		curr->code[address] = static_cast<uint8_t>(endAddress & 0xFF);
		curr->code[address + 1] = static_cast<uint8_t>((endAddress >> 8) & 0xFF);
	}

	for (uint16_t address : loopCtx.continueJumps)
	{
		curr->code[address] = static_cast<uint8_t>(loopCtx.startAddress & 0xFF);
		curr->code[address + 1] = static_cast<uint8_t>((loopCtx.startAddress >> 8) & 0xFF);
	}
}

void Codegen::patch_jump(uint16_t address)
{
	Chunk* curr = get_current_chunk();
	uint16_t target = static_cast<uint16_t>(curr->code.size());
	curr->code[address] = static_cast<uint8_t>(target & 0xFF);
	curr->code[address + 1] = static_cast<uint8_t>((target >> 8) & 0xFF);
}

Function* Codegen::make_fn(const std::string& name, uint16_t argc, uint16_t localsCount)
{
	Function* fn = ctx->arena.alloc<Function>();
	fn->name = name;
	fn->argc = argc;
	fn->locals = localsCount;
	fn->chunk = ctx->arena.alloc<Chunk>();

	return fn;
}

void Codegen::add_global_at_slot(const Value& value, uint16_t slot)
{
	// resize and insert directly at slot to preserve slot indices of global variables and functions
	if (module->globals.size() <= slot)
		module->globals.resize(slot + 1);

	module->globals[slot] = value;
}

uint16_t Codegen::add_constant_to_chunk(const Value& value)
{
	Chunk* curr = get_current_chunk();

	// first check if we have this constant already stored
	for (size_t i = 0; i < curr->constants.size(); i++)
	{
		if (value.data == curr->constants[i].data)
			return static_cast<uint16_t>(i);
	}

	curr->constants.push_back(value);
	return static_cast<uint16_t>(curr->constants.size() - 1);
}

Chunk* Codegen::get_current_chunk()
{
	// check if currentFn is null, if it is, reach into the main Module for this compilation unit and use that function instead
	return currentFn ? currentFn->chunk : module->root->chunk;
}

bool Codegen::class_declared_in_module(const std::string& className)
{
	for (ClassDecl* decl : module->classes)
	{
		if (decl->name == className)
			return true;
	}

	return false;
}

int Codegen::alloc_slot(const std::string& identifier)
{
	int slot = env.add_entry(identifier);

	// if we're in a function, we need to track the max local depth to know how much stack space we need for the whole function
	if (currentFn)
	{
		uint16_t totalSlots = env.current_scope_slot_count();
		std::cout << "alloc slot for: " << identifier << " slot [" << slot << "]" << "  -->  totalSlots: " << totalSlots << ", currentFn->locals" << currentFn->locals << std::endl;
		currentFn->locals = std::max(currentFn->locals, static_cast<uint16_t>(slot + 1));
	}

	return slot;
}

void Codegen::compile_node(ASTNode* node)
{
	switch (node->kind)
	{
		case ASTKind::AST_INT_LITERAL:
		{
			uint16_t constIndex = add_constant_to_chunk(Value(ValueKind::VALUE_INT, ValueData(std::in_place_type<int64_t>, std::get<ASTIntLiteral>(node->data).value)));
			emit_opcode(Opcode::LOAD_CONST);
			emit_operand(constIndex);
			break;
		}

		case ASTKind::AST_FLOAT_LITERAL:
		{
			uint16_t constIndex = add_constant_to_chunk(Value(ValueKind::VALUE_FLOAT, ValueData(std::in_place_type<double>, std::get<ASTFloatLiteral>(node->data).value)));
			emit_opcode(Opcode::LOAD_CONST);
			emit_operand(constIndex);
			break;
		}

		case ASTKind::AST_STRING_LITERAL:
		{
			uint16_t constIndex = add_constant_to_chunk(Value(ValueKind::VALUE_STRING, ValueData(std::in_place_type<std::string>, std::get<ASTStringLiteral>(node->data).value)));
			emit_opcode(Opcode::LOAD_CONST);
			emit_operand(constIndex);
			break;
		}

		case ASTKind::AST_BOOL_LITERAL:
		{
			uint16_t constIndex = add_constant_to_chunk(Value(ValueKind::VALUE_BOOL, ValueData(std::in_place_type<bool>, std::get<ASTBoolLiteral>(node->data).value)));
			emit_opcode(Opcode::LOAD_CONST);
			emit_operand(constIndex);
			break;
		}

		case ASTKind::AST_NULL:
		{
			uint16_t constIndex = add_constant_to_chunk(Value(ValueKind::VALUE_NULL, ValueData(std::in_place_type<NullValue>)));
			emit_opcode(Opcode::LOAD_CONST);
			emit_operand(constIndex);
			break;
		}

		case ASTKind::AST_VAR_DECL:
		{
			// if we're in a class declaration, but not a method, then we need to populate the classDecl's field map
			if (currentClass && !currentFn)
				emit_field_decl(node);

			// otherwise, a variable declaration in a method or normal function is the same
			else
				emit_var_decl(node);

			break;
		}

		case ASTKind::AST_FN_DECL:
		{
			if (currentClass)
				emit_method(node);
			else
				emit_function(node);

			break;
		}

		case ASTKind::AST_BLOCK:
		{
			// blocks aren't responsible for creating their scopes since bare blocks aren't allowed.
			// this allows each construct that uses blocks to explicitly create their own scope
			// this also means that blocks are only responsible for compiling each node they contain
			const ASTBlock& data = std::get<ASTBlock>(node->data);

			for (ASTNode* stmt : data.statements)
				compile_node(stmt);

			break;
		}

		case ASTKind::AST_IDENTIFIER:
		{
			const ASTIdentifier& data = std::get<ASTIdentifier>(node->data);

			// check for sentinel value that signifies the identifier was not found
			EnvEntry entry = env.resolve_entry(data.identifier);
			if (entry.scope == -1 && entry.slot == -1)
			{
				ctx->reporter.submit_diagnostic(Diagnostic{ Phase::Codegen, "undefined identifier \"" + data.identifier + "\"", node->line, node->column });
				break;
			}

			emit_load_for_entry(entry);
			break;
		}

		case ASTKind::AST_ASSIGNMENT:
		{
			const ASTAssignment& data = std::get<ASTAssignment>(node->data);

			// normal assignment, we need to compile the right hand side to push the value onto the stack
			if (data.op == TokenKind::TOKEN_EQUALS)
			{
				compile_node(data.rhs);
			}

			// compound assignment, all can be handled in the same pattern of load var, push rhs value, push compound add, sub, etc op
			else
			{
				emit_load_lhs(data.lhs);
				compile_node(data.rhs);
				emit_compound_assignment_op(data.op);
			}

			// at the end of it all, all assignments need a store
			emit_store_lhs(data.lhs);
			break;
		}

		case ASTKind::AST_RETURN:
		{
			const ASTReturn& data = std::get<ASTReturn>(node->data);

			if (data.returnExpr)
			{
				compile_node(data.returnExpr);
				emit_opcode(Opcode::RETURN);
			}
			else
			{
				emit_opcode(Opcode::NULL_RETURN);
			}

			break;
		}

		case ASTKind::AST_IF:
		{
			// compile condition, if it's false, jump over true branch to false branch
			// if its true, jmp doesnt fire, we execute true branch, then jmp over false branch
			const ASTIf& data = std::get<ASTIf>(node->data);

			compile_node(data.condition);
			emit_opcode(Opcode::JMP_IF_FALSE);
			uint16_t elseJump = emit_operand(0xFFFF);
			emit_opcode(Opcode::POP);
			compile_scoped_node(data.trueBranch);

			if (data.falseBranch)
			{
				emit_opcode(Opcode::JMP);
				uint16_t endJump = emit_operand(0xFFFF);
				patch_jump(elseJump);
				emit_opcode(Opcode::POP);
				compile_scoped_node(data.falseBranch);
				patch_jump(endJump);
			}
			else
			{
				patch_jump(elseJump);
				emit_opcode(Opcode::POP);
			}
			
			break;
		}

		case ASTKind::AST_WHILE:
		{
			const ASTWhile& data = std::get<ASTWhile>(node->data);

			uint16_t startAddress = get_current_chunk()->code.size();
			loopStack.push_back({ startAddress });

			compile_node(data.condition);
			emit_opcode(Opcode::JMP_IF_FALSE);
			uint16_t endJump = emit_operand(0xFFFF);
			emit_opcode(Opcode::POP);

			compile_scoped_node(data.body);
			emit_opcode(Opcode::JMP);
			emit_operand(startAddress);

			patch_jump(endJump);
			emit_opcode(Opcode::POP);

			patch_loop_context(loopStack.back());
			loopStack.pop_back();

			break;
		}

		case ASTKind::AST_FOR:
		{
			const ASTFor& data = std::get<ASTFor>(node->data);

			loopStack.push_back({ 0 });
			env.new_scope(ScopeKind::Normal);

			int iteratorSlot = alloc_slot(data.iterator);

			// compile the iterable <range> or <collection_based>
			IterContext iterCtx = compile_iterable_node(data.iterable, static_cast<uint16_t>(iteratorSlot));

			// need to emit cond_check -> body -> increment
			emit_iterable_condition_check(iterCtx);
			compile_node(data.body);
			emit_iterable_increment(iterCtx);

			patch_loop_context(loopStack.back());
			loopStack.pop_back();
			env.end_scope();
			break;
		}

		case ASTKind::AST_RANGE:
		{
			ctx->reporter.submit_diagnostic((Diagnostic{ Phase::Codegen, "unexpected range found in codegen", node->line, node->column }));
			break;
		}

		case ASTKind::AST_CLASS_DECL:
		{
			const ASTClassDecl& data = std::get<ASTClassDecl>(node->data);
			if (class_declared_in_module(data.identifier))
			{
				ctx->reporter.submit_diagnostic((Diagnostic{ Phase::Codegen, "duplicate class identifier \"" + data.identifier + "\"", node->line, node->column}));
				break;
			}

			currentClass = ctx->arena.alloc<ClassDecl>(data.identifier);
			module->classes.push_back(currentClass);

			for (ASTNode* member : data.members)
				compile_node(member);

			currentClass = nullptr;
			break;
		}

		case ASTKind::AST_LOGICAL_EXPR:
		{
			const ASTLogicalExpr& data = std::get<ASTLogicalExpr>(node->data);

			// always need to compile the lhs of the expression first
			compile_node(data.lhs);

			if (data.op == TokenKind::TOKEN_OR)
			{
				// to short circuit <or> expressions, we need to jmp over the evaluation of rhs if lhs is truthy
				// we don't know the address to jump to now, so we emit a placeholder address and backpatch it after 
				// emitting code for the rhs
				emit_opcode(Opcode::JMP_IF_TRUE);
				uint16_t address = emit_operand(0xFFFF);
				emit_opcode(Opcode::POP);
				compile_node(data.rhs);
				patch_jump(address);
			}
			else
			{
				// same basic logic for <and> expressions, except we jmp over the evaluation of rhs if rhs is falsey
				emit_opcode(Opcode::JMP_IF_FALSE);
				uint16_t address = emit_operand(0xFFFF);
				emit_opcode(Opcode::POP);
				compile_node(data.rhs);
				patch_jump(address);
			}

			break;
		}

		case ASTKind::AST_BINARY_EXPR:
		{
			const ASTBinaryExpr& data = std::get<ASTBinaryExpr>(node->data);
			compile_node(data.lhs);
			compile_node(data.rhs);
			emit_binary_op(data.op);
			break;
		}

		case ASTKind::AST_UNARY_EXPR:
		{
			const ASTUnaryExpr& data = std::get<ASTUnaryExpr>(node->data);
			compile_node(data.expr);
			emit_unary_op(data.op);
			break;
		}

		case ASTKind::AST_FN_CALL:
		{
			const ASTFnCall& data = std::get<ASTFnCall>(node->data);

			compile_node(data.callee);

			for (ASTNode* arg : data.arguments)
				compile_node(arg);

			emit_opcode(Opcode::CALL_FN);
			emit_operand(data.arguments.size());

			break;
		}

		case ASTKind::AST_ARRAY_ACCESS:
		{
			const ASTArrayAccess& data = std::get<ASTArrayAccess>(node->data);
			compile_node(data.arr);
			compile_node(data.index);
			emit_opcode(Opcode::ARRAY_LOAD);
			break;
		}

		case ASTKind::AST_FIELD_ACCESS:
		{
			const ASTFieldAccess& data = std::get<ASTFieldAccess>(node->data);

			compile_node(data.object);
			uint16_t fieldIndex = add_constant_to_chunk(Value(ValueKind::VALUE_STRING, ValueData(std::in_place_type<std::string>, data.field)));
			emit_opcode(Opcode::LOAD_FIELD);
			emit_operand(fieldIndex);

			break;
		}

		case ASTKind::AST_BREAK:
		{
			emit_opcode(Opcode::JMP);
			loopStack.back().breakJumps.push_back(emit_operand(0xFFFF));
			break;
		}

		case ASTKind::AST_CONTINUE:
		{
			emit_opcode(Opcode::JMP);
			loopStack.back().continueJumps.push_back(emit_operand(0xFFFF));
			break;
		}

		case ASTKind::AST_THIS:
		{
			// the this implicit parameter is always placed into slot 0 of a methods call frame by the vm
			emit_opcode(Opcode::LOAD_LOCAL);
			emit_operand(0);
			break;
		}

		case ASTKind::AST_EXPR_STMT:
		{
			const ASTExprStmt& data = std::get<ASTExprStmt>(node->data);
			compile_node(data.expr);
			emit_opcode(Opcode::POP);
			break;
		}
	}
}

void Codegen::compile_scoped_node(ASTNode* body)
{
	// helper to compile potential ASTBlocks in control flow constructs like if, while, and for
	// checks if the node is a block, if it is, it pushes a scope and compiles the body, if not it just 
	// compiles the body
	if (body->kind == ASTKind::AST_BLOCK)
		env.new_scope(ScopeKind::Normal);

	compile_node(body);

	if (body->kind == ASTKind::AST_BLOCK)
		env.end_scope();
}

IterContext Codegen::compile_iterable_node(ASTNode* node, uint16_t iteratorSlot)
{
	IterContext iterCtx;
	// handle range based iterable
	if (node->kind == ASTKind::AST_RANGE)
	{
		int scopeDepth = env.get_scope_depth();

		RangeIterContext& range = iterCtx.emplace<RangeIterContext>();
		const ASTRange& data = std::get<ASTRange>(node->data);

		// set up the iterator
		compile_node(data.start);
		emit_opcode(Opcode::STORE_LOCAL);
		emit_operand(iteratorSlot);

		// eval end once and store in hidden compiler variable
		std::cout << "$range_iter_end" + std::to_string(scopeDepth) << std::endl;
		int endSlot = alloc_slot("$range_iter_end" + std::to_string(scopeDepth));
		compile_node(data.end);
		emit_opcode(Opcode::STORE_LOCAL);
		emit_operand(endSlot);

		int stepSlot = -1;

		// check to see if a step is included in the range since it's not required
		if (data.step)
		{
			std::cout << "$range_iter_step" + std::to_string(scopeDepth) << std::endl;
			stepSlot = alloc_slot("$range_iter_step" + std::to_string(scopeDepth));
			compile_node(data.step);
			emit_opcode(Opcode::STORE_LOCAL);
			emit_operand(stepSlot);
		}

		// fill range in range iter context fields
		range.conditionAddress = get_current_chunk()->code.size();
		range.stepSlot = stepSlot;
		range.iterSlot = iteratorSlot;
		range.endSlot = endSlot;
	}

	// todo: handle collection based iterable
	else
	{

	}

	return iterCtx;
}