#include <fstream>
#include <iostream>

#include "test_suite/parser_test_suite.h"
#include "lexer/lexer.h"
#include "parser/parser.h"
#include "utils/debug_utils.h"

ParserTestSuite::ParserTestSuite()
{
    suite.push_back({ "literals",    [this]() { return test_literals(); } });
    suite.push_back({ "arithmetic",    [this]() { return test_literals(); } });
    suite.push_back({ "assignment",    [this]() { return test_literals(); } });
    suite.push_back({ "comparison",    [this]() { return test_literals(); } });
    suite.push_back({ "precedence",    [this]() { return test_literals(); } });
    suite.push_back({ "logical",    [this]() { return test_literals(); } });
    suite.push_back({ "bitwise",    [this]() { return test_literals(); } });
    suite.push_back({ "postfix",    [this]() { return test_literals(); } });
    suite.push_back({ "fn_decl",    [this]() { return test_literals(); } });
    suite.push_back({ "var_decl",    [this]() { return test_literals(); } });
    suite.push_back({ "class_decl",    [this]() { return test_literals(); } });
    suite.push_back({ "if_stmt",    [this]() { return test_literals(); } });
    suite.push_back({ "for_stmt",    [this]() { return test_literals(); } });
    suite.push_back({ "while_stmt",    [this]() { return test_literals(); } });
}

void ParserTestSuite::regenerate_ast_snapshots()
{
    // lex and parse all test files, then write the serialized ast to the appropriate
    for (const auto& test : suite)
    {
        Lexer lexer;
        Parser parser;
        PipelineContext ctx;

        lexer.lex_file(PROJECT_ROOT"/tests/parser/" + test.name + ".va");
        parser.parse_tokens(lexer.get_tokens_ptr(), &ctx);

        std::ofstream snapshot(PROJECT_ROOT"/tests/parser/" + test.name + ".ast");
        snapshot << Utils::serialize_ast(parser.get_ast());
    }
}

void ParserTestSuite::internal_entry()
{
    for (const auto& test : suite)
    {
        Lexer lexer;
        Parser parser;
        PipelineContext ctx;

        lexer.lex_file(PROJECT_ROOT"/tests/parser/" + test.name + ".va");
        parser.parse_tokens(lexer.get_tokens_ptr(), &ctx);

        std::ifstream snapshot(PROJECT_ROOT"/tests/parser/" + test.name + ".ast");
        std::string expectedAst = std::string((std::istreambuf_iterator<char>(snapshot)), std::istreambuf_iterator<char>());
        std::string actualAst = Utils::serialize_ast(parser.get_ast());

        if (actualAst == expectedAst)
        {
            std::cout << test.name << " ... passed\n";
            passed++;
        }
        else
        {
            std::cout << test.name << " ... failed\n";
            failed++;
            std::cerr << "mismatched ast output expected:\n" << expectedAst << " got:\n" << actualAst;
        }
    }
}

bool ParserTestSuite::test_literals()
{
    return false;
}