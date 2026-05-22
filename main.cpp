// main.cpp — YoloScript entry point
#include "compiler.hpp"
#include "utils.hpp"
#include <iostream>
#include <string>

static const char* VERSION = "1.0";

static void printHelp(const char* argv0) {
    std::cout
        << "YoloScript v" << VERSION << "\n"
        << "Usage:\n"
        << "  " << argv0 << " <file.yolo>   Run a YoloScript source file\n"
        << "  " << argv0 << "               Read source from stdin\n"
        << "  " << argv0 << " --help        Show this help\n"
        << "\nLanguage reference:\n"
        << "  yolo x = <expr> meh          Declare / assign variable\n"
        << "  sendit <expr> meh            Print value (no auto-newline)\n"
        << "  yeet x meh                   Read input into variable x\n"
        << "  bruh <cond> fr ... nocap     If block\n"
        << "  bruh <cond> fr ... nocap\n"
        << "    nah fr ... nocap           If / else\n"
        << "  goat <cond> fr ... nocap     While loop\n"
        << "  # comment                    Line comment\n"
        << "  Operators: + - * / %  ==  !=  <  >  <=  >=  and  or  not\n"
        << "  String concatenation: sendit \"hello \" + name meh\n";
}

int main(int argc, char* argv[]) {
    if (argc > 1 && (std::string(argv[1]) == "--help" || std::string(argv[1]) == "-h")) {
        printHelp(argv[0]);
        return 0;
    }

    std::string source;
    try {
        if (argc > 1) {
            source = readFromFile(argv[1]);
        } else {
            std::cerr << "YoloScript v" << VERSION
                      << " — no file given, running built-in demo.\n"
                      << "Run '" << argv[0] << " --help' for usage.\n\n";

            source = R"(
# YoloScript v1.0 - Kur0Sh1r0
yolo x = 10 meh
yolo greeting = "Hello, YoloScript!" meh

sendit greeting meh
sendit "\n" meh

bruh x == 10 fr
    sendit "x is 10 — correct!\n" meh
nocap nah fr
    sendit "x is NOT 10 — something is wrong!\n" meh
nocap

yolo i = 1 meh
goat i <= 5 fr
    sendit "  count: " meh
    sendit i meh
    sendit "\n" meh
    yolo i = i + 1 meh
nocap

sendit "Done!\n" meh
)";
        }
    } catch (const std::exception& e) {
        std::cerr << "Error: " << e.what() << std::endl;
        return 1;
    }

    YoloCompiler compiler(std::move(source));
    return compiler.compileAndRun() ? 0 : 1;
}
