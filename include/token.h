#ifndef TOKEN_H
#define TOKEN_H

#include <stddef.h>
#include <stdbool.h>

#define MAX_IDENTIFIER_LEN 64

typedef enum {
    TOKEN_NUMBER,
    TOKEN_VARIABLE,
    TOKEN_FUNCTION,
    TOKEN_CONSTANT,
    TOKEN_OP_ADD,
    TOKEN_OP_SUB,
    TOKEN_OP_MUL,
    TOKEN_OP_DIV,
    TOKEN_OP_POW,
    TOKEN_OP_NEG, // Unary minus
    TOKEN_LPAREN,
    TOKEN_RPAREN,
    TOKEN_COMMA,
    TOKEN_EOF,
    TOKEN_UNKNOWN
} TokenType;

typedef struct {
    TokenType type;
    double value;                     // For TOKEN_NUMBER or TOKEN_CONSTANT
    char name[MAX_IDENTIFIER_LEN];    // For variable or function name
    size_t position;                  // Character offset in expression
} Token;

/**
 * @brief Get human-readable string representation of token type.
 */
const char *token_type_to_string(TokenType type);

/**
 * @brief Check if a token type is an operator.
 */
bool token_is_operator(TokenType type);

/**
 * @brief Get operator precedence level (higher means evaluated first).
 */
int token_operator_precedence(TokenType type);

/**
 * @brief Check if operator is right-associative (e.g. exponentiation `^`, unary minus).
 */
bool token_is_right_associative(TokenType type);

/**
 * @brief Print a token to standard output for debugging/inspection.
 */
void token_print(const Token *token);

#endif /* TOKEN_H */
