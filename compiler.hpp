// compiler.hpp — YoloScript compiler (lex → parse → interpret)
#ifndef COMPILER_HPP
#define COMPILER_HPP

#include <string>

class YoloCompiler {
public:
    explicit YoloCompiler(std::string source);

    bool compileAndRun();

private:
    std::string source_;
};

#endif
