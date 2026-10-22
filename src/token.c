#include "token.h"
#include <stdio.h>

const char *token_type_to_string(MathTokenType type) {
    switch (type) {
        case TOKEN_NUMBER:   return "NUMBER";
        case TOKEN_VARIABLE: return "VARIABLE";
        case TOKEN_FUNCTION: return "FUNCTION";
        case TOKEN_CONSTANT: return "CONSTANT";
        case TOKEN_OP_ADD:   return "OP_ADD (+)";
        case TOKEN_OP_SUB:   return "OP_SUB (-)";
        case TOKEN_OP_MUL:   return "OP_MUL (*)";
        case TOKEN_OP_DIV:   return "OP_DIV (/)";
        case TOKEN_OP_POW:   return "OP_POW (^)";
        case TOKEN_OP_NEG:   return "OP_NEG (unary -)";
        case TOKEN_LPAREN:   return "LPAREN ('(')";
        case TOKEN_RPAREN:   return "RPAREN (')')";
        case TOKEN_COMMA:    return "COMMA (',')";
        case TOKEN_EOF:      return "EOF";
        case TOKEN_UNKNOWN:  return "UNKNOWN";
        default:             return "INVALID";
    }
}

bool token_is_operator(MathTokenType type) {
    return type == TOKEN_OP_ADD ||
           type == TOKEN_OP_SUB ||
           type == TOKEN_OP_MUL ||
           type == TOKEN_OP_DIV ||
           type == TOKEN_OP_POW ||
           type == TOKEN_OP_NEG;
}

int token_operator_precedence(MathTokenType type) {
    switch (type) {
        case TOKEN_OP_ADD:
        case TOKEN_OP_SUB:
            return 1;
        case TOKEN_OP_MUL:
        case TOKEN_OP_DIV:
            return 2;
        case TOKEN_OP_NEG:
            return 3;
        case TOKEN_OP_POW:
            return 4;
        default:
            return 0;
    }
}

bool token_is_right_associative(MathTokenType type) {
    return type == TOKEN_OP_POW || type == TOKEN_OP_NEG;
}

void token_print(const Token *token) {
    if (!token) {
        printf("(null token)\n");
        return;
    }

    switch (token->type) {
        case TOKEN_NUMBER:
            printf("[Token NUMBER val=%g pos=%zu]", token->value, token->position);
            break;
        case TOKEN_VARIABLE:
            printf("[Token VARIABLE name='%s' pos=%zu]", token->name, token->position);
            break;
        case TOKEN_FUNCTION:
            printf("[Token FUNCTION name='%s' pos=%zu]", token->name, token->position);
            break;
        case TOKEN_CONSTANT:
            printf("[Token CONSTANT name='%s' val=%g pos=%zu]", token->name, token->value, token->position);
            break;
        default:
            printf("[Token %s pos=%zu]", token_type_to_string(token->type), token->position);
            break;
    }
}
