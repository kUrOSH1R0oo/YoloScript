// tokens.hpp — YoloScript token definitions
#ifndef TOKENS_HPP
#define TOKENS_HPP

#include <string>

enum TokenType {
    TOK_YOLO, TOK_SENDIT, TOK_BRUH, TOK_NAH, TOK_LOWKEY,
    TOK_FAM, TOK_FLEX, TOK_BAE, TOK_SHEESH, TOK_CAP,
    TOK_GOAT, TOK_LIT, TOK_TO, TOK_STEP, TOK_MEH,
    TOK_IDENTIFIER, TOK_YEET, TOK_FR, TOK_NOCAP,
    TOK_AND, TOK_OR, TOK_NOT,
    TOK_BOUNCE, TOK_SKIP, TOK_BYE,
    TOK_VIBE, TOK_SLAY,
    TOK_SQUAD, TOK_LBRACKET, TOK_RBRACKET, TOK_COMMA,
    TOK_LPAREN, TOK_RPAREN,
    TOK_TRUE, TOK_FALSE,
    TOK_MOOD,
    TOK_MOODCHECK,
    TOK_MOODIS,
    TOK_VIBE_CHECK,
    TOK_EOF
};

struct Token {
    TokenType   type;
    std::string value;
    int         line;

    Token() : type(TOK_EOF), value(""), line(1) {}
    Token(TokenType t, std::string v, int l) : type(t), value(std::move(v)), line(l) {}
};

#endif
