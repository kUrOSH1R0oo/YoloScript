// ast.hpp — YoloScript AST node definitions
#ifndef AST_HPP
#define AST_HPP

#include <memory>
#include <string>
#include <vector>

enum NodeType {
    NODE_STATEMENTS,
    NODE_PRINT,
    NODE_ASSIGN,
    NODE_INDEX_ASSIGN,
    NODE_VARIABLE,
    NODE_NUMBER,
    NODE_FLOAT,
    NODE_STRING,
    NODE_BOOL,
    NODE_BINOP,
    NODE_LOGOP,
    NODE_IF,
    NODE_WHILE,
    NODE_FOR,       
    NODE_COMPARE,
    NODE_INPUT,
    NODE_BREAK,
    NODE_CONTINUE,
    NODE_EXIT,
    NODE_FUNC_DEF, 
    NODE_FUNC_CALL,   
    NODE_RETURN,      
    NODE_ARRAY,      
    NODE_INDEX,     
    NODE_BUILTIN_CALL,
    // ── Mood system ──────────────────────────────
    NODE_SET_MOOD,    
    NODE_MOODCHECK,   
    NODE_MOODIS,      
    NODE_VIBE_CHECK,
    // ─────────────────────────────────────────────
};

using ASTNodePtr = std::unique_ptr<struct ASTNode>;

struct ASTNode {
    NodeType                type;
    std::string             value;
    ASTNodePtr              left;
    ASTNodePtr              right;
    ASTNodePtr              extra;  
    ASTNodePtr              extra2;
    std::vector<ASTNodePtr> children;
    std::vector<std::string> params;

    explicit ASTNode(NodeType t, std::string v = "")
        : type(t), value(std::move(v)) {}

    ASTNode(const ASTNode&)            = delete;
    ASTNode& operator=(const ASTNode&) = delete;
    ASTNode(ASTNode&&)                 = default;
    ASTNode& operator=(ASTNode&&)      = default;
};

#endif
