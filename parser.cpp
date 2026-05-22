// parser.cpp — YoloScript parser implementation
#include "parser.hpp"
#include <sstream>

YoloParser::YoloParser(YoloLexer& lexer) : lexer_(lexer) {
    current_ = lexer_.getNextToken();
}

void YoloParser::syntaxError(const std::string& msg) const {
    throw std::runtime_error(msg + " (line " + std::to_string(current_.line) + ")");
}

void YoloParser::eat(TokenType type) {
    if (current_.type == type) {
        current_ = lexer_.getNextToken();
    } else {
        std::string got = current_.value.empty()
            ? "<token#" + std::to_string(current_.type) + ">"
            : current_.value;
        syntaxError("Expected different token but got '" + got + "'");
    }
}

bool YoloParser::tryEat(TokenType type) {
    if (current_.type == type) { current_ = lexer_.getNextToken(); return true; }
    return false;
}

void YoloParser::eatMehIfPresent() {
    while (current_.type == TOK_MEH) eat(TOK_MEH);
}

// ---------------------------------------------------------------------------
// Primary
// ---------------------------------------------------------------------------

ASTNodePtr YoloParser::parsePrimary() {
    Token tok = current_;

    if (tok.type == TOK_TRUE)  { eat(TOK_TRUE);  return std::make_unique<ASTNode>(NODE_BOOL, "1"); }
    if (tok.type == TOK_FALSE) { eat(TOK_FALSE); return std::make_unique<ASTNode>(NODE_BOOL, "0"); }

    if (tok.type == TOK_FLEX) {
        eat(TOK_FLEX);
        if (tok.value.find('.') != std::string::npos)
            return std::make_unique<ASTNode>(NODE_FLOAT, tok.value);
        return std::make_unique<ASTNode>(NODE_NUMBER, tok.value);
    }

    if (tok.type == TOK_FAM) {
        eat(TOK_FAM);
        return std::make_unique<ASTNode>(NODE_STRING, tok.value);
    }

    if (tok.type == TOK_SQUAD) {
        eat(TOK_SQUAD);
        eat(TOK_LBRACKET);
        auto arr = std::make_unique<ASTNode>(NODE_ARRAY);
        if (current_.type != TOK_RBRACKET) {
            arr->children.push_back(parseLogical());
            while (current_.type == TOK_COMMA) {
                eat(TOK_COMMA);
                arr->children.push_back(parseLogical());
            }
        }
        eat(TOK_RBRACKET);
        return arr;
    }

    if (tok.type == TOK_IDENTIFIER) {
        eat(TOK_IDENTIFIER);

        static const std::vector<std::string> builtins = {
            "len","str","num","abs","max","min","sqrt","floor","ceil","type","push","pop","join","split","upper","lower","trim","contains","replace"
        };
        bool isBuiltin = false;
        for (auto& b : builtins) if (tok.value == b) { isBuiltin = true; break; }

        if (current_.type == TOK_LPAREN) {
            eat(TOK_LPAREN);
            auto call = std::make_unique<ASTNode>(isBuiltin ? NODE_BUILTIN_CALL : NODE_FUNC_CALL, tok.value);
            if (current_.type != TOK_RPAREN) {
                call->children.push_back(parseLogical());
                while (current_.type == TOK_COMMA) {
                    eat(TOK_COMMA);
                    call->children.push_back(parseLogical());
                }
            }
            eat(TOK_RPAREN);
            return call;
        }

        return std::make_unique<ASTNode>(NODE_VARIABLE, tok.value);
    }

    if (tok.type == TOK_MOODIS) {
        eat(TOK_MOODIS);
        if (current_.type != TOK_IDENTIFIER)
            syntaxError("Expected mood name after 'moodis'");
        std::string moodName = current_.value;
        eat(TOK_IDENTIFIER);
        return std::make_unique<ASTNode>(NODE_MOODIS, moodName);
    }

    if (tok.type == TOK_LPAREN) {
        eat(TOK_LPAREN);
        auto expr = parseLogical();
        eat(TOK_RPAREN);
        return expr;
    }

    syntaxError("Expected a value but got '" + tok.value + "'");
}

ASTNodePtr YoloParser::parsePostfix() {
    auto node = parsePrimary();
    while (current_.type == TOK_LBRACKET) {
        eat(TOK_LBRACKET);
        auto idx = std::make_unique<ASTNode>(NODE_INDEX);
        idx->left  = std::move(node);
        idx->right = parseLogical();
        eat(TOK_RBRACKET);
        node = std::move(idx);
    }
    return node;
}

ASTNodePtr YoloParser::parseUnary() {
    if (current_.type == TOK_NOT) {
        eat(TOK_NOT);
        auto n = std::make_unique<ASTNode>(NODE_LOGOP, "not");
        n->left = parseUnary();
        return n;
    }
    if (current_.type == TOK_SHEESH && current_.value == "-") {
        eat(TOK_SHEESH);
        auto n   = std::make_unique<ASTNode>(NODE_BINOP, "-");
        n->left  = std::make_unique<ASTNode>(NODE_NUMBER, "0");
        n->right = parsePostfix();
        return n;
    }
    return parsePostfix();
}

ASTNodePtr YoloParser::parsePower() {
    auto node = parseUnary();
    if (current_.type == TOK_SHEESH && current_.value == "**") {
        eat(TOK_SHEESH);
        auto n   = std::make_unique<ASTNode>(NODE_BINOP, "**");
        n->left  = std::move(node);
        n->right = parsePower();
        return n;
    }
    return node;
}

ASTNodePtr YoloParser::parseMulDiv() {
    auto node = parsePower();
    while (current_.type == TOK_SHEESH &&
           (current_.value == "*" || current_.value == "/" || current_.value == "%")) {
        Token op = current_; eat(TOK_SHEESH);
        auto n = std::make_unique<ASTNode>(NODE_BINOP, op.value);
        n->left  = std::move(node);
        n->right = parsePower();
        node = std::move(n);
    }
    return node;
}

ASTNodePtr YoloParser::parseAddSub() {
    auto node = parseMulDiv();
    while (current_.type == TOK_SHEESH &&
           (current_.value == "+" || current_.value == "-")) {
        Token op = current_; eat(TOK_SHEESH);
        auto n = std::make_unique<ASTNode>(NODE_BINOP, op.value);
        n->left  = std::move(node);
        n->right = parseMulDiv();
        node = std::move(n);
    }
    return node;
}

ASTNodePtr YoloParser::parseComparison() {
    auto node = parseAddSub();
    if (current_.type == TOK_CAP) {
        Token op = current_; eat(TOK_CAP);
        auto n = std::make_unique<ASTNode>(NODE_COMPARE, op.value);
        n->left  = std::move(node);
        n->right = parseAddSub();
        return n;
    }
    return node;
}

ASTNodePtr YoloParser::parseLogical() {
    auto node = parseComparison();
    while (current_.type == TOK_AND || current_.type == TOK_OR) {
        std::string op = (current_.type == TOK_AND) ? "and" : "or";
        eat(current_.type);
        auto n = std::make_unique<ASTNode>(NODE_LOGOP, op);
        n->left  = std::move(node);
        n->right = parseComparison();
        node = std::move(n);
    }
    return node;
}

// ---------------------------------------------------------------------------
// Block
// ---------------------------------------------------------------------------

ASTNodePtr YoloParser::parseBlock() {
    auto block = std::make_unique<ASTNode>(NODE_STATEMENTS);
    if (current_.type == TOK_FR) {
        eat(TOK_FR);
        eatMehIfPresent();
        while (current_.type != TOK_EOF   &&
               current_.type != TOK_NOCAP &&
               current_.type != TOK_NAH   &&
               current_.type != TOK_LOWKEY) {
            auto stmt = parseStatement();
            if (stmt) block->children.push_back(std::move(stmt));
            eatMehIfPresent();
        }
        eat(TOK_NOCAP);
    } else {
        auto stmt = parseStatement();
        if (stmt) block->children.push_back(std::move(stmt));
    }
    return block;
}

// ---------------------------------------------------------------------------
// Statements
// ---------------------------------------------------------------------------

ASTNodePtr YoloParser::parseDeclaration() {
    eat(TOK_YOLO);
    if (current_.type != TOK_IDENTIFIER)
        syntaxError("Expected variable name after 'yolo'");
    std::string name = current_.value;
    eat(TOK_IDENTIFIER);

    if (current_.type == TOK_LBRACKET) {
        eat(TOK_LBRACKET);
        auto idx = parseLogical();
        eat(TOK_RBRACKET);
        eat(TOK_BAE);
        auto n    = std::make_unique<ASTNode>(NODE_INDEX_ASSIGN, name);
        n->left   = std::move(idx);
        n->right  = parseLogical();
        eatMehIfPresent();
        return n;
    }

    eat(TOK_BAE);
    auto n   = std::make_unique<ASTNode>(NODE_ASSIGN, name);
    n->right = parseLogical();
    eatMehIfPresent();
    return n;
}

ASTNodePtr YoloParser::parsePrint() {
    eat(TOK_SENDIT);
    auto n  = std::make_unique<ASTNode>(NODE_PRINT);
    n->left = parseLogical();
    eatMehIfPresent();
    return n;
}

ASTNodePtr YoloParser::parseInput() {
    eat(TOK_YEET);
    if (current_.type != TOK_IDENTIFIER)
        syntaxError("Expected variable name after 'yeet'");
    std::string name = current_.value;
    eat(TOK_IDENTIFIER);
    eatMehIfPresent();
    return std::make_unique<ASTNode>(NODE_INPUT, name);
}

ASTNodePtr YoloParser::parseIf() {
    eat(TOK_BRUH);
    auto cond = parseLogical();
    auto then = parseBlock();

    auto n   = std::make_unique<ASTNode>(NODE_IF);
    n->left  = std::move(cond);
    n->right = std::move(then);

    if (current_.type == TOK_LOWKEY) {
        eat(TOK_LOWKEY);
        auto elseifCond = parseLogical();
        auto elseifThen = parseBlock();
        auto elseif     = std::make_unique<ASTNode>(NODE_IF);
        elseif->left    = std::move(elseifCond);
        elseif->right   = std::move(elseifThen);
        if (current_.type == TOK_LOWKEY) {
            eat(TOK_LOWKEY);
            auto nextCond = parseLogical();
            auto nextThen = parseBlock();
            auto next     = std::make_unique<ASTNode>(NODE_IF);
            next->left    = std::move(nextCond);
            next->right   = std::move(nextThen);
            ASTNode* tip = next.get();
            while (current_.type == TOK_LOWKEY) {
                eat(TOK_LOWKEY);
                auto c2 = parseLogical();
                auto t2 = parseBlock();
                auto n2 = std::make_unique<ASTNode>(NODE_IF);
                n2->left  = std::move(c2);
                n2->right = std::move(t2);
                tip->extra = std::move(n2);
                tip = tip->extra.get();
            }
            if (current_.type == TOK_NAH) {
                eat(TOK_NAH);
                tip->extra = parseBlock();
            }
            elseif->extra = std::move(next);
        } else if (current_.type == TOK_NAH) {
            eat(TOK_NAH);
            elseif->extra = parseBlock();
        }
        n->extra = std::move(elseif);
    } else if (current_.type == TOK_NAH) {
        eat(TOK_NAH);
        n->extra = parseBlock();
    }
    return n;
}

ASTNodePtr YoloParser::parseWhile() {
    eat(TOK_GOAT);
    auto cond = parseLogical();
    auto body = parseBlock();
    auto n    = std::make_unique<ASTNode>(NODE_WHILE);
    n->left   = std::move(cond);
    n->right  = std::move(body);
    return n;
}

ASTNodePtr YoloParser::parseFor() {
    eat(TOK_LIT);
    if (current_.type != TOK_IDENTIFIER)
        syntaxError("Expected variable name after 'lit'");
    std::string var = current_.value;
    eat(TOK_IDENTIFIER);
    eat(TOK_BAE);
    auto start = parseLogical();
    eat(TOK_TO);
    auto end   = parseLogical();

    // Optional step
    ASTNodePtr step;
    if (current_.type == TOK_STEP) {
        eat(TOK_STEP);
        step = parseLogical();
    } else {
        step = std::make_unique<ASTNode>(NODE_NUMBER, "1");
    }

    auto body = parseBlock();

    auto n       = std::make_unique<ASTNode>(NODE_FOR, var);
    n->left      = std::move(start);
    n->right     = std::move(end);
    n->extra     = std::move(body);
    n->extra2    = std::move(step);
    return n;
}

// vibe funcname(p1, p2) fr ... nocap
ASTNodePtr YoloParser::parseFuncDef() {
    eat(TOK_VIBE);
    if (current_.type != TOK_IDENTIFIER)
        syntaxError("Expected function name after 'vibe'");
    std::string name = current_.value;
    eat(TOK_IDENTIFIER);
    eat(TOK_LPAREN);
    std::vector<std::string> params;
    if (current_.type != TOK_RPAREN) {
        if (current_.type != TOK_IDENTIFIER)
            syntaxError("Expected parameter name");
        params.push_back(current_.value);
        eat(TOK_IDENTIFIER);
        while (current_.type == TOK_COMMA) {
            eat(TOK_COMMA);
            if (current_.type != TOK_IDENTIFIER)
                syntaxError("Expected parameter name");
            params.push_back(current_.value);
            eat(TOK_IDENTIFIER);
        }
    }
    eat(TOK_RPAREN);
    auto body = parseBlock();
    auto n    = std::make_unique<ASTNode>(NODE_FUNC_DEF, name);
    n->params = std::move(params);
    n->right  = std::move(body);
    eatMehIfPresent();
    return n;
}

ASTNodePtr YoloParser::parseReturn() {
    eat(TOK_SLAY);
    auto n  = std::make_unique<ASTNode>(NODE_RETURN);
    if (current_.type != TOK_MEH && current_.type != TOK_EOF &&
        current_.type != TOK_NOCAP)
        n->left = parseLogical();
    eatMehIfPresent();
    return n;
}

ASTNodePtr YoloParser::parseBreak() {
    eat(TOK_BOUNCE);
    eatMehIfPresent();
    return std::make_unique<ASTNode>(NODE_BREAK);
}

ASTNodePtr YoloParser::parseContinue() {
    eat(TOK_SKIP);
    eatMehIfPresent();
    return std::make_unique<ASTNode>(NODE_CONTINUE);
}

ASTNodePtr YoloParser::parseExit() {
    eat(TOK_BYE);
    auto n = std::make_unique<ASTNode>(NODE_EXIT);
    if (current_.type != TOK_MEH && current_.type != TOK_EOF)
        n->left = parseLogical();
    eatMehIfPresent();
    return n;
}

ASTNodePtr YoloParser::parseStatement() {
    eatMehIfPresent();
    switch (current_.type) {
        case TOK_YOLO:   return parseDeclaration();
        case TOK_SENDIT: return parsePrint();
        case TOK_YEET:   return parseInput();
        case TOK_BRUH:   return parseIf();
        case TOK_GOAT:   return parseWhile();
        case TOK_LIT:    return parseFor();
        case TOK_VIBE:   return parseFuncDef();
        case TOK_SLAY:   return parseReturn();
        case TOK_BOUNCE: return parseBreak();
        case TOK_SKIP:   return parseContinue();
        case TOK_BYE:        return parseExit();
        case TOK_MOOD:       return parseMood();
        case TOK_MOODCHECK:  return parseMoodCheck();
        case TOK_VIBE_CHECK: return parseVibeCheck();
        case TOK_EOF:        return nullptr;
        default:
            if (current_.type == TOK_IDENTIFIER) {
                Token saved = current_;
                Token next = lexer_.peekNextToken();
                if (next.type == TOK_LPAREN) {
                    auto expr = parseLogical();
                    eatMehIfPresent();
                    auto n = std::make_unique<ASTNode>(NODE_PRINT);
                    n->value = "__void__";
                    n->left  = std::move(expr);
                    return n;
                }
            }
            syntaxError("Unexpected token '" + current_.value + "'");
    }
}


// ── Mood system ──────────────────────────────────────────────────────────────

ASTNodePtr YoloParser::parseMood() {
    eat(TOK_MOOD);
    if (current_.type != TOK_IDENTIFIER)
        syntaxError("Expected mood name after 'mood' (e.g. mood hyped)");
    std::string moodName = current_.value;
    eat(TOK_IDENTIFIER);
    eatMehIfPresent();
    return std::make_unique<ASTNode>(NODE_SET_MOOD, moodName);
}

ASTNodePtr YoloParser::parseMoodCheck() {
    eat(TOK_MOODCHECK);
    eatMehIfPresent();
    return std::make_unique<ASTNode>(NODE_MOODCHECK);
}

ASTNodePtr YoloParser::parseMoodIs() {
    eat(TOK_MOODIS);
    if (current_.type != TOK_IDENTIFIER)
        syntaxError("Expected mood name after 'moodis'");
    std::string moodName = current_.value;
    eat(TOK_IDENTIFIER);
    return std::make_unique<ASTNode>(NODE_MOODIS, moodName);
}

ASTNodePtr YoloParser::parseVibeCheck() {
    eat(TOK_VIBE_CHECK);
    eatMehIfPresent();
    return std::make_unique<ASTNode>(NODE_VIBE_CHECK);
}

// ─────────────────────────────────────────────────────────────────────────────

ASTNodePtr YoloParser::parse() {
    auto program = std::make_unique<ASTNode>(NODE_STATEMENTS);
    while (current_.type != TOK_EOF) {
        eatMehIfPresent();
        if (current_.type == TOK_EOF) break;
        auto stmt = parseStatement();
        if (stmt) program->children.push_back(std::move(stmt));
    }
    return program;
}
