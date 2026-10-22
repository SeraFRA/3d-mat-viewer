#ifndef PARSER_H
#define PARSER_H

#include "dynamic_list.h"
#include "token.h"
#include <stddef.h>
#include <stdbool.h>

typedef enum {
    OP_LOAD_NUM,      // Push constant number
    OP_LOAD_VAR,      // Push variable value by index
    OP_ADD,           // a + b
    OP_SUB,           // a - b
    OP_MUL,           // a * b
    OP_DIV,           // a / b
    OP_POW,           // a ^ b
    OP_NEG,           // -a (unary negation)
    OP_FUNC_1ARG,     // 1-argument function: sin, cos, sqrt, etc.
    OP_FUNC_2ARG      // 2-argument function: min, max, atan2, pow
} OpCode;

typedef enum {
    FUNC1_SIN,
    FUNC1_COS,
    FUNC1_TAN,
    FUNC1_ASIN,
    FUNC1_ACOS,
    FUNC1_ATAN,
    FUNC1_SINH,
    FUNC1_COSH,
    FUNC1_TANH,
    FUNC1_EXP,
    FUNC1_LOG,     // Natural logarithm ln
    FUNC1_LOG10,
    FUNC1_SQRT,
    FUNC1_CBRT,
    FUNC1_ABS,
    FUNC1_FLOOR,
    FUNC1_CEIL,
    FUNC1_UNKNOWN
} MathFunc1Arg;

typedef enum {
    FUNC2_MIN,
    FUNC2_MAX,
    FUNC2_ATAN2,
    FUNC2_POW,
    FUNC2_UNKNOWN
} MathFunc2Arg;

typedef struct {
    OpCode opcode;
    union {
        double number;           // For OP_LOAD_NUM
        struct {
            int index;
            char name[MAX_IDENTIFIER_LEN];
        } variable;              // For OP_LOAD_VAR
        struct {
            MathFunc1Arg id;
            char name[MAX_IDENTIFIER_LEN];
        } func1;                 // For OP_FUNC_1ARG
        struct {
            MathFunc2Arg id;
            char name[MAX_IDENTIFIER_LEN];
        } func2;                 // For OP_FUNC_2ARG
    };
} Instruction;

typedef struct {
    char name[MAX_IDENTIFIER_LEN];
    int index;
} Variable;

typedef enum {
    PARSE_SUCCESS = 0,
    PARSE_ERROR_EMPTY_INPUT,
    PARSE_ERROR_INVALID_CHAR,
    PARSE_ERROR_UNEXPECTED_TOKEN,
    PARSE_ERROR_MISMATCHED_PAREN,
    PARSE_ERROR_UNKNOWN_FUNCTION,
    PARSE_ERROR_MISSING_OPERAND,
    PARSE_ERROR_TOO_MANY_OPERANDS,
    PARSE_ERROR_OUT_OF_MEMORY
} ParseStatus;

typedef struct {
    char source[256];
    DynamicList tokens;        // List of Token
    DynamicList variables;     // List of Variable (unique variables detected)
    DynamicList instructions;  // List of Instruction (RPN execution instructions)
} ParsedFunction;

/**
 * @brief Initialize a ParsedFunction container.
 */
void parsed_function_init(ParsedFunction *pf);

/**
 * @brief Free all dynamic memory in a ParsedFunction.
 */
void parsed_function_free(ParsedFunction *pf);

/**
 * @brief Parse a mathematical string expression into tokens, variables, and an RPN instruction list.
 *
 * @param expression The mathematical expression as string (e.g. "x^2 + y^2 + z^2 - 1")
 * @param pf Output ParsedFunction struct containing dynamic lists
 * @param err_msg Buffer to store error description if parsing fails (can be NULL)
 * @param err_msg_len Size of err_msg buffer
 * @return ParseStatus PARSE_SUCCESS if parsed successfully, error code otherwise
 */
ParseStatus parse_expression(const char *expression, ParsedFunction *pf, char *err_msg, size_t err_msg_len);

/**
 * @brief Find the index of a variable by name in the parsed function.
 * @return Variable index, or -1 if not found.
 */
int parsed_function_find_var_index(const ParsedFunction *pf, const char *name);

/**
 * @brief Print all tokens in the dynamic list to stdout.
 */
void parsed_function_print_tokens(const ParsedFunction *pf);

/**
 * @brief Print all detected variables in the dynamic list to stdout.
 */
void parsed_function_print_variables(const ParsedFunction *pf);

/**
 * @brief Print all parsed operations/instructions in the dynamic list to stdout.
 */
void parsed_function_print_instructions(const ParsedFunction *pf);

/**
 * @brief Print a full summary of the parsed function.
 */
void parsed_function_dump(const ParsedFunction *pf);

#endif /* PARSER_H */
