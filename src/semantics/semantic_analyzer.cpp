#include <unordered_set>

#include "semantics/semantic_analyzer.h"
#include "utils/debug_utils.h"

SemanticAnalyzer::SemanticAnalyzer()
{
	this->loopDepth = 0;
	this->fnDeclDepth = 0;
	this->classDeclDepth = 0;
	this->ctx = nullptr;
}

void SemanticAnalyzer::analyze(const std::vector<ASTNode*>& ast, PipelineContext* ctx)
{
	this->ctx = ctx;

	for (ASTNode* node : ast)
		analyze_node(node);
}

void SemanticAnalyzer::check_duplicate_class_member(const std::string& identifier, ASTNode* node)
{
	// if we are in a class declaration, check if a member with this identifier already exists, if it does, we have a duplicate name error
	// if not, add it into the classMembers set
	if (classDeclDepth > 0 && fnDeclDepth == 0)
	{
		if (classMembers.count(identifier))
			ctx->reporter.submit_diagnostic({ Phase::Semantic, "duplicate class member '" + identifier + "'", node->line, node->column });
		else
			classMembers.insert(identifier);
	}
}

void SemanticAnalyzer::analyze_node(ASTNode* node)
{
	switch (node->kind)
	{
		// nothing to check
		case ASTKind::AST_INT_LITERAL:
		case ASTKind::AST_FLOAT_LITERAL:
		case ASTKind::AST_STRING_LITERAL:
		case ASTKind::AST_BOOL_LITERAL:
		case ASTKind::AST_NULL:
		case ASTKind::AST_IDENTIFIER:
		case ASTKind::AST_IMPORT_STMT:
			break; 

		case ASTKind::AST_EXPR_STMT:
		{
			const ASTExprStmt& data = std::get<ASTExprStmt>(node->data);
			analyze_node(data.expr);
			break;
		}

		case ASTKind::AST_FN_CALL:
		{
			const ASTFnCall& data = std::get<ASTFnCall>(node->data);
			
			analyze_node(data.callee);

			for (ASTNode* arg : data.arguments)
				analyze_node(arg);

			break;
		}

		case ASTKind::AST_ARRAY:
		{
			const ASTArray& data = std::get<ASTArray>(node->data);
			
			for (ASTNode* element : data.arr)
				analyze_node(element);

			break;
		}

		case ASTKind::AST_DICT:
		{
			const ASTDict& data = std::get<ASTDict>(node->data);

			for (ASTNode* key : data.keys)
				analyze_node(key);

			for (ASTNode* val : data.vals)
				analyze_node(val);

			break;
		}

		case ASTKind::AST_VAR_DECL:
		{
			const ASTVarDecl& data = std::get<ASTVarDecl>(node->data);

			check_duplicate_class_member(data.identifier, node);

			break;
		}

		case ASTKind::AST_FN_DECL:
		{
			const ASTFnDecl& data = std::get<ASTFnDecl>(node->data);

			check_duplicate_class_member(data.identifier, node);

			// check for duplicate parameter names
			fnParams.clear();
			for (const std::string& param : data.params)
			{
				if (fnParams.find(param) != fnParams.end())
					ctx->reporter.submit_diagnostic({ Phase::Semantic, "duplicate parameter '" + param + "' in fn '" + data.identifier + "'", node->line, node->column });

				fnParams.insert(param);
			}

			inConstructor = (classDeclDepth > 0 && data.identifier == currentClassName);
			fnDeclDepth++;

			analyze_node(data.body);

			fnDeclDepth--;
			inConstructor = false;
			break;
		}

		case ASTKind::AST_RETURN:
		{
			const ASTReturn& data = std::get<ASTReturn>(node->data);
			if (fnDeclDepth == 0)
				ctx->reporter.submit_diagnostic({ Phase::Semantic, "cannot return outside of function", node->line, node->column });

			if (inConstructor && data.returnExpr)
				ctx->reporter.submit_diagnostic({ Phase::Semantic, "cannot return a non-null value from constructor", node->line, node->column });

			break;
		}

		case ASTKind::AST_FOR:
		{
			const ASTFor& data = std::get<ASTFor>(node->data);
			loopDepth++;
			analyze_node(data.body);
			loopDepth--;
			break;
		}

		case ASTKind::AST_WHILE:
		{
			const ASTWhile& data = std::get<ASTWhile>(node->data);
			loopDepth++;
			analyze_node(data.body);
			loopDepth--;
			break;
		}

		case ASTKind::AST_CONTINUE:
		{
			if (loopDepth == 0)
				ctx->reporter.submit_diagnostic({ Phase::Semantic, "continue found outside of loop", node->line, node->column });

			break;
		}

		case ASTKind::AST_BREAK:
		{
			if (loopDepth == 0)
				ctx->reporter.submit_diagnostic({ Phase::Semantic, "break found outside of loop", node->line, node->column });

			break;
		}

		case ASTKind::AST_BLOCK:
		{
			const ASTBlock& data = std::get<ASTBlock>(node->data);
			
			for (ASTNode* stmt : data.statements)
				analyze_node(stmt);

			break;
		}

		case ASTKind::AST_THIS:
		{
			if (classDeclDepth == 0 || fnDeclDepth == 0)
				ctx->reporter.submit_diagnostic({ Phase::Semantic, "this found outside of class method", node->line, node->column });

			break;
		}

		case ASTKind::AST_CLASS_DECL:
		{
			const ASTClassDecl& data = std::get<ASTClassDecl>(node->data);

			if (classDeclDepth > 0)
			{
				ctx->reporter.submit_diagnostic({ Phase::Semantic, "nested classes are not allowed", node->line, node->column });
				break;
			}

			classMembers.clear();
			classDeclDepth++;
			currentClassName = data.identifier;

			for (ASTNode* member : data.members)
				analyze_node(member);

			classDeclDepth--;
			currentClassName.clear();
			break;
		}

		case ASTKind::AST_IF:
		{
			const ASTIf& data = std::get<ASTIf>(node->data);
			analyze_node(data.condition);
			analyze_node(data.trueBranch);

			if (data.falseBranch)
				analyze_node(data.falseBranch);

			break;
		}

		case ASTKind::AST_FIELD_ACCESS:
		{
			const ASTFieldAccess& data = std::get<ASTFieldAccess>(node->data);
			analyze_node(data.object);
			break;
		}

		case ASTKind::AST_ARRAY_ACCESS:
		{
			const ASTArrayAccess& data = std::get<ASTArrayAccess>(node->data);
			analyze_node(data.arr);
			analyze_node(data.index);
			break;
		}

		case ASTKind::AST_ASSIGNMENT:
		{
			const ASTAssignment& data = std::get<ASTAssignment>(node->data);
			analyze_node(data.lhs);
			analyze_node(data.rhs);
			break;
		}

		case ASTKind::AST_LOGICAL_EXPR:
		{
			const ASTLogicalExpr& data = std::get<ASTLogicalExpr>(node->data);
			analyze_node(data.lhs);
			analyze_node(data.rhs);
			break;
		}

		case ASTKind::AST_BINARY_EXPR:
		{
			const ASTBinaryExpr& data = std::get<ASTBinaryExpr>(node->data);
			analyze_node(data.lhs);
			analyze_node(data.rhs);
			break;
		}

		case ASTKind::AST_UNARY_EXPR:
		{
			const ASTUnaryExpr& data = std::get<ASTUnaryExpr>(node->data);
			analyze_node(data.expr);
			break;
		}

		case ASTKind::AST_RANGE:
		{
			const ASTRange& data = std::get<ASTRange>(node->data);
			analyze_node(data.start);
			analyze_node(data.end);

			if (data.step)
				analyze_node(data.step);

			break;
		}

		case ASTKind::AST_INSTANTIATION:
		{
			const ASTInstantiation& data = std::get<ASTInstantiation>(node->data);

			for (ASTNode* arg : data.args)
				analyze_node(arg);

			break;
		}

		default:
			ctx->reporter.submit_diagnostic({ Phase::Semantic, "unhandled AST node, enum val: " + std::to_string((int)node->kind) + " kind: " + std::string(Utils::ast_kind_to_string(node->kind)), node->line, node->column});
			break;
	}
}