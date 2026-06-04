#include <iostream>

#include "parser/parser.h"
#include "utils/debug_utils.h"

void Parser::parse_tokens(std::vector<Token>* tokens)
{
	this->currentIndex = 0;
	this->tokens = tokens;
	this->inFnDecl = false;
	this->inClassDecl = false;

	while (tokens_left() && peek() != TokenKind::TOKEN_EOF)
	{
		ASTNode* node = statement();

		if (node)
			ast.emplace_back(node);
	}
}

ASTNode* Parser::statement()
{
	try
	{
		if (peek() == TokenKind::TOKEN_VAR) return var_decl();
		if (peek() == TokenKind::TOKEN_CLASS) return class_decl();
		if (peek() == TokenKind::TOKEN_IF) return if_stmt();
		if (peek() == TokenKind::TOKEN_FOR) return for_stmt();
		if (peek() == TokenKind::TOKEN_WHILE) return while_stmt();
		if (peek() == TokenKind::TOKEN_FN)  return fn_decl();
		if (peek() == TokenKind::TOKEN_RETURN) return return_stmt();
		if (peek() == TokenKind::TOKEN_BREAK) return break_stmt();
		if (peek() == TokenKind::TOKEN_CONTINUE) return continue_stmt();
		if (peek() == TokenKind::TOKEN_L_BRACE) return block();
	}
	catch (const ParseError& error)
	{
		synchronize();
		return nullptr;
	}

	return expr_stmt();
}

ASTNode* Parser::var_decl()
{
	// syntax: 
	// 'var' <identifier> '=' <expression>
	// <expression> is required in vanta, so you can't declare a variable with no value
	// this value can be anything though, a number, string, float, null, etc

	// advance through the 'var' keyword
	Token* start = advance(); 

	// assert that an identifier and '=' tokens are found and advance through them
	Token* identifier = assert_current(TokenKind::TOKEN_IDENTIFIER, "Expect identifier in variable declaration");
	assert_current(TokenKind::TOKEN_EQUALS, "Expect '=' in variable declaration");

	// get the initializer value for the variable and assert that a semicolon terminates the instruction
	ASTNode* initializer = expression();
	assert_current(TokenKind::TOKEN_SEMICOLON, "Expect ';' after variable declaration");

	return arena.alloc<ASTNode>(
		ASTKind::AST_VAR_DECL,
		start->line,
		start->column,
		ASTData(std::in_place_type<ASTVarDecl>, identifier->tokenLiteral, initializer));
}

ASTNode* Parser::fn_decl()
{
	// syntax:
	// 'fn' <identifier> '( <param_list> ') <block>

	// parser flag to detect when nested functions are found which are not allowed
	if (inFnDecl)
	{
		inFnDecl = false;
		throw_error("Nested functions are not allowed");
	}

	// advance through the 'fn' keyword
	inFnDecl = true;
	Token* start = advance();

	// get the function identifier, and assert that there's an opening parenthesis
	Token* identifier = assert_current(TokenKind::TOKEN_IDENTIFIER, "Expect function name in function declaration");
	assert_current(TokenKind::TOKEN_L_PAREN, "Expect '(' to start paramater list in function declaration");

	// collect all parameters (if there are any)
	std::vector<std::string> params;
	while (peek() == TokenKind::TOKEN_IDENTIFIER)
	{
		Token* paramIdentifier = advance();
		params.emplace_back(paramIdentifier->tokenLiteral);

		if (peek() == TokenKind::TOKEN_COMMA)
			advance();
	}

	// assert a closing parenthesis to the parameter list and get the body of the function
	assert_current(TokenKind::TOKEN_R_PAREN, "Expect ')' to close parameter list in function declaration");
	ASTNode* body = block();
	inFnDecl = false;

	return arena.alloc<ASTNode>(
		ASTKind::AST_FN_DECL,
		start->line,
		start->column,
		ASTData(std::in_place_type<ASTFnDecl>, identifier->tokenLiteral, std::move(params), body));
}

ASTNode* Parser::class_decl()
{
	if (inClassDecl)
	{
		inClassDecl = false;
		throw_error("Nested classes are not allowed");
	}

	inClassDecl = true;

	// syntax: 'class' <identifier> '{' [<var_decl> | <fn_decl>]* '}'
	Token* start = advance();
	Token* identifier = assert_current(TokenKind::TOKEN_IDENTIFIER, "Expect identifier in class declaration");
	assert_current(TokenKind::TOKEN_L_BRACE, "Expect '{' before class methods and fields");

	// collect all var and fn declarations within the class, and break as soon as we find a token that's not one of those things
	std::vector<ASTNode*> members;
	while (tokens_left())
	{
		if (peek() == TokenKind::TOKEN_VAR)
			members.push_back(var_decl());
		else if (peek() == TokenKind::TOKEN_FN)
			members.push_back(fn_decl());
		else
			break;
	}

	assert_current(TokenKind::TOKEN_R_BRACE, "Expect '}' after class methods and fields");
	inClassDecl = false;

	return arena.alloc<ASTNode>(
		ASTKind::AST_CLASS_DECL,
		start->line,
		start->column,
		ASTData(std::in_place_type<ASTClassDecl>, identifier->tokenLiteral, std::move(members)));
}

ASTNode* Parser::if_stmt()
{
	// syntax: 'if' <expression> <statement> ['else' <statement>]?
	// advance through 'if' token, collect the condition, collect the true branch, and by default the false branch is null since it's not required
	Token* start = advance();
	ASTNode* condition = expression();
	ASTNode* trueBranch = statement();
	ASTNode* falseBranch = nullptr;

	// check to see if there's an else branch
	if (peek() == TokenKind::TOKEN_ELSE)
	{
		advance();
		falseBranch = statement();
	}

	return arena.alloc<ASTNode>(
		ASTKind::AST_IF,
		start->column,
		start->line,
		ASTData(std::in_place_type<ASTIf>, condition, trueBranch, falseBranch));
}

ASTNode* Parser::while_stmt()
{
	// syntax: 'while' <expression> <statement>
	Token* start = advance();
	ASTNode* condition = expression();
	ASTNode* body = statement();

	return arena.alloc<ASTNode>(
		ASTKind::AST_WHILE,
		start->line,
		start->column,
		ASTData(std::in_place_type<ASTWhile>, condition, body));
}

ASTNode* Parser::for_stmt()
{
	// syntax: 'for' <identifier> 'in' <for_iterable> <statement>
	Token* start = advance();
	Token* iterator = assert_current(TokenKind::TOKEN_IDENTIFIER, "Expect iterator identifier in for loop");

	assert_current(TokenKind::TOKEN_IN, "Expect 'in' when declaring for loop");

	ASTNode* iterable = for_iterable();
	ASTNode* body = statement();

	return arena.alloc<ASTNode>(
		ASTKind::AST_FOR,
		start->line,
		start->column,
		ASTData(std::in_place_type<ASTFor>, iterator->tokenLiteral, iterable, body));
}

ASTNode* Parser::for_iterable()
{
	// an iterable can be either
	//    1. range expression in the form of start..end..[increment]? where increment is optional. 0..10 and 0..10..2 are valid
	//       where the syntax is <expression> .. <expression> [.. <expression>]? 
	//    2. collection based, this can be many forms like an identifier to a collection, a hardcoded collection, etc

	ASTNode* expr = expression();

	// if after parsing the first expression of the iterable we find a range, we know its a range based for loop
	if (peek() == TokenKind::TOKEN_RANGE)
	{
		Token* rangeStart = advance();
		ASTNode* end = expression();
		ASTNode* step = nullptr;

		// the step is optional, so we need to check for another range here
		if (peek() == TokenKind::TOKEN_RANGE)
		{
			advance();
			step = expression();
		}

		return arena.alloc<ASTNode>(
			ASTKind::AST_RANGE,
			rangeStart->line,
			rangeStart->column,
			ASTData(std::in_place_type<ASTRange>, expr, end, step));
	}

	// otherwise, the iterable is just a collection so we can return expr directly here
	return expr;
}

ASTNode* Parser::return_stmt()
{
	// advance through the 'return' keyword
	Token* start = advance();

	// return is by default null to account for empty returns
	ASTNode* returnExpr = nullptr;
	if (peek() != TokenKind::TOKEN_SEMICOLON)
		returnExpr = expression();

	assert_current(TokenKind::TOKEN_SEMICOLON, "Expect ';' after return");

	return arena.alloc<ASTNode>(
		ASTKind::AST_RETURN,
		start->line,
		start->column,
		ASTData(std::in_place_type<ASTReturn>, returnExpr));
}

ASTNode* Parser::break_stmt()
{
	Token* start = advance();
	assert_current(TokenKind::TOKEN_SEMICOLON, "Expect ';' after break");
	return arena.alloc<ASTNode>(
		ASTKind::AST_BREAK,
		start->line,
		start->column,
		ASTData(std::in_place_type<ASTBreak>));
}

ASTNode* Parser::continue_stmt()
{
	Token* start = advance();
	assert_current(TokenKind::TOKEN_SEMICOLON, "Expect ';' after continue");
	return arena.alloc<ASTNode>(
		ASTKind::AST_CONTINUE,
		start->line,
		start->column,
		ASTData(std::in_place_type<ASTContinue>));
}

ASTNode* Parser::expr_stmt()
{
	ASTNode* expr = expression();
	assert_current(TokenKind::TOKEN_SEMICOLON, "Expect ';' after expression");
	return expr;
}

ASTNode* Parser::block()
{
	Token* start = assert_current(TokenKind::TOKEN_L_BRACE, "Expect '{' to begin block");

	std::vector<ASTNode*> statements;
	while (tokens_left() && peek() != TokenKind::TOKEN_R_BRACE)
	{
		ASTNode* stmt = statement();

		if (stmt)
			statements.emplace_back(stmt);
	}

	assert_current(TokenKind::TOKEN_R_BRACE, "Expect '}' to close block");
	return arena.alloc<ASTNode>(
		ASTKind::AST_BLOCK,
		start->line,
		start->column,
		std::move(statements));
}

ASTNode* Parser::expression()
{
	return assignment();
}

ASTNode* Parser::assignment()
{
	// syntax: <assignment> ::= <identifier> <assignment_operator> <assignment>
	//	| <logical_or>

	// we dont know if this is an assignment yet, so we parse the lhs as a normal expression down the production chain
	ASTNode* lhs = logical_or();

	// then we check if there's an assignment operator, if there is, we found an assignment, if not, we just return the lhs expression we parsed
	if (peek_is_assignment_op())
	{
		Token* op = advance();
		ASTNode* rhs = assignment();

		return arena.alloc<ASTNode>(
			ASTKind::AST_ASSIGNMENT,
			lhs->line,
			lhs->column,
			ASTData(std::in_place_type<ASTAssignment>, lhs, op->kind, rhs));
	}

	return lhs;
}

ASTNode* Parser::logical_or()
{
	ASTNode* lhs = logical_and();

	while (tokens_left() && peek() == TokenKind::TOKEN_OR)
	{
		Token* op = advance();
		ASTNode* rhs = logical_and();
		lhs = arena.alloc<ASTNode>(
			ASTKind::AST_LOGICAL_EXPR,
			lhs->line,
			lhs->column,
			ASTData(std::in_place_type<ASTLogicalExpr>, lhs, op->kind, rhs));
	}

	return lhs;
}

ASTNode* Parser::logical_and()
{
	ASTNode* lhs = equality();

	while (tokens_left() && peek() == TokenKind::TOKEN_AND)
	{
		Token* op = advance();
		ASTNode* rhs = equality();
		lhs = arena.alloc<ASTNode>(
			ASTKind::AST_LOGICAL_EXPR,
			lhs->line,
			lhs->column,
			ASTData(std::in_place_type<ASTLogicalExpr>, lhs, op->kind, rhs));
	}

	return lhs;
}

ASTNode* Parser::equality()
{
	ASTNode* lhs = comparison();

	while (tokens_left() && peek_is_equality_op())
	{
		Token* op = advance();
		ASTNode* rhs = comparison();
		lhs = arena.alloc<ASTNode>(
			ASTKind::AST_BINARY_EXPR,
			lhs->line,
			lhs->column,
			ASTData(std::in_place_type<ASTBinaryExpr>, lhs, op->kind, rhs));
	}

	return lhs;
}

ASTNode* Parser::comparison()
{
	ASTNode* lhs = bitwise_or();

	while (tokens_left() && peek_is_comparison_op())
	{
		Token* op = advance();
		ASTNode* rhs = bitwise_or();
		lhs = arena.alloc<ASTNode>(
			ASTKind::AST_BINARY_EXPR,
			lhs->line,
			lhs->column,
			ASTData(std::in_place_type<ASTBinaryExpr>, lhs, op->kind, rhs));
	}

	return lhs;
}

ASTNode* Parser::bitwise_or()
{
	ASTNode* lhs = bitwise_xor();

	while (tokens_left() && peek() == TokenKind::TOKEN_BITWISE_OR)
	{
		Token* op = advance();
		ASTNode* rhs = bitwise_xor();
		lhs = arena.alloc<ASTNode>(
			ASTKind::AST_BINARY_EXPR,
			lhs->line,
			lhs->column,
			ASTData(std::in_place_type<ASTBinaryExpr>, lhs, op->kind, rhs));
	}

	return lhs;
}

ASTNode* Parser::bitwise_xor()
{
	ASTNode* lhs = bitwise_and();

	while (tokens_left() && peek() == TokenKind::TOKEN_BITWISE_XOR)
	{
		Token* op = advance();
		ASTNode* rhs = bitwise_and();
		lhs = arena.alloc<ASTNode>(
			ASTKind::AST_BINARY_EXPR,
			lhs->line,
			lhs->column,
			ASTData(std::in_place_type<ASTBinaryExpr>, lhs, op->kind, rhs));
	}

	return lhs;
}

ASTNode* Parser::bitwise_and()
{
	ASTNode* lhs = bitwise_shift();

	while (tokens_left() && peek() == TokenKind::TOKEN_BITWISE_AND)
	{
		Token* op = advance();
		ASTNode* rhs = bitwise_shift();
		lhs = arena.alloc<ASTNode>(
			ASTKind::AST_BINARY_EXPR,
			lhs->line,
			lhs->column,
			ASTData(std::in_place_type<ASTBinaryExpr>, lhs, op->kind, rhs));
	}

	return lhs;
}

ASTNode* Parser::bitwise_shift()
{
	ASTNode* lhs = term();

	while (tokens_left() && peek_is_bitwise_shift_op())
	{
		Token* op = advance();
		ASTNode* rhs = term();
		lhs = arena.alloc<ASTNode>(
			ASTKind::AST_BINARY_EXPR,
			lhs->line,
			lhs->column,
			ASTData(std::in_place_type<ASTBinaryExpr>, lhs, op->kind, rhs));
	}

	return lhs;
}

ASTNode* Parser::term()
{
	ASTNode* lhs = factor();

	while (tokens_left() && peek_is_term_op())
	{
		Token* op = advance();
		ASTNode* rhs = factor();
		lhs = arena.alloc<ASTNode>(
			ASTKind::AST_BINARY_EXPR,
			lhs->line,
			lhs->column,
			ASTData(std::in_place_type<ASTBinaryExpr>, lhs, op->kind, rhs));
	}

	return lhs;
}

ASTNode* Parser::factor()
{
	ASTNode* lhs = power();

	while (tokens_left() && peek_is_factor_op())
	{
		Token* op = advance();
		ASTNode* rhs = power();
		lhs = arena.alloc<ASTNode>(
			ASTKind::AST_BINARY_EXPR,
			lhs->line,
			lhs->column,
			ASTData(std::in_place_type<ASTBinaryExpr>, lhs, op->kind, rhs));
	}

	return lhs;
}

ASTNode* Parser::power()
{
	ASTNode* base = unary();

	if (peek() == TokenKind::TOKEN_POWER)
	{
		Token* op = advance();
		ASTNode* exponent = power();
		return arena.alloc<ASTNode>(
			ASTKind::AST_BINARY_EXPR,
			base->line,
			base->column,
			ASTData(std::in_place_type<ASTBinaryExpr>, base, op->kind, exponent));
	}

	return base;
}

ASTNode* Parser::unary()
{
	if (peek_is_unary_op())
	{
		Token* op = advance();
		ASTNode* expr = unary();
		return arena.alloc<ASTNode>(
			ASTKind::AST_UNARY_EXPR,
			op->line,
			op->column,
			ASTData(std::in_place_type<ASTUnaryExpr>, op->kind, expr));
	}

	return postfix();
}

ASTNode* Parser::postfix()
{
	ASTNode* expr = primary();

	// need to find and chain instances of postfix operations using (), [], and .
	// this is to be able to support things like foo().field[2]
	while (true)
	{
		if (peek() == TokenKind::TOKEN_L_PAREN)
		{
			Token* start = advance();
			std::vector<ASTNode*> arguments;

			while (tokens_left() && peek() != TokenKind::TOKEN_R_PAREN)
			{
				arguments.push_back(expression());

				if (peek() == TokenKind::TOKEN_COMMA)
					advance();
			}

			assert_current(TokenKind::TOKEN_R_PAREN, "Expect ')' after fn arguments");
			expr = arena.alloc<ASTNode>(
				ASTKind::AST_FN_CALL,
				start->line,
				start->column,
				ASTData(std::in_place_type<ASTFnCall>, expr, std::move(arguments)));
		}
		else if (peek() == TokenKind::TOKEN_L_BRACKET)
		{
			Token* start = advance();
			ASTNode* index = expression();
			assert_current(TokenKind::TOKEN_R_BRACKET, "Expect ']' after array index");
			expr = arena.alloc<ASTNode>(
				ASTKind::AST_ARRAY_ACCESS,
				start->line,
				start->column,
				ASTData(std::in_place_type<ASTArrayAccess>, expr, index));
		}
		else if (peek() == TokenKind::TOKEN_DOT)
		{
			Token* start = advance();
			Token* identifier = assert_current(TokenKind::TOKEN_IDENTIFIER, "Expect identifier when accessing class fields");
			expr = arena.alloc<ASTNode>(
				ASTKind::AST_FIELD_ACCESS,
				start->line,
				start->column,
				ASTData(std::in_place_type<ASTFieldAccess>, expr, identifier->tokenLiteral));
		}
		else
			break;
	}

	return expr;
}

ASTNode* Parser::primary()
{
	if (peek() == TokenKind::TOKEN_INT_LITERAL)
	{
		Token* token = advance();

		return arena.alloc<ASTNode>(
			ASTKind::AST_INT_LITERAL,
			token->line,
			token->column,
			ASTData(std::in_place_type<ASTIntLiteral>, std::stoll(token->tokenLiteral)));
	}

	if (peek() == TokenKind::TOKEN_FLOAT_LITERAL)
	{
		Token* token = advance();

		return arena.alloc<ASTNode>(
			ASTKind::AST_FLOAT_LITERAL,
			token->line,
			token->column,
			ASTData(std::in_place_type<ASTFloatLiteral>, std::stod(token->tokenLiteral)));
	}

	if (peek() == TokenKind::TOKEN_STRING_LITERAL)
	{
		Token* token = advance();

		return arena.alloc<ASTNode>(
			ASTKind::AST_STRING_LITERAL,
			token->line,
			token->column,
			ASTData(std::in_place_type<ASTStringLiteral>, token->tokenLiteral));
	}

	if (peek() == TokenKind::TOKEN_TRUE)
	{
		Token* token = advance();

		return arena.alloc<ASTNode>(
			ASTKind::AST_BOOL_LITERAL,
			token->line,
			token->column,
			ASTData(std::in_place_type<ASTBoolLiteral>, true));
	}

	if (peek() == TokenKind::TOKEN_FALSE)
	{
		Token* token = advance();

		return arena.alloc<ASTNode>(
			ASTKind::AST_BOOL_LITERAL,
			token->line,
			token->column,
			ASTData(std::in_place_type<ASTBoolLiteral>, false));
	}

	if (peek() == TokenKind::TOKEN_NULL)
	{
		Token* token = advance();

		return arena.alloc<ASTNode>(
			ASTKind::AST_NULL,
			token->line,
			token->column,
			ASTData(std::in_place_type<ASTNull>));
	}

	if (peek() == TokenKind::TOKEN_IDENTIFIER)
	{
		Token* token = advance();

		return arena.alloc<ASTNode>(
			ASTKind::AST_IDENTIFIER,
			token->line,
			token->column,
			ASTData(std::in_place_type<ASTIdentifier>, token->tokenLiteral));
	}

	if (peek() == TokenKind::TOKEN_L_PAREN)
	{
		advance();
		ASTNode* expr = expression();
		assert_current(TokenKind::TOKEN_R_PAREN, "Expect ')' after expression");
		return expr;
	}

	if (peek() == TokenKind::TOKEN_THIS)
	{
		Token* start = advance();
		return arena.alloc<ASTNode>(
			ASTKind::AST_THIS,
			start->line,
			start->column,
			ASTData(std::in_place_type<ASTThis>));
	}

	throw_error("Expected valid expression");
	return nullptr;
}

TokenKind Parser::peek()
{
	return (*tokens)[currentIndex].kind;
}

Token* Parser::advance()
{
	if (!tokens_left())
		return nullptr;

	Token* ret = &((*tokens)[currentIndex]);
	currentIndex++;
	return ret;
}

Token* Parser::prev_token()
{
	return &((*tokens)[currentIndex - 1]);
}

bool Parser::tokens_left()
{
	return peek() != TokenKind::TOKEN_EOF;
}

bool Parser::peek_is_assignment_op()
{
	TokenKind top = peek();

	return (top == TokenKind::TOKEN_EQUALS || top == TokenKind::TOKEN_PLUS_EQUALS || top == TokenKind::TOKEN_MINUS_EQUALS
		|| top == TokenKind::TOKEN_TIMES_EQUALS || top == TokenKind::TOKEN_DIVIDE_EQUALS || top == TokenKind::TOKEN_MODULO_EQUALS
		|| top == TokenKind::TOKEN_BITWISE_AND_EQUALS || top == TokenKind::TOKEN_BITWISE_OR_EQUALS || top == TokenKind::TOKEN_BITWISE_XOR_EQUALS
		|| top == TokenKind::TOKEN_BITWISE_L_SHIFT_EQUALS || top == TokenKind::TOKEN_BITWISE_R_SHIFT_EQUALS);
}

bool Parser::peek_is_equality_op()
{
	TokenKind top = peek();

	return (top == TokenKind::TOKEN_EQUALITY || top == TokenKind::TOKEN_NOT_EQUALITY);
}

bool Parser::peek_is_comparison_op()
{
	TokenKind top = peek();

	return (top == TokenKind::TOKEN_GT || top == TokenKind::TOKEN_GTE || top == TokenKind::TOKEN_LT || top == TokenKind::TOKEN_LTE);
}

bool Parser::peek_is_bitwise_shift_op()
{
	TokenKind top = peek();

	return (top == TokenKind::TOKEN_BITWISE_L_SHIFT || top == TokenKind::TOKEN_BITWISE_R_SHIFT);
}

bool Parser::peek_is_term_op()
{
	TokenKind top = peek();

	return (top == TokenKind::TOKEN_PLUS || top == TokenKind::TOKEN_MINUS);
}

bool Parser::peek_is_factor_op()
{
	TokenKind top = peek();

	return (top == TokenKind::TOKEN_ASTERISK || top == TokenKind::TOKEN_FORWARD_SLASH || top == TokenKind::TOKEN_MODULO);
}

bool Parser::peek_is_unary_op()
{
	TokenKind top = peek();

	return (top == TokenKind::TOKEN_MINUS || top == TokenKind::TOKEN_NOT || top == TokenKind::TOKEN_BITWISE_NOT);
}

Token* Parser::assert_current(TokenKind kind, const std::string& errorMsg)
{
	if (peek() == kind)
		return advance();
	else
		throw_error(errorMsg);
}

void Parser::throw_error(const std::string& errorMsg)
{
	// if there are tokens left, use the top token as the error token, if not we can settle for the previously consumed token
	Token* error_token = (tokens_left()) ? advance() : prev_token();

	if (error_token)
		std::cout << "Parse error at line: " << error_token->line << " col: " << error_token->column << "\n" << errorMsg << "\n";

	throw ParseError();
}

void Parser::synchronize()
{
	// when a parsing error happens, we need to get back to a working state by advancing through tokens until we reach a semicolon
	// we know that after the semicolon, the start of a new instruction starts
	while (tokens_left() && peek() != TokenKind::TOKEN_SEMICOLON)
		advance();

	// consume the semicolon if there's tokens left
	if (tokens_left())
		advance();
}