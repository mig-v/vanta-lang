cmake_minimum_required(VERSION 3.10)
set(CMAKE_CXX_STANDARD 17)

project(vanta-lang)

add_executable(vanta-lang 
               src/main.cpp

               src/core/arena_block.h
               src/core/memory_arena.h
               src/core/diagnostic.h
               src/core/diagnostic.cpp
               src/core/pipeline_context.h
               src/core/interpreter.h
               src/core/interpreter.cpp
               src/core/compilation_unit.h
               src/core/compilation_unit.cpp

               src/lexer/lexer.h
               src/lexer/lexer.cpp
               src/lexer/token.h

               src/parser/ast.h
               src/parser/ast.cpp
               src/parser/parser.h
               src/parser/parser.cpp

               src/semantics/semantic_analyzer.h
               src/semantics/semantic_analyzer.cpp
               src/semantics/import_resolver.h
               src/semantics/import_resolver.cpp

               src/codegen/codegen.h
               src/codegen/codegen.cpp
               src/codegen/opcode.h
               src/codegen/value.h
               src/codegen/value.cpp
               src/codegen/environment.h
               src/codegen/environment.cpp
               src/codegen/chunk.h
               src/codegen/chunk.cpp

               src/runtime/runtime.h
               src/runtime/runtime.cpp
               src/runtime/vm.h
               src/runtime/vm.cpp
               src/runtime/built_ins.h
               src/runtime/built_ins.cpp
               src/runtime/gc.h
               src/runtime/gc.cpp

               src/utils/debug_utils.h
               src/utils/debug_utils.cpp
               src/utils/value_utils.h
               src/utils/value_utils.cpp

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