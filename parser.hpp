// parser.hpp — YoloScript parser
#ifndef PARSER_HPP
#define PARSER_HPP

#include "lexer.hpp"
#include "ast.hpp"
#include <stdexcept>
#include <vector>

class YoloParser {
public:
    explicit YoloParser(YoloLexer& lexer);
    ASTNodePtr parse();

private:
    YoloLexer& lexer_;
    Token      current_;

    void eat(TokenType type);
    bool tryEat(TokenType type);
    void eatMehIfPresent();
    [[noreturn]] void syntaxError(const std::string& msg) const;

    ASTNodePtr parseLogical();
    ASTNodePtr parseComparison();
    ASTNodePtr parseAddSub();
    ASTNodePtr parseMulDiv();
    ASTNodePtr parsePower();
    ASTNodePtr parseUnary();
    ASTNodePtr parsePostfix();
    ASTNodePtr parsePrimary();

    ASTNodePtr parseStatement();
    ASTNodePtr parseBlock();

    ASTNodePtr parseDeclaration();
    ASTNodePtr parsePrint();
    ASTNodePtr parseInput();
    ASTNodePtr parseIf();
    ASTNodePtr parseWhile();
    ASTNodePtr parseFor();
    ASTNodePtr parseFuncDef();
    ASTNodePtr parseReturn();
    ASTNodePtr parseBreak();
    ASTNodePtr parseContinue();
    ASTNodePtr parseExit();
    ASTNodePtr parseMood();
    ASTNodePtr parseMoodCheck();
    ASTNodePtr parseMoodIs();
    ASTNodePtr parseVibeCheck();
};

#endif
