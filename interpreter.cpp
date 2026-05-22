// interpreter.cpp — YoloScript interpreter implementation
#include "interpreter.hpp"
#include <iostream>
#include <cstdlib>
#include <ctime>
#include <sstream>
#include <cmath>
#include <algorithm>
#include <stdexcept>

// -------------------------------------------------------------------------
// YoloValue helpers
// -------------------------------------------------------------------------

bool YoloValue::truthy() const {
    switch (kind) {
        case INT:   return ival != 0;
        case FLOAT: return fval != 0.0;
        case STR:   return !sval.empty();
        case ARRAY: return aval && !aval->empty();
        case NIL:   return false;
    }
    return false;
}

std::string YoloValue::toString() const {
    switch (kind) {
        case INT:   return std::to_string(ival);
        case FLOAT: {
            std::ostringstream oss;
            oss << fval;
            return oss.str();
        }
        case STR:   return sval;
        case ARRAY: {
            std::string s = "[";
            if (aval) {
                for (std::size_t i = 0; i < aval->size(); ++i) {
                    if (i) s += ", ";
                    const auto& v = (*aval)[i];
                    s += (v.isStr() ? "\"" + v.sval + "\"" : v.toString());
                }
            }
            return s + "]";
        }
        case NIL:   return "nil";
    }
    return "";
}

std::string YoloValue::typeName() const {
    switch (kind) {
        case INT:   return "int";
        case FLOAT: return "float";
        case STR:   return "string";
        case ARRAY: return "array";
        case NIL:   return "nil";
    }
    return "unknown";
}

static YoloValue makeInt(long long v)      { return YoloValue(v); }
static YoloValue makeFloat(double v)       { return YoloValue(v); }
static YoloValue makeStr(std::string s)    { return YoloValue(std::move(s)); }
static YoloValue makeNil()                 { return YoloValue(); }

// -------------------------------------------------------------------------
// Builtin functions
// -------------------------------------------------------------------------

YoloValue YoloInterpreter::callBuiltin(const std::string& name,
                                        std::vector<YoloValue> args) {
    auto requireArgs = [&](int n) {
        if ((int)args.size() != n)
            throw std::runtime_error("'" + name + "' expects " +
                std::to_string(n) + " argument(s), got " +
                std::to_string(args.size()));
    };

    if (name == "len") {
        requireArgs(1);
        if (args[0].isStr())  return makeInt((long long)args[0].sval.size());
        if (args[0].isArr())  return makeInt((long long)args[0].aval->size());
        throw std::runtime_error("len() requires a string or array");
    }
    if (name == "str") {
        requireArgs(1);
        return makeStr(args[0].toString());
    }
    if (name == "num") {
        requireArgs(1);
        if (args[0].isNum()) return args[0];
        if (args[0].isStr()) {
            try {
                std::size_t p = 0;
                if (args[0].sval.find('.') != std::string::npos)
                    return makeFloat(std::stod(args[0].sval, &p));
                long long v = std::stoll(args[0].sval, &p);
                return makeInt(v);
            } catch (...) {}
            throw std::runtime_error("Cannot convert \"" + args[0].sval + "\" to a number");
        }
        throw std::runtime_error("num() requires a string or number");
    }
    if (name == "abs") {
        requireArgs(1);
        if (args[0].isInt())   return makeInt(std::abs(args[0].ival));
        if (args[0].isFloat()) return makeFloat(std::abs(args[0].fval));
        throw std::runtime_error("abs() requires a number");
    }
    if (name == "sqrt") {
        requireArgs(1);
        if (!args[0].isNum()) throw std::runtime_error("sqrt() requires a number");
        return makeFloat(std::sqrt(args[0].toDouble()));
    }
    if (name == "floor") {
        requireArgs(1);
        if (!args[0].isNum()) throw std::runtime_error("floor() requires a number");
        return makeInt((long long)std::floor(args[0].toDouble()));
    }
    if (name == "ceil") {
        requireArgs(1);
        if (!args[0].isNum()) throw std::runtime_error("ceil() requires a number");
        return makeInt((long long)std::ceil(args[0].toDouble()));
    }
    if (name == "max") {
        if (args.size() == 1 && args[0].isArr()) {
            if (args[0].aval->empty()) throw std::runtime_error("max() on empty array");
            auto& v = *args[0].aval;
            return *std::max_element(v.begin(), v.end(), [](const YoloValue& a, const YoloValue& b){
                return a.toDouble() < b.toDouble();
            });
        }
        if (args.size() < 2) throw std::runtime_error("max() requires at least 2 arguments");
        YoloValue best = args[0];
        for (std::size_t i = 1; i < args.size(); ++i)
            if (args[i].toDouble() > best.toDouble()) best = args[i];
        return best;
    }
    if (name == "min") {
        if (args.size() == 1 && args[0].isArr()) {
            if (args[0].aval->empty()) throw std::runtime_error("min() on empty array");
            auto& v = *args[0].aval;
            return *std::min_element(v.begin(), v.end(), [](const YoloValue& a, const YoloValue& b){
                return a.toDouble() < b.toDouble();
            });
        }
        if (args.size() < 2) throw std::runtime_error("min() requires at least 2 arguments");
        YoloValue best = args[0];
        for (std::size_t i = 1; i < args.size(); ++i)
            if (args[i].toDouble() < best.toDouble()) best = args[i];
        return best;
    }
    if (name == "type") {
        requireArgs(1);
        return makeStr(args[0].typeName());
    }
    if (name == "push") {
        if (args.size() < 2) throw std::runtime_error("push() requires array and value");
        if (!args[0].isArr()) throw std::runtime_error("push() first argument must be an array");
        args[0].aval->push_back(args[1]);
        return args[0];
    }
    if (name == "pop") {
        requireArgs(1);
        if (!args[0].isArr()) throw std::runtime_error("pop() requires an array");
        if (args[0].aval->empty()) throw std::runtime_error("pop() on empty array");
        YoloValue last = args[0].aval->back();
        args[0].aval->pop_back();
        return last;
    }
    if (name == "join") {
        if (args.size() < 1 || !args[0].isArr()) throw std::runtime_error("join() requires array");
        std::string sep = (args.size() >= 2) ? args[1].toString() : "";
        std::string result;
        for (std::size_t i = 0; i < args[0].aval->size(); ++i) {
            if (i) result += sep;
            result += (*args[0].aval)[i].toString();
        }
        return makeStr(result);
    }
    if (name == "split") {
        if (args.size() < 2) throw std::runtime_error("split() requires string and delimiter");
        std::string s   = args[0].toString();
        std::string del = args[1].toString();
        YoloValue arr = YoloValue::makeArray();
        if (del.empty()) {
            for (char c : s) arr.aval->push_back(makeStr(std::string(1, c)));
        } else {
            std::size_t pos = 0, found;
            while ((found = s.find(del, pos)) != std::string::npos) {
                arr.aval->push_back(makeStr(s.substr(pos, found - pos)));
                pos = found + del.size();
            }
            arr.aval->push_back(makeStr(s.substr(pos)));
        }
        return arr;
    }
    if (name == "upper") {
        requireArgs(1);
        std::string s = args[0].toString();
        std::transform(s.begin(), s.end(), s.begin(), ::toupper);
        return makeStr(s);
    }
    if (name == "lower") {
        requireArgs(1);
        std::string s = args[0].toString();
        std::transform(s.begin(), s.end(), s.begin(), ::tolower);
        return makeStr(s);
    }
    if (name == "trim") {
        requireArgs(1);
        std::string s = args[0].toString();
        auto l = s.find_first_not_of(" \t\r\n");
        auto r = s.find_last_not_of(" \t\r\n");
        return makeStr((l == std::string::npos) ? "" : s.substr(l, r - l + 1));
    }
    if (name == "contains") {
        if (args.size() < 2) throw std::runtime_error("contains() requires 2 arguments");
        if (args[0].isStr())
            return makeInt(args[0].sval.find(args[1].toString()) != std::string::npos ? 1 : 0);
        if (args[0].isArr()) {
            for (auto& v : *args[0].aval)
                if (v.toString() == args[1].toString()) return makeInt(1);
            return makeInt(0);
        }
        throw std::runtime_error("contains() requires string or array");
    }
    if (name == "replace") {
        if (args.size() < 3) throw std::runtime_error("replace() requires 3 arguments");
        std::string s   = args[0].toString();
        std::string from = args[1].toString();
        std::string to   = args[2].toString();
        std::size_t pos = 0;
        while ((pos = s.find(from, pos)) != std::string::npos) {
            s.replace(pos, from.size(), to);
            pos += to.size();
        }
        return makeStr(s);
    }

    throw std::runtime_error("Unknown builtin function '" + name + "'");
}

// -------------------------------------------------------------------------
// User-defined function call
// -------------------------------------------------------------------------

YoloValue YoloInterpreter::callFunction(const std::string& name,
                                         std::vector<YoloValue> args,
                                         Scope& /*callerScope*/) {
    auto it = functions_.find(name);
    if (it == functions_.end())
        throw std::runtime_error("Undefined function '" + name + "'");

    const YoloFunc& fn = it->second;
    if (args.size() != fn.params.size())
        throw std::runtime_error("Function '" + name + "' expects " +
            std::to_string(fn.params.size()) + " argument(s), got " +
            std::to_string(args.size()));

    // Create a new scope for the function, with global as parent
    Scope funcScope;
    funcScope.parent = &globalScope_;
    for (std::size_t i = 0; i < fn.params.size(); ++i)
        funcScope.setLocal(fn.params[i], args[i]);

    try {
        executeStatements(fn.body, funcScope);
    } catch (ReturnSignal& ret) {
        return ret.val;
    }
    return makeNil();
}


// ── Mood engine helpers ───────────────────────────────────────────────────────

YoloValue YoloInterpreter::applyMoodToArith(YoloValue v, const std::string& op) {
    switch (mood_.current) {
        case Mood::HYPED:
            if (v.isInt())   return makeInt(v.ival * 2);
            if (v.isFloat()) return makeFloat(v.fval * 2.0);
            break;
        case Mood::LOWBATTERY:
            if (v.isInt())   return makeInt(v.ival / 2);
            if (v.isFloat()) return makeInt((long long)(v.fval / 2.0));
            break;
        case Mood::CHILL:
            if (v.isFloat()) return makeInt((long long)v.fval);
            break;
        case Mood::CHAOS: {
            if (op == "+" || op == "-") {
                if (std::rand() % 3 == 0) {
                    if (v.isInt())   return makeInt(-v.ival);
                    if (v.isFloat()) return makeFloat(-v.fval);
                }
            }
            break;
        }
        default: break;
    }
    return v;
}

bool YoloInterpreter::moodSuppressOutput() {
    if (mood_.current == Mood::SUS) {
        if (std::rand() % 5 == 0) {
            std::cout << "???" << std::flush;
            return true;
        }
    }
    return false;
}

// ─────────────────────────────────────────────────────────────────────────────

// -------------------------------------------------------------------------
// eval
// -------------------------------------------------------------------------

YoloValue YoloInterpreter::eval(const ASTNode* node, Scope& scope) {
    if (!node) throw std::runtime_error("Internal: null node in eval");

    switch (node->type) {

        case NODE_NUMBER: return makeInt(std::stoll(node->value));
        case NODE_FLOAT:  return makeFloat(std::stod(node->value));
        case NODE_STRING: return makeStr(node->value);
        case NODE_BOOL:   return makeInt(node->value == "1" ? 1 : 0);

        case NODE_VARIABLE: {
            auto* v = scope.lookup(node->value);
            if (!v) throw std::runtime_error("Undefined variable '" + node->value + "'");
            return *v;
        }

        case NODE_ARRAY: {
            YoloValue arr = YoloValue::makeArray();
            for (auto& child : node->children)
                arr.aval->push_back(eval(child.get(), scope));
            return arr;
        }

        case NODE_INDEX: {
            YoloValue obj = eval(node->left.get(), scope);
            YoloValue idx = eval(node->right.get(), scope);
            if (obj.isArr()) {
                if (!idx.isInt()) throw std::runtime_error("Array index must be an integer");
                long long i = idx.ival;
                long long sz = (long long)obj.aval->size();
                if (i < 0) i += sz;
                if (i < 0 || i >= sz)
                    throw std::runtime_error("Array index " + std::to_string(idx.ival) + " out of range (size=" + std::to_string(sz) + ")");
                return (*obj.aval)[i];
            }
            if (obj.isStr()) {
                if (!idx.isInt()) throw std::runtime_error("String index must be an integer");
                long long i = idx.ival;
                long long sz = (long long)obj.sval.size();
                if (i < 0) i += sz;
                if (i < 0 || i >= sz)
                    throw std::runtime_error("String index out of range");
                return makeStr(std::string(1, obj.sval[i]));
            }
            throw std::runtime_error("Cannot index a " + obj.typeName());
        }

        case NODE_BINOP: {
            YoloValue lv = eval(node->left.get(), scope);
            YoloValue rv = eval(node->right.get(), scope);

            if (node->value == "+" && (lv.isStr() || rv.isStr()))
                return makeStr(lv.toString() + rv.toString());

            if ((lv.isNum() && rv.isNum()) &&
                (lv.isFloat() || rv.isFloat() || node->value == "/")) {
                double l = lv.toDouble(), r = rv.toDouble();
                YoloValue fresult;
                if (node->value == "+") fresult = makeFloat(l + r);
                else if (node->value == "-") fresult = makeFloat(l - r);
                else if (node->value == "*") fresult = makeFloat(l * r);
                else if (node->value == "/") {
                    if (r == 0.0) throw std::runtime_error("Division by zero");
                    fresult = makeFloat(l / r);
                }
                else if (node->value == "**") fresult = makeFloat(std::pow(l, r));
                else if (node->value == "%") {
                    if (r == 0.0) throw std::runtime_error("Modulo by zero");
                    fresult = makeFloat(std::fmod(l, r));
                }
                else throw std::runtime_error("Unknown operator '" + node->value + "'");
                return applyMoodToArith(fresult, node->value);
            }

            if (!lv.isNum() || !rv.isNum())
                throw std::runtime_error("Operator '" + node->value + "' requires numeric operands");

            long long l = lv.ival, r = rv.ival;
            YoloValue result;
            if (node->value == "+")  result = makeInt(l + r);
            else if (node->value == "-")  result = makeInt(l - r);
            else if (node->value == "*")  result = makeInt(l * r);
            else if (node->value == "**") result = makeFloat(std::pow((double)l, (double)r));
            else if (node->value == "/") {
                if (r == 0) throw std::runtime_error("Division by zero");
                result = makeInt(l / r);
            }
            else if (node->value == "%") {
                if (r == 0) throw std::runtime_error("Modulo by zero");
                result = makeInt(l % r);
            }
            else throw std::runtime_error("Unknown operator '" + node->value + "'");
            return applyMoodToArith(result, node->value);
        }

        case NODE_COMPARE: {
            YoloValue lv = eval(node->left.get(), scope);
            YoloValue rv = eval(node->right.get(), scope);
            if (lv.isStr() || rv.isStr()) {
                std::string l = lv.toString(), r = rv.toString();
                if (node->value == "==") return makeInt(l == r);
                if (node->value == "!=") return makeInt(l != r);
                throw std::runtime_error("Operator '" + node->value + "' not supported for strings");
            }
            double l = lv.toDouble(), r = rv.toDouble();
            if (node->value == "==") return makeInt(l == r);
            if (node->value == "!=") return makeInt(l != r);
            if (node->value == "<")  return makeInt(l <  r);
            if (node->value == ">")  return makeInt(l >  r);
            if (node->value == "<=") return makeInt(l <= r);
            if (node->value == ">=") return makeInt(l >= r);
            throw std::runtime_error("Unknown comparison operator '" + node->value + "'");
        }

        case NODE_LOGOP: {
            if (node->value == "not") return makeInt(!eval(node->left.get(), scope).truthy());
            if (node->value == "and") {
                if (!eval(node->left.get(), scope).truthy()) return makeInt(0);
                return makeInt(eval(node->right.get(), scope).truthy() ? 1 : 0);
            }
            if (node->value == "or") {
                if (eval(node->left.get(), scope).truthy()) return makeInt(1);
                return makeInt(eval(node->right.get(), scope).truthy() ? 1 : 0);
            }
            throw std::runtime_error("Unknown logical operator '" + node->value + "'");
        }

        case NODE_MOODIS: {
            return makeInt(mood_.current == MoodEngine::fromString(node->value) ? 1 : 0);
        }

        case NODE_FUNC_CALL: {
            std::vector<YoloValue> args;
            for (auto& c : node->children) args.push_back(eval(c.get(), scope));
            return callFunction(node->value, std::move(args), scope);
        }

        case NODE_BUILTIN_CALL: {
            std::vector<YoloValue> args;
            for (auto& c : node->children) args.push_back(eval(c.get(), scope));
            return callBuiltin(node->value, std::move(args));
        }

        default:
            throw std::runtime_error("Unexpected node type in eval: " + std::to_string(node->type));
    }
}

// -------------------------------------------------------------------------
// getInput
// -------------------------------------------------------------------------

YoloValue YoloInterpreter::getInput() {
    std::cout.flush();
    std::string line;
    if (!std::getline(std::cin, line)) return makeInt(0);
    try {
        std::size_t p = 0;
        if (line.find('.') != std::string::npos) {
            double d = std::stod(line, &p);
            if (p == line.size()) return makeFloat(d);
        } else {
            long long v = std::stoll(line, &p);
            if (p == line.size()) return makeInt(v);
        }
    } catch (...) {}
    return makeStr(line);
}

// -------------------------------------------------------------------------
// executeStatements
// -------------------------------------------------------------------------

void YoloInterpreter::executeStatements(const ASTNode* node, Scope& scope) {
    if (!node) return;
    for (auto& child : node->children)
        execute(child.get(), scope);
}

// -------------------------------------------------------------------------
// execute
// -------------------------------------------------------------------------

void YoloInterpreter::execute(const ASTNode* node, Scope& scope) {
    if (!node) return;
    mood_.tick();

    switch (node->type) {

        case NODE_STATEMENTS:
            executeStatements(node, scope);
            break;

        case NODE_ASSIGN: {
            YoloValue val = eval(node->right.get(), scope);
            scope.set(node->value, val);
            break;
        }

        case NODE_INDEX_ASSIGN: {
            auto* arrPtr = scope.lookup(node->value);
            if (!arrPtr) throw std::runtime_error("Undefined variable '" + node->value + "'");
            if (!arrPtr->isArr()) throw std::runtime_error("'" + node->value + "' is not an array");
            YoloValue idx = eval(node->left.get(), scope);
            if (!idx.isInt()) throw std::runtime_error("Array index must be an integer");
            long long i  = idx.ival;
            long long sz = (long long)arrPtr->aval->size();
            if (i < 0) i += sz;
            if (i < 0 || i >= sz)
                throw std::runtime_error("Array index out of range");
            (*arrPtr->aval)[i] = eval(node->right.get(), scope);
            break;
        }

        case NODE_PRINT: {
            if (node->value == "__void__") {
                eval(node->left.get(), scope);
            } else {
                YoloValue printVal = eval(node->left.get(), scope);
                if (!moodSuppressOutput())
                    std::cout << printVal.toString();
                std::cout.flush();
            }
            break;
        }

        case NODE_INPUT: {
            scope.set(node->value, getInput());
            break;
        }

        case NODE_IF: {
            if (eval(node->left.get(), scope).truthy()) {
                executeStatements(node->right.get(), scope);
            } else if (node->extra) {
                if (node->extra->type == NODE_IF)
                    execute(node->extra.get(), scope);
                else
                    executeStatements(node->extra.get(), scope);
            }
            break;
        }

        case NODE_WHILE: {
            constexpr long long MAX = 100'000'000LL;
            long long iters = 0;
            while (eval(node->left.get(), scope).truthy()) {
                try {
                    executeStatements(node->right.get(), scope);
                } catch (BreakSignal&)    { break; }
                  catch (ContinueSignal&) { /* continue */ }
                if (++iters > MAX)
                    throw std::runtime_error("Loop exceeded " + std::to_string(MAX) + " iterations. Possible infinite loop.");
            }
            break;
        }

        case NODE_FOR: {
            YoloValue startVal = eval(node->left.get(), scope);
            YoloValue endVal   = eval(node->right.get(), scope);
            YoloValue stepVal  = eval(node->extra2.get(), scope);

            if (!startVal.isNum() || !endVal.isNum() || !stepVal.isNum())
                throw std::runtime_error("'lit' loop bounds and step must be numbers");

            double start = startVal.toDouble();
            double end   = endVal.toDouble();
            double step  = stepVal.toDouble();
            if (step == 0.0) throw std::runtime_error("'lit' loop step cannot be zero");

            constexpr long long MAX = 100'000'000LL;
            long long iters = 0;

            // Use a child scope so the loop variable doesn't clobber outer vars
            Scope loopScope;
            loopScope.parent = &scope;

            for (double i = start;
                 (step > 0 ? i <= end : i >= end);
                 i += step) {
                // Store as int if step is integer and value is whole
                YoloValue iv = (step == std::floor(step) && i == std::floor(i))
                    ? makeInt((long long)i) : makeFloat(i);
                loopScope.setLocal(node->value, iv);
                try {
                    executeStatements(node->extra.get(), loopScope);
                } catch (BreakSignal&)    { break; }
                  catch (ContinueSignal&) { continue; }
                if (++iters > MAX)
                    throw std::runtime_error("For loop exceeded " + std::to_string(MAX) + " iterations.");
            }
            break;
        }

        case NODE_FUNC_DEF: {
            YoloFunc fn;
            fn.params = node->params;
            fn.body   = node->right.get();
            functions_[node->value] = std::move(fn);
            break;
        }

        case NODE_FUNC_CALL: {
            eval(node, scope);
            break;
        }
        case NODE_BUILTIN_CALL: {
            eval(node, scope);
            break;
        }

        case NODE_RETURN: {
            YoloValue val = node->left ? eval(node->left.get(), scope) : makeNil();
            throw ReturnSignal{val};
        }

        case NODE_SET_MOOD: {
            mood_.set(MoodEngine::fromString(node->value));
            break;
        }
        case NODE_MOODCHECK: {
            std::cout << "Current mood: " << MoodEngine::name(mood_.current) << "\n";
            std::cout.flush();
            break;
        }
        case NODE_VIBE_CHECK: {
            Mood newMood = MoodEngine::randomMood();
            mood_.set(newMood);
            break;
        }
        case NODE_BREAK:    throw BreakSignal{};
        case NODE_CONTINUE: throw ContinueSignal{};

        case NODE_EXIT: {
            int code = node->left ? (int)eval(node->left.get(), scope).toDouble() : 0;
            throw ExitSignal{code};
        }

        default:
            throw std::runtime_error("Unexpected statement node: " + std::to_string(node->type));
    }
}

// -------------------------------------------------------------------------
// interpret
// -------------------------------------------------------------------------

bool YoloInterpreter::interpret(const ASTNode* tree) {
    if (!moodSeeded_) { std::srand((unsigned)std::time(nullptr)); moodSeeded_ = true; }
    try {
        execute(tree, globalScope_);
        return true;
    } catch (ExitSignal& ex) {
        return ex.code == 0;
    } catch (ReturnSignal&) {
        std::cerr << "\nYoloScript Runtime Error: 'slay' used outside a function\n";
        return false;
    } catch (BreakSignal&) {
        std::cerr << "\nYoloScript Runtime Error: 'bounce' used outside a loop\n";
        return false;
    } catch (ContinueSignal&) {
        std::cerr << "\nYoloScript Runtime Error: 'skip' used outside a loop\n";
        return false;
    } catch (const std::exception& e) {
        std::cerr << "\nYoloScript Runtime Error: " << e.what() << "\n";
        return false;
    }
}
