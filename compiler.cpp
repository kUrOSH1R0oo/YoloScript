// compiler.cpp — YoloScript compiler implementation
#include "compiler.hpp"
#include "lexer.hpp"
#include "parser.hpp"
#include "interpreter.hpp"
#include <iostream>

YoloCompiler::YoloCompiler(std::string source) : source_(std::move(source)) {}

bool YoloCompiler::compileAndRun() {
    try {
        YoloLexer  lexer(source_);
        YoloParser parser(lexer);
        auto ast = parser.parse();     
        YoloInterpreter interp;
        return interp.interpret(ast.get()); 
    } catch (const std::exception& e) {
        std::cerr << "YoloScript Syntax Error: " << e.what() << std::endl;
        return false;
    }
}
