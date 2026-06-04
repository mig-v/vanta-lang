cmake_minimum_required(VERSION 3.10)
set(CMAKE_CXX_STANDARD 17)

project(vanta-lang)

add_executable(vanta-lang 
               src/main.cpp

               src/core/arena_block.h
               src/core/memory_arena.h

               src/lexer/lexer.h
               src/lexer/lexer.cpp
               src/lexer/token.h

               src/parser/ast.h
               src/parser/ast.cpp
               src/parser/parser.h
               src/parser/parser.cpp

               src/utils/debug_utils.h
               src/utils/debug_utils.cpp

               src/test_suite/test_suite.h
               src/test_suite/test_suite.cpp
               src/test_suite/lexer_test_suite.h
               src/test_suite/lexer_test_suite.cpp
               src/test_suite/test.h
               src/test_suite/test.cpp
               src/test_suite/parser_test_suite.h
               src/test_suite/parser_test_suite.cpp
)

target_compile_definitions(vanta-lang PRIVATE PROJECT_ROOT="${CMAKE_SOURCE_DIR}")

target_include_directories(vanta-lang PUBLIC src/)