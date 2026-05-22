// interpreter.hpp — YoloScript interpreter
#ifndef INTERPRETER_HPP
#define INTERPRETER_HPP

#include "ast.hpp"
#include <map>
#include <memory>
#include <string>
#include <vector>
#include <functional>
#include <stdexcept>
#include <iostream>

// -------------------------------------------------------------------------
// Simple control-flow signals (declared before YoloValue)
// -------------------------------------------------------------------------
struct BreakSignal    {};
struct ContinueSignal {};
struct ExitSignal     { int code; };

// -------------------------------------------------------------------------
// Runtime value: int, float, string, array, or null
// -------------------------------------------------------------------------
struct YoloValue {
    enum Kind { INT, FLOAT, STR, ARRAY, NIL } kind = NIL;

    long long   ival = 0;
    double      fval = 0.0;
    std::string sval;
    std::shared_ptr<std::vector<YoloValue>> aval;

    YoloValue()                       : kind(NIL)   {}
    explicit YoloValue(long long v)   : kind(INT),   ival(v)          {}
    explicit YoloValue(double v)      : kind(FLOAT), fval(v)          {}
    explicit YoloValue(std::string s) : kind(STR),   sval(std::move(s)) {}

    static YoloValue makeArray() {
        YoloValue v; v.kind = ARRAY;
        v.aval = std::make_shared<std::vector<YoloValue>>();
        return v;
    }

    bool isInt()   const { return kind == INT;   }
    bool isFloat() const { return kind == FLOAT; }
    bool isNum()   const { return kind == INT || kind == FLOAT; }
    bool isStr()   const { return kind == STR;   }
    bool isArr()   const { return kind == ARRAY; }
    bool isNil()   const { return kind == NIL;   }

    double toDouble() const { return isFloat() ? fval : (double)ival; }
    bool   truthy()   const;
    std::string toString() const;
    std::string typeName() const;
};

// -------------------------------------------------------------------------
// Return signal (needs complete YoloValue)
// -------------------------------------------------------------------------
struct ReturnSignal { YoloValue val; };

// -------------------------------------------------------------------------
// Function definition (stored in environment)
// -------------------------------------------------------------------------
struct YoloFunc {
    std::vector<std::string> params;
    const ASTNode*           body   = nullptr; // non-owning, lives in AST
};

// -------------------------------------------------------------------------
// Scope (for local variables and function calls)
// -------------------------------------------------------------------------
struct Scope {
    std::map<std::string, YoloValue> vars;
    Scope* parent = nullptr;

    YoloValue* lookup(const std::string& name) {
        auto it = vars.find(name);
        if (it != vars.end()) return &it->second;
        return parent ? parent->lookup(name) : nullptr;
    }
    void set(const std::string& name, YoloValue val) {
        // If already defined somewhere up the chain, update there
        if (auto* p = lookup(name)) { *p = std::move(val); return; }
        vars[name] = std::move(val);
    }
    void setLocal(const std::string& name, YoloValue val) {
        vars[name] = std::move(val);
    }
};

// -------------------------------------------------------------------------
// Interpreter
// -------------------------------------------------------------------------

// ── Mood Engine ──────────────────────────────────────────────────────────────
// The mood affects how the interpreter runs.
enum class Mood {
    NORMAL,      // default — runs as expected
    HYPED,       // arithmetic results are amplified (×2)
    CHILL,       // integer division on everything, results floored
    SUS,         // sendit randomly lies (20% chance output is ??? )
    LOWBATTERY,  // all numbers shrink toward zero (×0.5, truncated)
    CHAOS        // + and - are randomly swapped at runtime
};

struct MoodEngine {
    Mood        current   = Mood::NORMAL;
    int         stmtCount = 0;
    int         decayAt   = 50;
    bool        decayEnabled = true;

    static std::string name(Mood m) {
        switch (m) {
            case Mood::NORMAL:     return "normal";
            case Mood::HYPED:      return "hyped";
            case Mood::CHILL:      return "chill";
            case Mood::SUS:        return "sus";
            case Mood::LOWBATTERY: return "lowbattery";
            case Mood::CHAOS:      return "chaos";
        }
        return "normal";
    }

    static Mood fromString(const std::string& s) {
        if (s == "hyped")      return Mood::HYPED;
        if (s == "chill")      return Mood::CHILL;
        if (s == "sus")        return Mood::SUS;
        if (s == "lowbattery") return Mood::LOWBATTERY;
        if (s == "chaos")      return Mood::CHAOS;
        if (s == "normal")     return Mood::NORMAL;
        throw std::runtime_error("Unknown mood '" + s +
            "'. Valid moods: normal, hyped, chill, sus, lowbattery, chaos");
    }

    static Mood randomMood() {
        static const Mood moods[] = {
            Mood::NORMAL, Mood::HYPED, Mood::CHILL,
            Mood::SUS, Mood::LOWBATTERY, Mood::CHAOS
        };
        return moods[std::rand() % 6];
    }

    void tick() {
        if (!decayEnabled) return;
        if (++stmtCount >= decayAt && current != Mood::NORMAL) {
            current   = Mood::NORMAL;
            stmtCount = 0;
            std::cerr << "[YoloScript] Mood decayed back to normal.\n";
        }
    }

    void set(Mood m) {
        current   = m;
        stmtCount = 0;
        std::cerr << "[YoloScript] Mood set to: " << name(m) << "\n";
    }
};
// ─────────────────────────────────────────────────────────────────────────────

class YoloInterpreter {
public:
    bool interpret(const ASTNode* tree);

private:
    std::map<std::string, YoloFunc> functions_;
    Scope      globalScope_;
    MoodEngine mood_;
    bool       moodSeeded_ = false;

    YoloValue eval(const ASTNode* node, Scope& scope);
    void      execute(const ASTNode* node, Scope& scope);
    void      executeStatements(const ASTNode* node, Scope& scope);

    YoloValue callFunction(const std::string& name,
                           std::vector<YoloValue> args,
                           Scope& callerScope);
    YoloValue callBuiltin(const std::string& name,
                          std::vector<YoloValue> args);

    YoloValue getInput();
    YoloValue applyMoodToArith(YoloValue v, const std::string& op);
    bool      moodSuppressOutput();
};

#endif
