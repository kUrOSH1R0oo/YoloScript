// lexer.hpp — YoloScript lexer
#ifndef LEXER_HPP
#define LEXER_HPP

#include <string>
#include "tokens.hpp"

class YoloLexer {
public:
    explicit YoloLexer(std::string src);

    Token getNextToken();
    Token peekNextToken();
    int   currentLine() const { return line_; }

private:
    std::string source_;
    std::size_t position_;
    int         line_;

    mutable std::size_t peekPos_  = 0;
    mutable int         peekLine_ = 1;
    mutable bool        hasPeek_  = false;
    mutable Token       peeked_;

    char  peek() const;
    char  peekAhead(int offset) const;
    char  advance();
    void  skipWhitespaceAndComments();

    Token readToken();
    Token readIdentifierOrKeyword();
    Token readNumber();
    Token readString();
};

#endif