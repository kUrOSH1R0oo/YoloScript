// lexer.cpp — YoloScript lexer implementation
#include "lexer.hpp"
#include <cctype>
#include <stdexcept>

YoloLexer::YoloLexer(std::string src)
    : source_(std::move(src)), position_(0), line_(1) {}

char YoloLexer::peek() const {
    return (position_ < source_.size()) ? source_[position_] : '\0';
}

char YoloLexer::peekAhead(int offset) const {
    std::size_t pos = position_ + offset;
    return (pos < source_.size()) ? source_[pos] : '\0';
}

char YoloLexer::advance() {
    if (position_ >= source_.size()) return '\0';
    char c = source_[position_++];
    if (c == '\n') ++line_;
    return c;
}

void YoloLexer::skipWhitespaceAndComments() {
    while (true) {
        while (position_ < source_.size() && std::isspace((unsigned char)peek()))
            advance();
        if (peek() == '#') {
            while (peek() != '\n' && peek() != '\0')
                advance();
        } else {
            break;
        }
    }
}

Token YoloLexer::readIdentifierOrKeyword() {
    std::string ident;
    while (std::isalnum((unsigned char)peek()) || peek() == '_')
        ident += advance();

    if (ident == "yolo")   return Token(TOK_YOLO,    ident, line_);
    if (ident == "sendit") return Token(TOK_SENDIT,  ident, line_);
    if (ident == "bruh")   return Token(TOK_BRUH,    ident, line_);
    if (ident == "nah")    return Token(TOK_NAH,     ident, line_);
    if (ident == "lowkey") return Token(TOK_LOWKEY,  ident, line_);
    if (ident == "goat")   return Token(TOK_GOAT,    ident, line_);
    if (ident == "lit")    return Token(TOK_LIT,     ident, line_);
    if (ident == "to")     return Token(TOK_TO,      ident, line_);
    if (ident == "step")   return Token(TOK_STEP,    ident, line_);
    if (ident == "meh")    return Token(TOK_MEH,     ident, line_);
    if (ident == "yeet")   return Token(TOK_YEET,    ident, line_);
    if (ident == "fr")     return Token(TOK_FR,      ident, line_);
    if (ident == "nocap")  return Token(TOK_NOCAP,   ident, line_);
    if (ident == "and")    return Token(TOK_AND,     ident, line_);
    if (ident == "or")     return Token(TOK_OR,      ident, line_);
    if (ident == "not")    return Token(TOK_NOT,     ident, line_);
    if (ident == "bounce") return Token(TOK_BOUNCE,  ident, line_);
    if (ident == "skip")   return Token(TOK_SKIP,    ident, line_);
    if (ident == "bye")    return Token(TOK_BYE,     ident, line_);
    if (ident == "vibe")   return Token(TOK_VIBE,    ident, line_);
    if (ident == "slay")   return Token(TOK_SLAY,    ident, line_);
    if (ident == "squad")  return Token(TOK_SQUAD,   ident, line_);
    if (ident == "true")   return Token(TOK_TRUE,    ident, line_);
    if (ident == "false")  return Token(TOK_FALSE,      ident, line_);
    if (ident == "mood")       return Token(TOK_MOOD,       ident, line_);
    if (ident == "moodcheck")  return Token(TOK_MOODCHECK,  ident, line_);
    if (ident == "moodis")     return Token(TOK_MOODIS,     ident, line_);
    if (ident == "vibecheck")  return Token(TOK_VIBE_CHECK, ident, line_);

    return Token(TOK_IDENTIFIER, ident, line_);
}

Token YoloLexer::readNumber() {
    std::string number;
    while (std::isdigit((unsigned char)peek()))
        number += advance();
    if (peek() == '.' && std::isdigit((unsigned char)peekAhead(1))) {
        number += advance();
        while (std::isdigit((unsigned char)peek()))
            number += advance();
        return Token(TOK_FLEX, number, line_);
    }
    return Token(TOK_FLEX, number, line_);
}

Token YoloLexer::readString() {
    advance();
    std::string str;
    while (peek() != '"' && peek() != '\0') {
        if (peek() == '\n')
            throw std::runtime_error("Unterminated string literal at line " + std::to_string(line_));
        if (peek() == '\\') {
            advance();
            switch (peek()) {
                case 'n':  str += '\n'; advance(); break;
                case 't':  str += '\t'; advance(); break;
                case '"':  str += '"';  advance(); break;
                case '\\': str += '\\'; advance(); break;
                default:   str += '\\'; str += advance(); break;
            }
        } else {
            str += advance();
        }
    }
    if (peek() == '"') advance();
    else throw std::runtime_error("Unterminated string literal at line " + std::to_string(line_));
    return Token(TOK_FAM, str, line_);
}

Token YoloLexer::readToken() {
    skipWhitespaceAndComments();
    if (position_ >= source_.size()) return Token(TOK_EOF, "", line_);

    char c = peek();

    if (std::isalpha((unsigned char)c) || c == '_') return readIdentifierOrKeyword();
    if (std::isdigit((unsigned char)c))             return readNumber();
    if (c == '"')                                    return readString();

    if (c == '=') { advance(); if (peek()=='='){advance();return Token(TOK_CAP,"==",line_);} return Token(TOK_BAE,"=",line_); }
    if (c == '!') { advance(); if (peek()=='='){advance();return Token(TOK_CAP,"!=",line_);}
        throw std::runtime_error(std::string("Unexpected '!' at line ")+std::to_string(line_)+". Use 'not' or '!='"); }
    if (c == '<') { advance(); if (peek()=='='){advance();return Token(TOK_CAP,"<=",line_);} return Token(TOK_CAP,"<",line_); }
    if (c == '>') { advance(); if (peek()=='='){advance();return Token(TOK_CAP,">=",line_);} return Token(TOK_CAP,">",line_); }
    if (c == '*') {
        advance();
        if (peek() == '*') { advance(); return Token(TOK_SHEESH, "**", line_); }
        return Token(TOK_SHEESH, "*", line_);
    }
    if (c == '+') { advance(); return Token(TOK_SHEESH, "+", line_); }
    if (c == '-') { advance(); return Token(TOK_SHEESH, "-", line_); }
    if (c == '/') { advance(); return Token(TOK_SHEESH, "/", line_); }
    if (c == '%') { advance(); return Token(TOK_SHEESH, "%", line_); }
    if (c == '[') { advance(); return Token(TOK_LBRACKET, "[", line_); }
    if (c == ']') { advance(); return Token(TOK_RBRACKET, "]", line_); }
    if (c == ',') { advance(); return Token(TOK_COMMA,    ",", line_); }
    if (c == '(') { advance(); return Token(TOK_LPAREN,   "(", line_); }
    if (c == ')') { advance(); return Token(TOK_RPAREN,   ")", line_); }

    char bad = advance();
    throw std::runtime_error(std::string("Unknown character '") + bad +
        "' (ASCII " + std::to_string((unsigned char)bad) + ") at line " + std::to_string(line_));
}

Token YoloLexer::getNextToken() {
    if (hasPeek_) {
        hasPeek_  = false;
        position_ = peekPos_;
        line_     = peekLine_;
        return peeked_;
    }
    return readToken();
}

Token YoloLexer::peekNextToken() {
    if (!hasPeek_) {
        std::size_t savedPos  = position_;
        int         savedLine = line_;
        peeked_    = readToken();
        peekPos_   = position_;
        peekLine_  = line_;
        position_  = savedPos;
        line_      = savedLine;
        hasPeek_   = true;
    }
    return peeked_;
}
