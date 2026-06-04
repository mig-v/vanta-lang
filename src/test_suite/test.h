#pragma once

#include <string>
#include <functional>

struct Test
{
    std::string name;
    std::function<bool()> fn;
};

// general require macro for generic boolean expressions
#define REQUIRE(expr, errorMessage, expectedValue, actualValue)       \
    if (!(expr)) {                                                    \
        std::cerr << errorMessage << " expected "                     \
                  << expectedValue << " got " << actualValue << "\n"; \
        return false;                                                 \
    }

// targeted macro for comparing token kinds with a descriptive error message
#define REQUIRE_TOKEN_KIND(actual, expected)                                  \
    if ((actual) != (expected))                                               \
    {                                                                         \
        std::cerr << "mismatched token type expected"                         \
                  << Utils::token_kind_to_string(expected) << " got "         \
                  << Utils::token_kind_to_string(actual) << "\n";             \
        return false;                                                         \
    }

// targeted macro for comparing token lexemes
#define REQUIRE_TOKEN_LEXEME(actual, expected)                                                \
    if ((actual != expected))                                                                 \
    {                                                                                         \
        std::cerr << "mismatched lexemes expected " << expected << " got " << actual << "\n"; \
        return false;                                                                         \
    }                                                                                         \