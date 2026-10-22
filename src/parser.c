#include "parser.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <ctype.h>
#include <math.h>

#ifndef M_PI
#define M_PI 3.14159265358979323846
#endif

#ifndef M_E
#define M_E 2.71828182845904523536
#endif

static bool str_equals_case_insensitive(const char *s1, const char *s2) {
    if (!s1 || !s2) return false;
    while (*s1 && *s2) {
        if (tolower((unsigned char)*s1) != tolower((unsigned char)*s2)) {
            return false;
        }
        s1++;
        s2++;
    }
    return (*s1 == '\0' && *s2 == '\0');
}

void parsed_function_init(ParsedFunction *pf) {
    if (!pf) return;
    memset(pf, 0, sizeof(ParsedFunction));
    dynamic_list_init(&pf->tokens, sizeof(Token), 16);
    dynamic_list_init(&pf->variables, sizeof(Variable), 4);
    dynamic_list_init(&pf->instructions, sizeof(Instruction), 16);
}

void parsed_function_free(ParsedFunction *pf) {
    if (!pf) return;
    dynamic_list_free(&pf->tokens);
    dynamic_list_free(&pf->variables);
    dynamic_list_free(&pf->instructions);
    pf->source[0] = '\0';
}

int parsed_function_find_var_index(const ParsedFunction *pf, const char *name) {
    if (!pf || !name) return -1;
    for (size_t i = 0; i < pf->variables.count; i++) {
        const Variable *var = (const Variable *)dynamic_list_get(&pf->variables, i);
        if (var && str_equals_case_insensitive(var->name, name)) {
            return var->index;
        }
    }
    return -1;
}

static int register_or_get_variable(ParsedFunction *pf, const char *name) {
    int idx = parsed_function_find_var_index(pf, name);
    if (idx >= 0) {
        return idx;
    }

    Variable new_var;
    strncpy(new_var.name, name, MAX_IDENTIFIER_LEN - 1);
    new_var.name[MAX_IDENTIFIER_LEN - 1] = '\0';
    new_var.index = (int)pf->variables.count;

    dynamic_list_push(&pf->variables, &new_var);
    return new_var.index;
}

static bool match_1arg_function(const char *name, MathFunc1Arg *out_func) {
    if (str_equals_case_insensitive(name, "sin"))   { *out_func = FUNC1_SIN;   return true; }
    if (str_equals_case_insensitive(name, "cos"))   { *out_func = FUNC1_COS;   return true; }
    if (str_equals_case_insensitive(name, "tan"))   { *out_func = FUNC1_TAN;   return true; }
    if (str_equals_case_insensitive(name, "asin"))  { *out_func = FUNC1_ASIN;  return true; }
    if (str_equals_case_insensitive(name, "acos"))  { *out_func = FUNC1_ACOS;  return true; }
    if (str_equals_case_insensitive(name, "atan"))  { *out_func = FUNC1_ATAN;  return true; }
    if (str_equals_case_insensitive(name, "sinh"))  { *out_func = FUNC1_SINH;  return true; }
    if (str_equals_case_insensitive(name, "cosh"))  { *out_func = FUNC1_COSH;  return true; }
    if (str_equals_case_insensitive(name, "tanh"))  { *out_func = FUNC1_TANH;  return true; }
    if (str_equals_case_insensitive(name, "exp"))   { *out_func = FUNC1_EXP;   return true; }
    if (str_equals_case_insensitive(name, "log") || str_equals_case_insensitive(name, "ln")) {
        *out_func = FUNC1_LOG;
        return true;
    }
    if (str_equals_case_insensitive(name, "log10")) { *out_func = FUNC1_LOG10; return true; }
    if (str_equals_case_insensitive(name, "sqrt"))  { *out_func = FUNC1_SQRT;  return true; }
    if (str_equals_case_insensitive(name, "cbrt"))  { *out_func = FUNC1_CBRT;  return true; }
    if (str_equals_case_insensitive(name, "abs") || str_equals_case_insensitive(name, "fabs")) {
        *out_func = FUNC1_ABS;
        return true;
    }
    if (str_equals_case_insensitive(name, "floor")) { *out_func = FUNC1_FLOOR; return true; }
    if (str_equals_case_insensitive(name, "ceil"))  { *out_func = FUNC1_CEIL;  return true; }
    return false;
}

static bool match_2arg_function(const char *name, MathFunc2Arg *out_func) {
    if (str_equals_case_insensitive(name, "min"))   { *out_func = FUNC2_MIN;   return true; }
    if (str_equals_case_insensitive(name, "max"))   { *out_func = FUNC2_MAX;   return true; }
    if (str_equals_case_insensitive(name, "atan2")) { *out_func = FUNC2_ATAN2; return true; }
    if (str_equals_case_insensitive(name, "pow"))   { *out_func = FUNC2_POW;   return true; }
    return false;
}

static bool match_constant(const char *name, double *out_val) {
    if (str_equals_case_insensitive(name, "pi")) {
        *out_val = M_PI;
        return true;
    }
    if (str_equals_case_insensitive(name, "e")) {
        *out_val = M_E;
        return true;
    }
    return false;
}

static bool is_operand_or_rparen(MathTokenType type) {
    return type == TOKEN_NUMBER ||
           type == TOKEN_VARIABLE ||
           type == TOKEN_CONSTANT ||
           type == TOKEN_RPAREN;
}

/**
 * @brief Tokenize the expression into pf->tokens.
 */
static ParseStatus tokenize_expression(const char *expr, ParsedFunction *pf, char *err_msg, size_t err_msg_len) {
    size_t i = 0;
    size_t len = strlen(expr);

    while (i < len) {
        char c = expr[i];

        if (isspace((unsigned char)c)) {
            i++;
            continue;
        }

        // Check if we need implicit multiplication before pushing an operand or '('
        Token *prev = (Token *)dynamic_list_peek(&pf->tokens);

        // Digits / floating point numbers
        if (isdigit((unsigned char)c) || (c == '.' && i + 1 < len && isdigit((unsigned char)expr[i + 1]))) {
            if (prev && is_operand_or_rparen(prev->type)) {
                Token mul_tok = { .type = TOKEN_OP_MUL, .position = i };
                dynamic_list_push(&pf->tokens, &mul_tok);
            }

            char *end_ptr = NULL;
            double val = strtod(&expr[i], &end_ptr);
            size_t consumed = end_ptr - &expr[i];

            Token num_tok = {
                .type = TOKEN_NUMBER,
                .value = val,
                .position = i
            };
            dynamic_list_push(&pf->tokens, &num_tok);
            i += consumed;
            continue;
        }

        // Identifiers: variables, functions, constants
        if (isalpha((unsigned char)c) || c == '_') {
            size_t start = i;
            while (i < len && (isalnum((unsigned char)expr[i]) || expr[i] == '_')) {
                i++;
            }
            size_t id_len = i - start;
            char name_buf[MAX_IDENTIFIER_LEN];
            if (id_len >= MAX_IDENTIFIER_LEN) {
                id_len = MAX_IDENTIFIER_LEN - 1;
            }
            strncpy(name_buf, &expr[start], id_len);
            name_buf[id_len] = '\0';

            MathFunc1Arg f1;
            MathFunc2Arg f2;
            double const_val;

            Token tok = { .position = start };

            if (match_1arg_function(name_buf, &f1) || match_2arg_function(name_buf, &f2)) {
                if (prev && is_operand_or_rparen(prev->type)) {
                    Token mul_tok = { .type = TOKEN_OP_MUL, .position = start };
                    dynamic_list_push(&pf->tokens, &mul_tok);
                }
                tok.type = TOKEN_FUNCTION;
                snprintf(tok.name, sizeof(tok.name), "%s", name_buf);
            } else if (match_constant(name_buf, &const_val)) {
                if (prev && is_operand_or_rparen(prev->type)) {
                    Token mul_tok = { .type = TOKEN_OP_MUL, .position = start };
                    dynamic_list_push(&pf->tokens, &mul_tok);
                }
                tok.type = TOKEN_CONSTANT;
                tok.value = const_val;
                snprintf(tok.name, sizeof(tok.name), "%s", name_buf);
            } else {
                // Variable
                if (prev && is_operand_or_rparen(prev->type)) {
                    Token mul_tok = { .type = TOKEN_OP_MUL, .position = start };
                    dynamic_list_push(&pf->tokens, &mul_tok);
                }
                tok.type = TOKEN_VARIABLE;
                snprintf(tok.name, sizeof(tok.name), "%s", name_buf);
                register_or_get_variable(pf, name_buf);
            }

            dynamic_list_push(&pf->tokens, &tok);
            continue;
        }

        // Operators & delimiters
        switch (c) {
            case '+': {
                // Check if unary plus
                if (!prev || (!is_operand_or_rparen(prev->type) && prev->type != TOKEN_RPAREN)) {
                    // Unary plus: effectively a no-op, just skip
                    i++;
                    continue;
                }
                Token t = { .type = TOKEN_OP_ADD, .position = i };
                dynamic_list_push(&pf->tokens, &t);
                i++;
                break;
            }
            case '-': {
                // Determine whether unary negation or binary subtraction
                Token t = { .position = i };
                if (!prev || (!is_operand_or_rparen(prev->type) && prev->type != TOKEN_RPAREN)) {
                    t.type = TOKEN_OP_NEG;
                } else {
                    t.type = TOKEN_OP_SUB;
                }
                dynamic_list_push(&pf->tokens, &t);
                i++;
                break;
            }
            case '*': {
                Token t = { .type = TOKEN_OP_MUL, .position = i };
                dynamic_list_push(&pf->tokens, &t);
                i++;
                break;
            }
            case '/': {
                Token t = { .type = TOKEN_OP_DIV, .position = i };
                dynamic_list_push(&pf->tokens, &t);
                i++;
                break;
            }
            case '^': {
                Token t = { .type = TOKEN_OP_POW, .position = i };
                dynamic_list_push(&pf->tokens, &t);
                i++;
                break;
            }
            case '(': {
                if (prev && is_operand_or_rparen(prev->type)) {
                    Token mul_tok = { .type = TOKEN_OP_MUL, .position = i };
                    dynamic_list_push(&pf->tokens, &mul_tok);
                }
                Token t = { .type = TOKEN_LPAREN, .position = i };
                dynamic_list_push(&pf->tokens, &t);
                i++;
                break;
            }
            case ')': {
                Token t = { .type = TOKEN_RPAREN, .position = i };
                dynamic_list_push(&pf->tokens, &t);
                i++;
                break;
            }
            case ',': {
                Token t = { .type = TOKEN_COMMA, .position = i };
                dynamic_list_push(&pf->tokens, &t);
                i++;
                break;
            }
            default: {
                if (err_msg && err_msg_len > 0) {
                    snprintf(err_msg, err_msg_len, "Invalid character '%c' at position %zu", c, i);
                }
                return PARSE_ERROR_INVALID_CHAR;
            }
        }
    }

    if (pf->tokens.count == 0) {
        if (err_msg && err_msg_len > 0) {
            snprintf(err_msg, err_msg_len, "Expression is empty");
        }
        return PARSE_ERROR_EMPTY_INPUT;
    }

    return PARSE_SUCCESS;
}

static bool emit_operator_instruction(ParsedFunction *pf, MathTokenType op_type) {
    Instruction instr;
    memset(&instr, 0, sizeof(Instruction));

    switch (op_type) {
        case TOKEN_OP_ADD: instr.opcode = OP_ADD; break;
        case TOKEN_OP_SUB: instr.opcode = OP_SUB; break;
        case TOKEN_OP_MUL: instr.opcode = OP_MUL; break;
        case TOKEN_OP_DIV: instr.opcode = OP_DIV; break;
        case TOKEN_OP_POW: instr.opcode = OP_POW; break;
        case TOKEN_OP_NEG: instr.opcode = OP_NEG; break;
        default: return false;
    }

    return dynamic_list_push(&pf->instructions, &instr);
}

static bool emit_function_instruction(ParsedFunction *pf, const char *func_name) {
    Instruction instr;
    memset(&instr, 0, sizeof(Instruction));

    MathFunc1Arg f1;
    MathFunc2Arg f2;

    if (match_1arg_function(func_name, &f1)) {
        instr.opcode = OP_FUNC_1ARG;
        instr.func1.id = f1;
        strncpy(instr.func1.name, func_name, MAX_IDENTIFIER_LEN - 1);
        return dynamic_list_push(&pf->instructions, &instr);
    } else if (match_2arg_function(func_name, &f2)) {
        instr.opcode = OP_FUNC_2ARG;
        instr.func2.id = f2;
        strncpy(instr.func2.name, func_name, MAX_IDENTIFIER_LEN - 1);
        return dynamic_list_push(&pf->instructions, &instr);
    }
    return false;
}

/**
 * @brief Convert token list to instructions using Shunting-Yard algorithm.
 */
static ParseStatus compile_to_rpn(ParsedFunction *pf, char *err_msg, size_t err_msg_len) {
    DynamicList op_stack;
    dynamic_list_init(&op_stack, sizeof(Token), 16);

    for (size_t i = 0; i < pf->tokens.count; i++) {
        Token *tok = (Token *)dynamic_list_get(&pf->tokens, i);
        if (!tok) continue;

        switch (tok->type) {
            case TOKEN_NUMBER: {
                Instruction instr = {
                    .opcode = OP_LOAD_NUM,
                    .number = tok->value
                };
                dynamic_list_push(&pf->instructions, &instr);
                break;
            }
            case TOKEN_CONSTANT: {
                Instruction instr = {
                    .opcode = OP_LOAD_NUM,
                    .number = tok->value
                };
                dynamic_list_push(&pf->instructions, &instr);
                break;
            }
            case TOKEN_VARIABLE: {
                int var_idx = parsed_function_find_var_index(pf, tok->name);
                Instruction instr;
                instr.opcode = OP_LOAD_VAR;
                instr.variable.index = var_idx;
                strncpy(instr.variable.name, tok->name, MAX_IDENTIFIER_LEN - 1);
                instr.variable.name[MAX_IDENTIFIER_LEN - 1] = '\0';
                dynamic_list_push(&pf->instructions, &instr);
                break;
            }
            case TOKEN_FUNCTION: {
                dynamic_list_push(&op_stack, tok);
                break;
            }
            case TOKEN_COMMA: {
                bool found_lparen = false;
                while (op_stack.count > 0) {
                    Token top;
                    dynamic_list_pop(&op_stack, &top);
                    if (top.type == TOKEN_LPAREN) {
                        // Put LPAREN back for subsequent arguments
                        dynamic_list_push(&op_stack, &top);
                        found_lparen = true;
                        break;
                    }
                    if (token_is_operator(top.type)) {
                        emit_operator_instruction(pf, top.type);
                    }
                }
                if (!found_lparen) {
                    if (err_msg && err_msg_len > 0) {
                        snprintf(err_msg, err_msg_len, "Comma outside function argument list at pos %zu", tok->position);
                    }
                    dynamic_list_free(&op_stack);
                    return PARSE_ERROR_MISMATCHED_PAREN;
                }
                break;
            }
            case TOKEN_OP_ADD:
            case TOKEN_OP_SUB:
            case TOKEN_OP_MUL:
            case TOKEN_OP_DIV:
            case TOKEN_OP_POW:
            case TOKEN_OP_NEG: {
                int p1 = token_operator_precedence(tok->type);
                bool right_assoc = token_is_right_associative(tok->type);

                while (op_stack.count > 0) {
                    Token *top = (Token *)dynamic_list_peek(&op_stack);
                    if (!top || top->type == TOKEN_LPAREN) {
                        break;
                    }

                    if (top->type == TOKEN_FUNCTION) {
                        Token func_tok;
                        dynamic_list_pop(&op_stack, &func_tok);
                        emit_function_instruction(pf, func_tok.name);
                        continue;
                    }

                    int p2 = token_operator_precedence(top->type);
                    if (p2 > p1 || (p2 == p1 && !right_assoc)) {
                        Token op_tok;
                        dynamic_list_pop(&op_stack, &op_tok);
                        emit_operator_instruction(pf, op_tok.type);
                    } else {
                        break;
                    }
                }
                dynamic_list_push(&op_stack, tok);
                break;
            }
            case TOKEN_LPAREN: {
                dynamic_list_push(&op_stack, tok);
                break;
            }
            case TOKEN_RPAREN: {
                bool found_lparen = false;
                while (op_stack.count > 0) {
                    Token top;
                    dynamic_list_pop(&op_stack, &top);
                    if (top.type == TOKEN_LPAREN) {
                        found_lparen = true;
                        break;
                    }
                    if (token_is_operator(top.type)) {
                        emit_operator_instruction(pf, top.type);
                    } else if (top.type == TOKEN_FUNCTION) {
                        emit_function_instruction(pf, top.name);
                    }
                }

                if (!found_lparen) {
                    if (err_msg && err_msg_len > 0) {
                        snprintf(err_msg, err_msg_len, "Mismatched closing parenthesis at pos %zu", tok->position);
                    }
                    dynamic_list_free(&op_stack);
                    return PARSE_ERROR_MISMATCHED_PAREN;
                }

                // If function call preceded this LPAREN, pop and emit it
                if (op_stack.count > 0) {
                    Token *top = (Token *)dynamic_list_peek(&op_stack);
                    if (top && top->type == TOKEN_FUNCTION) {
                        Token func_tok;
                        dynamic_list_pop(&op_stack, &func_tok);
                        emit_function_instruction(pf, func_tok.name);
                    }
                }
                break;
            }
            default:
                break;
        }
    }

    // Pop remaining operators
    while (op_stack.count > 0) {
        Token top;
        dynamic_list_pop(&op_stack, &top);
        if (top.type == TOKEN_LPAREN || top.type == TOKEN_RPAREN) {
            if (err_msg && err_msg_len > 0) {
                snprintf(err_msg, err_msg_len, "Mismatched parenthesis in expression");
            }
            dynamic_list_free(&op_stack);
            return PARSE_ERROR_MISMATCHED_PAREN;
        }
        if (token_is_operator(top.type)) {
            emit_operator_instruction(pf, top.type);
        } else if (top.type == TOKEN_FUNCTION) {
            emit_function_instruction(pf, top.name);
        }
    }

    dynamic_list_free(&op_stack);

    // Validate that we produced instructions
    if (pf->instructions.count == 0) {
        if (err_msg && err_msg_len > 0) {
            snprintf(err_msg, err_msg_len, "No instructions generated");
        }
        return PARSE_ERROR_MISSING_OPERAND;
    }

    return PARSE_SUCCESS;
}

ParseStatus parse_expression(const char *expression, ParsedFunction *pf, char *err_msg, size_t err_msg_len) {
    if (!expression || !pf) {
        if (err_msg && err_msg_len > 0) {
            snprintf(err_msg, err_msg_len, "Null pointer passed to parse_expression");
        }
        return PARSE_ERROR_EMPTY_INPUT;
    }

    parsed_function_init(pf);

    strncpy(pf->source, expression, sizeof(pf->source) - 1);
    pf->source[sizeof(pf->source) - 1] = '\0';

    ParseStatus status = tokenize_expression(expression, pf, err_msg, err_msg_len);
    if (status != PARSE_SUCCESS) {
        return status;
    }

    status = compile_to_rpn(pf, err_msg, err_msg_len);
    return status;
}

void parsed_function_print_tokens(const ParsedFunction *pf) {
    if (!pf) return;
    printf("=== TOKENS (%zu) ===\n", pf->tokens.count);
    for (size_t i = 0; i < pf->tokens.count; i++) {
        const Token *tok = (const Token *)dynamic_list_get(&pf->tokens, i);
        printf("[%02zu] ", i);
        token_print(tok);
        printf("\n");
    }
}

void parsed_function_print_variables(const ParsedFunction *pf) {
    if (!pf) return;
    printf("=== VARIABLES (%zu) ===\n", pf->variables.count);
    for (size_t i = 0; i < pf->variables.count; i++) {
        const Variable *var = (const Variable *)dynamic_list_get(&pf->variables, i);
        printf("Slot #%d: '%s'\n", var->index, var->name);
    }
}

void parsed_function_print_instructions(const ParsedFunction *pf) {
    if (!pf) return;
    printf("=== POSTFIX INSTRUCTIONS / OPERATIONS (%zu) ===\n", pf->instructions.count);
    for (size_t i = 0; i < pf->instructions.count; i++) {
        const Instruction *instr = (const Instruction *)dynamic_list_get(&pf->instructions, i);
        printf("[%02zu] ", i);
        switch (instr->opcode) {
            case OP_LOAD_NUM:
                printf("LOAD_NUM %g\n", instr->number);
                break;
            case OP_LOAD_VAR:
                printf("LOAD_VAR '%s' (slot %d)\n", instr->variable.name, instr->variable.index);
                break;
            case OP_ADD:
                printf("OP_ADD (+)\n");
                break;
            case OP_SUB:
                printf("OP_SUB (-)\n");
                break;
            case OP_MUL:
                printf("OP_MUL (*)\n");
                break;
            case OP_DIV:
                printf("OP_DIV (/)\n");
                break;
            case OP_POW:
                printf("OP_POW (^)\n");
                break;
            case OP_NEG:
                printf("OP_NEG (unary -)\n");
                break;
            case OP_FUNC_1ARG:
                printf("FUNC1 '%s'\n", instr->func1.name);
                break;
            case OP_FUNC_2ARG:
                printf("FUNC2 '%s'\n", instr->func2.name);
                break;
            default:
                printf("UNKNOWN_OPCODE %d\n", instr->opcode);
                break;
        }
    }
}

void parsed_function_dump(const ParsedFunction *pf) {
    if (!pf) return;
    printf("----------------------------------------\n");
    printf("Expression: \"%s\"\n", pf->source);
    parsed_function_print_tokens(pf);
    parsed_function_print_variables(pf);
    parsed_function_print_instructions(pf);
    printf("----------------------------------------\n");
}
