# YoloScript Programming Language

> *"Life's too short for boring syntax. Send it."*

YoloScript is a meme-flavored, interpreted programming language written in C++. Every keyword is pulled straight from internet slang — `yolo` declares a variable, `sendit` prints output, `bruh` is your `if`, and `fr` opens a block. It's silly on the surface, but underneath sits a real tree-walk interpreter with a surprising feature set: first-class functions, arrays, floats, string builtins, a `for`-loop, and — its most distinctive trick — a **Mood Engine** that changes how arithmetic behaves at runtime.

---

## Table of Contents

1. [Building & Running](#building--running)
2. [Language Quick-Reference](#language-quick-reference)
3. [Unique Features](#unique-features)
4. [The Mood Engine — Deep Dive](#the-mood-engine--deep-dive)
5. [Built-in Functions](#built-in-functions)
6. [Error Handling](#error-handling)
7. [What YoloScript Lacks](#what-yoloscript-lacks)
8. [Internals & Architecture](#internals--architecture)
9. [Full Language Reference](#full-language-reference)
10. [Example Programs](#example-programs)
11. [Roadmap](#roadmap)

---

## Building & Running

**Requirements:** `g++` with C++17 support, `make`

```bash
git clone https://github.com/kUrOSH1R0oo/YoloScript.git
cd YoloScript
make
```

This produces the `yoloscript` executable.

```bash
./yoloscript yourfile.yolo    # run a .yolo file
./yoloscript                  # run the built-in demo
./yoloscript --help           # show help and keyword list
make clean                    # remove build artifacts
```

---

## Language Quick-Reference

### Keywords

| Keyword | Role | Equivalent in a mainstream language |
|---------|------|-------------------------------------|
| `yolo` | declare / assign variable | `var` / `let` |
| `sendit` | print to stdout | `print` |
| `yeet` | read a line of user input | `input()` |
| `bruh` | `if` statement | `if` |
| `lowkey` | `else if` | `elif` / `else if` |
| `nah` | `else` | `else` |
| `goat` | `while` loop | `while` |
| `lit` | `for` / range loop | `for i in range(...)` |
| `to` | upper bound in `lit` loop | — |
| `step` | step size in `lit` loop | — |
| `fr` | open a block | `{` |
| `nocap` | close a block | `}` |
| `meh` | end of statement | `;` |
| `vibe` | define a function | `def` / `function` |
| `slay` | return from a function | `return` |
| `squad` | array literal | `[]` |
| `bounce` | break out of a loop | `break` |
| `skip` | skip to next loop iteration | `continue` |
| `bye` | exit the program | `exit()` / `sys.exit()` |
| `true` / `false` | boolean literals | `true` / `false` |
| `and` / `or` / `not` | logical operators | `&&` / `\|\|` / `!` |
| `#` | line comment | `//` |
| `mood` | set the Mood Engine state | *(unique)* |
| `moodcheck` | print current mood | *(unique)* |
| `moodis` | test current mood (returns bool) | *(unique)* |
| `vibecheck` | randomly shift the current mood | *(unique)* |

### Operators

| Operator | Description |
|----------|-------------|
| `+` `-` `*` `/` `%` | Arithmetic |
| `**` | Exponentiation |
| `==` `!=` `<` `>` `<=` `>=` | Comparison |
| `and` `or` `not` | Logical |
| `+` (strings) | Concatenation |

---

## Unique Features

### 1. Meme-Syntax That Actually Works

YoloScript is not a joke language in the tradition of Brainfuck or Whitespace — it is fully functional. The slang keywords map one-to-one onto real control flow constructs, and the syntax is consistent enough to write real programs with. `meh` as a statement terminator is the only real ergonomic hurdle.

### 2. The Mood Engine 🎭

This is YoloScript's signature feature. The interpreter carries a **global mood state** that can alter how arithmetic evaluates, how output is printed, and when loops run. There are six moods:

| Mood | Effect |
|------|--------|
| `normal` | Standard behavior — no distortions |
| `hyped` | All arithmetic results are **multiplied by 2** |
| `chill` | Float results are **floored to integers** |
| `sus` | 20% of `sendit` outputs are silently replaced with `???` |
| `lowbattery` | All arithmetic results are **halved** |
| `chaos` | Addition and subtraction have a **33% chance of flipping the sign** |

Mood is set with `mood <name>`, queried with `moodcheck`, tested in an expression with `moodis <name>`, and randomly shifted with `vibecheck`. Moods also decay back to `normal` after a configurable number of executed statements, so effects are temporary by design.

This makes YoloScript useful for chaos-testing: you can deliberately skew arithmetic to verify your program handles unexpected values correctly, or just make demos more entertaining.

### 3. Functions with Lexical Scoping

Functions are first-class citizens declared with `vibe` and closed with `nocap`. They support any number of parameters, have their own local scope (parent chain goes to global), and use `slay` to return a value.

```
vibe add(a, b) fr
    slay a + b meh
nocap

yolo result = add(10, 32) meh
sendit result meh   # 42
```

### 4. Arrays with Negative Indexing

Arrays are created with the `squad` literal syntax and support Python-style negative indexing (`arr[-1]` gives the last element). Elements are zero-indexed, mutable, and can hold mixed types.

```
yolo nums = squad [10, 20, 30, 40] meh
sendit nums[-1] meh   # 40
yolo nums[0] = 99 meh
```

### 5. Floats as First-Class Values

Division always promotes to float. The `**` operator supports fractional exponents. Float literals use a dot (`3.14`). Integer arithmetic stays integer when both operands are ints and the operator is not `/`.

### 6. `lit` For-Loop (Range Loop)

In addition to `goat` (while), YoloScript has a dedicated counted `lit` loop with configurable start, end, and step — no manual counter bookkeeping needed.

```
lit i = 1 to 10 step 2 fr
    sendit i meh
    sendit " " meh
nocap
# prints: 1 3 5 7 9
```

The `lit` loop variable is scoped to the loop body and supports float step values.

### 7. Rich String Builtins

YoloScript ships `upper`, `lower`, `trim`, `contains`, `replace`, `split`, and `join` as built-in functions — enough to do real text processing without a standard library.

### 8. Auto-Typing on Input

`yeet` reads a line from stdin and automatically stores it as an integer, a float, or a string, depending on what the content looks like. No manual `int()` cast needed in common cases.

---

## The Mood Engine — Deep Dive

The `MoodEngine` lives inside the interpreter and is checked on every `execute()` call. It also has a **decay counter**: after a configurable number of statements, any non-normal mood automatically reverts to `normal`, logging a message to stderr. This prevents a `mood chaos` at the top of a file from ruining the entire program.

```
mood hyped fr      # arithmetic now doubles results
yolo x = 5 + 3 meh
sendit x meh       # prints 16, not 8 (8 * 2 = 16)
moodcheck          # prints "Current mood: hyped"
nocap

bruh moodis normal fr
    sendit "we're back to baseline\n" meh
nocap
```

The `vibecheck` keyword randomly picks one of the six moods — useful for stochastic testing or just chaos.

> **Design note:** Moods that affect arithmetic (`hyped`, `lowbattery`, `chaos`, `chill`) are applied *after* the computation, not inside it, so operator precedence and type promotion are unaffected. The result is then post-processed by `applyMoodToArith`.

---

## Built-in Functions

These are called like regular function calls and are resolved before user-defined functions are checked.

| Function | Signature | Description |
|----------|-----------|-------------|
| `len` | `len(s or arr)` | Length of a string or array |
| `str` | `str(x)` | Convert any value to string |
| `num` | `num(s)` | Parse a string as a number |
| `abs` | `abs(n)` | Absolute value |
| `sqrt` | `sqrt(n)` | Square root (returns float) |
| `floor` | `floor(n)` | Floor to integer |
| `ceil` | `ceil(n)` | Ceiling to integer |
| `max` | `max(a, b, ...)` or `max(arr)` | Maximum value |
| `min` | `min(a, b, ...)` or `min(arr)` | Minimum value |
| `type` | `type(x)` | Returns the type name as a string |
| `push` | `push(arr, val)` | Append to array, returns new array |
| `pop` | `pop(arr)` | Remove and return last element |
| `join` | `join(arr, sep)` | Join array elements into a string |
| `split` | `split(s, delim)` | Split string into array |
| `upper` | `upper(s)` | Uppercase string |
| `lower` | `lower(s)` | Lowercase string |
| `trim` | `trim(s)` | Strip leading/trailing whitespace |
| `contains` | `contains(s or arr, val)` | Check membership |
| `replace` | `replace(s, from, to)` | Replace all occurrences |

---

## Error Handling

YoloScript has three categories of errors, all with descriptive messages. There is **no try/catch mechanism** — errors terminate the program.

### Syntax Errors (at parse time)

```
YoloScript Syntax Error: Expected variable name after 'yolo' (line 3)
YoloScript Syntax Error: Unterminated string literal at line 7
YoloScript Syntax Error: Unknown character '@' (ASCII 64) at line 2
```

### Runtime Errors (during execution)

```
YoloScript Runtime Error: Undefined variable 'score'
YoloScript Runtime Error: Division by zero
YoloScript Runtime Error: Loop exceeded 100000000 iterations. Possible infinite loop.
YoloScript Runtime Error: Array index 5 out of range (size=3)
YoloScript Runtime Error: 'bounce' used outside a loop
YoloScript Runtime Error: 'slay' used outside a function
```

### Type Errors

```
YoloScript Runtime Error: Operator '*' requires numeric operands
YoloScript Runtime Error: len() requires a string or array
YoloScript Runtime Error: Cannot convert "abc" to a number
```

---

## What YoloScript Lacks

YoloScript is a hobby language, and that shows in some deliberate (and some unintentional) omissions. Here is an honest accounting:

### No Exception Handling

There is no `try`/`catch`. Any runtime error kills the process. Programs cannot recover from bad input gracefully inside the language itself.

### No File I/O

There is no way to open, read, or write files. Input comes only from stdin via `yeet`, and output goes only to stdout via `sendit`. Building anything that persists data requires shelling out.

### No Closures

Functions have access to the global scope and their own local scope, but they **cannot capture variables from an enclosing function's scope**. There are no closures. Higher-order programming is possible (you can pass values into functions), but you cannot return a function that "remembers" a surrounding environment.

### No First-Class Functions

Functions are not values — you cannot store a function in a variable, pass it as an argument, or return it from another function. `vibe` definitions live in a separate function table, not in the variable namespace.

### No Hash Maps / Dictionaries

Arrays are the only collection type. There are no key-value maps, sets, or associative structures. Simulating a dictionary requires parallel arrays, which is awkward.

### No Modules or Imports

Every YoloScript program is a single file. There is no `import` or `include`. Code reuse across files is impossible without manually concatenating sources.

### No String Slicing

While you can index individual characters with `arr[i]` syntax on a string, there is no slice notation (`s[1:5]`). Substring extraction requires `split`, `replace`, or a manual loop.

### No Recursion Depth Protection

Deeply recursive functions will crash with a C++ stack overflow rather than a friendly YoloScript error message. The loop iteration guard (100 million iterations) does not apply to recursive calls.

### No Multi-line Strings

String literals cannot span multiple lines. Long text must be concatenated with `+`.

### Integer-Only Modulo for Floats Conceptually

While `%` works on floats via `fmod`, the semantics can surprise: `5.0 % 2` returns `1.0` but `5.1 % 2` returns `1.1`, consistent with C `fmod` behavior, not Python's `%`.

### No Boolean Type

Booleans are represented as integers (`0` and `1`). The `true` and `false` literals evaluate to `1` and `0`. `type(true)` returns `"int"`.

### No Standard Math Constants

There is no built-in `pi`, `e`, or `inf`. You must define them yourself: `yolo PI = 3.14159265358979 meh`.

---

## Internals & Architecture

```
YoloScript/
├── main.cpp           # Entry point, CLI argument handling, demo program
├── lexer.hpp/cpp      # Tokenizer — converts source text into a Token stream
├── parser.hpp/cpp     # Recursive-descent parser — builds the AST
├── interpreter.hpp/cpp# Tree-walk interpreter — walks the AST and executes it
├── compiler.hpp/cpp   # Thin glue layer: lex → parse → interpret
├── ast.hpp            # ASTNode and NodeType definitions
├── tokens.hpp         # Token struct and TokenType enum
└── utils.hpp/cpp      # File/stdin reading helpers
```

**Execution pipeline:** Source text → `YoloLexer` (tokenize) → `YoloParser` (recursive-descent parse, produces an `ASTNode` tree) → `YoloInterpreter` (tree-walk execution with a `Scope` chain and `MoodEngine`).

The interpreter uses C++ exception-based control flow for `bounce` (break), `skip` (continue), `slay` (return), and `bye` (exit) — each throws a distinct signal struct that is caught at the appropriate execution boundary.

Scopes form a parent chain: function calls get a fresh `Scope` whose parent points to `globalScope_`. There is no closure capture; `lookup()` walks the chain to global, but not into sibling or enclosing function scopes.

---

## Full Language Reference

### Variables

Variables are declared and assigned with `yolo`. Re-assignment uses the same syntax. Types are dynamic — a variable can hold an int, then be reassigned to a string.

```
yolo x = 42 meh
yolo name = "Alice" meh
yolo pi = 3.14159 meh
yolo flag = true meh
yolo x = "now a string" meh   # re-assign
```

### Printing

`sendit` does **not** add a newline — include `"\n"` explicitly. String + non-string auto-converts the non-string operand.

```
sendit "Hello, " + name + "!\n" meh
sendit x meh
```

### Input

`yeet` reads one line from stdin. If it looks like an integer or float, it is stored as one automatically.

```
sendit "Enter your age: " meh
yeet age meh
```

### If / Else-if / Else

```
bruh score >= 90 fr
    sendit "A\n" meh
nocap lowkey score >= 80 fr
    sendit "B\n" meh
nocap nah fr
    sendit "F\n" meh
nocap
```

### While Loop

```
yolo i = 0 meh
goat i < 10 fr
    sendit i meh
    sendit " " meh
    yolo i = i + 1 meh
nocap
```

### For Loop

```
lit i = 0 to 100 step 10 fr
    sendit i meh
    sendit " " meh
nocap
# 0 10 20 30 40 50 60 70 80 90 100
```

Negative steps work for counting down:

```
lit i = 5 to 1 step -1 fr
    sendit i meh
    sendit " " meh
nocap
# 5 4 3 2 1
```

### Functions

```
vibe greet(name) fr
    sendit "Hello, " + name + "!\n" meh
nocap

vibe factorial(n) fr
    bruh n <= 1 fr
        slay 1 meh
    nocap
    slay n * factorial(n - 1) meh
nocap

greet("world") meh
sendit factorial(5) meh   # 120
```

### Arrays

```
yolo primes = squad [2, 3, 5, 7, 11] meh
sendit primes[0] meh           # 2
sendit primes[-1] meh          # 11
yolo primes[0] = 999 meh       # mutate
yolo primes = push(primes, 13) meh
sendit len(primes) meh         # 6
yolo last = pop(primes) meh
```

### Break / Continue / Exit

```
goat true fr
    yeet val meh
    bruh val == 0 fr
        bounce meh        # break
    nocap
    bruh val < 0 fr
        skip meh          # continue (skips rest of body)
    nocap
    sendit val meh
nocap

bye 0 meh   # exit with code 0
```

### Mood System

```
mood hyped fr
    yolo x = 10 + 5 meh   # = 30, not 15
    sendit x meh
nocap

moodcheck          # prints current mood
vibecheck          # randomize mood
bruh moodis chill fr
    sendit "taking it easy\n" meh
nocap
```

---

## Example Programs

### Hello World

```
sendit "Hello, World!\n" meh
```

### FizzBuzz (1–100)

```
lit i = 1 to 100 step 1 fr
    bruh i % 15 == 0 fr
        sendit "FizzBuzz\n" meh
    nocap lowkey i % 3 == 0 fr
        sendit "Fizz\n" meh
    nocap lowkey i % 5 == 0 fr
        sendit "Buzz\n" meh
    nocap nah fr
        sendit i meh
        sendit "\n" meh
    nocap
nocap
```

### Fibonacci (iterative)

```
yolo a = 0 meh
yolo b = 1 meh
lit i = 0 to 14 step 1 fr
    sendit a meh
    sendit " " meh
    yolo temp = a + b meh
    yolo a = b meh
    yolo b = temp meh
nocap
sendit "\n" meh
```

### Bubble Sort

```
vibe bubble_sort(arr) fr
    yolo n = len(arr) meh
    yolo i = 0 meh
    goat i < n - 1 fr
        yolo j = 0 meh
        goat j < n - i - 1 fr
            bruh arr[j] > arr[j + 1] fr
                yolo tmp = arr[j] meh
                yolo arr[j] = arr[j + 1] meh
                yolo arr[j + 1] = tmp meh
            nocap
            yolo j = j + 1 meh
        nocap
        yolo i = i + 1 meh
    nocap
    slay arr meh
nocap
```

---

## Roadmap

- [ ] Hash maps / dictionaries (`drip { "key": value }`)
- [ ] String slicing (`s[1:5]`)
- [ ] First-class functions (functions as values)
- [ ] Module system / `import`
- [ ] Try/catch error handling
- [ ] Multi-line string literals
- [ ] Built-in math constants (`PI`, `E`)
- [ ] A proper REPL with history
- [ ] Bytecode compilation for speed

---

## License

MIT — do whatever you want fam. No cap.