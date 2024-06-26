#include <stdio.h>
#include <stdlib.h>
#include <ctype.h>
#include <assert.h>
#include <stdbool.h>
#include <string.h>

#define ARRAY_SIZE(arr) (sizeof(arr) / sizeof((arr)[0]))

char* read_file(const char* file_path);
int powi(int b, int p);
int getline(char *dest, int size, FILE *stream);

#define MAX_NUMBER_OF_SYMBOLS 256
#define MAX_TOKEN_SIZE        256

#define UNREACHABLE assert(0 && "UNREACHABLE")

// Table
#define BOOLEAN __int8_t

struct _SYMBOLS_TABLE {
    BOOLEAN symbols[MAX_NUMBER_OF_SYMBOLS];
    char keys[MAX_TOKEN_SIZE][MAX_NUMBER_OF_SYMBOLS];
    int length;
};

struct _SYMBOLS_TABLE TABLE;

BOOLEAN symbols_get(char *symbol);
void symbols_insert(char *symbol, BOOLEAN value);
void symbols_delete(char *symbol);
void symbols_reset();

#define EMPTY_SLOT (-1)

long int hash(char *s) {
    int cnt = 0, p = 7877;
    long int h = 0;
    while (*s != '\0' && cnt < MAX_TOKEN_SIZE) h = p*h + *(s++);
    return h;
}

BOOLEAN symbols_get(char *symbol) {
    return TABLE.symbols[hash(symbol) % MAX_NUMBER_OF_SYMBOLS];
}

void symbols_insert(char *symbol, BOOLEAN value) {
    assert(TABLE.length < MAX_NUMBER_OF_SYMBOLS && "Symbols table exceds maximum number os entries");
    assert(value != EMPTY_SLOT && "Cannot use EMPTY_SLOT has a value");

    int i = hash(symbol) % MAX_NUMBER_OF_SYMBOLS;

    if (TABLE.symbols[i] == EMPTY_SLOT) {
        strncpy(TABLE.keys[TABLE.length++], symbol, MAX_TOKEN_SIZE);
    }

    TABLE.symbols[i] = value;
}

void symbols_delete(char *symbol) {
    assert(TABLE.length > 0 && "Trying to delete a symbol in an empty table");
    int i = hash(symbol) % MAX_NUMBER_OF_SYMBOLS;
    TABLE.symbols[i] = EMPTY_SLOT;
    TABLE.length--;
}

void symbols_reset() {
    int total_symbols = TABLE.length;
    for (int k = 0; k < total_symbols; k++) {
        symbols_delete(TABLE.keys[k]);
    }
}

void INIT_SYMBOLS_TABLE() {
    for (int i = 0; i < MAX_NUMBER_OF_SYMBOLS; i++) {
        TABLE.symbols[i] = EMPTY_SLOT;
    }
}

void print_symbols_table() {
    for (int i = 0; i < TABLE.length; i++) {
        printf("[%s] %d ", TABLE.keys[i], symbols_get(TABLE.keys[i]));
    } printf("\n");
}

// end Table

// Lexer
typedef enum {
    TOKEN_PREP = 0    ,
    TOKEN_SIGOP       ,
    TOKEN_BINOP       ,
    TOKEN_OPPAREN     ,
    TOKEN_CLPAREN     ,
    TOKEN_FINAL_OP    ,
    _TOTAL_TOKEN_TYPES,
} TOKEN_TYPE;

char* TOKEN_TYPE_DESC[] = {
    [TOKEN_PREP]     = "TOKEN_PREP"       ,
    [TOKEN_SIGOP]    = "TOKEN_SIGOP"      ,
    [TOKEN_BINOP]    = "TOKEN_BINOP"      ,
    [TOKEN_OPPAREN]  = "TOKEN_OPPAREN"    ,
    [TOKEN_CLPAREN]  = "TOKEN_CLPAREN"    ,
    [TOKEN_FINAL_OP] = "TOKEN_FINAL_OP",
};

typedef struct {
    char value[MAX_TOKEN_SIZE];
    TOKEN_TYPE type;
} Token;

_Static_assert(ARRAY_SIZE(TOKEN_TYPE_DESC) == _TOTAL_TOKEN_TYPES, "assert that you have implemented the description of all the tokens");

#define FMT_TOKEN "Token( %s, %s )"
#define ARGS_TOKEN(t) (t).value, TOKEN_TYPE_DESC[(t).type]

typedef struct {
    char *text;
    Token *token;
    int cursor;
} Lexer;

Lexer* lex_make(char *code) {
    assert(code != NULL && "cannot read the code!");
    Lexer *lex = malloc(sizeof(Lexer));

    lex->cursor = 0;
    lex->text = code;
    lex->token = malloc(sizeof(Token));
    return lex;
}

void lex_reset(Lexer *lex) {
    lex->cursor = 0;
}

void lex_free(Lexer *lex) {
    if (lex == NULL) return;

    free(lex->text);
    free(lex->token);
    free(lex);
}

char lex_currc(Lexer *lex) {
    char c = lex->text[lex->cursor];
    if (c == '\0') {
        return -1;
    }

    return c;
}

char lex_nextc(Lexer *lex) {
    if (lex_currc(lex) == -1) {
        return -1;
    }

    lex->cursor = lex->cursor + 1;
    return lex_currc(lex);
}

bool token_is_single_char(char c) {
    static char chars_that_are_single_tokens[] = { '^', 'v', '~', '(', ')', '+' };

    for (size_t i = 0; i < ARRAY_SIZE(chars_that_are_single_tokens); i++) {
        if (c == chars_that_are_single_tokens[i]) {
            return true;
        }
    }

    return false;
}

int lex_nextt(Lexer *lex) {
    char c = lex_currc(lex);
    while (c != -1 && isspace(c)) {
        c = lex_nextc(lex);
    }

    if (c == -1) {
        return -1;
    }

    int i = 0;
    if (isalpha(c) && c != 'v') {
        while (c != -1 && isalpha(c)) {
            assert(i < MAX_TOKEN_SIZE-1 && "ERROR: MAX_TOKEN_SIZE");
            lex->token->value[i++] = c;
            c = lex_nextc(lex);
        }

        lex->token->value[i] = '\0';
        lex->token->type = TOKEN_PREP;
        BOOLEAN s = symbols_get(lex->token->value);
        if (s == EMPTY_SLOT) {
            symbols_insert(lex->token->value, 1);
        }

        return 0;
    }

    if (token_is_single_char(c)) {
        lex->token->value[0] = c;
        lex->token->value[1] = '\0';
        lex_nextc(lex);
        switch (c) {
            case 'v': /* OR  */
            case '+': /* XOR */
            case '^': /* AND */
                      lex->token->type = TOKEN_BINOP;   return 0;

            case '~': lex->token->type = TOKEN_SIGOP;   return 0;
            case '(': lex->token->type = TOKEN_OPPAREN; return 0;
            case ')': lex->token->type = TOKEN_CLPAREN; return 0;
            default: { UNREACHABLE; }
        }
    }

    // TODO: add token for => and <=>
    if (c == '<') {
        if ((c = lex_nextc(lex)) == '=') {
            assert(lex_nextc(lex) == '>');
            lex->token->value[0] = '<';
            lex->token->value[1] = '=';
            lex->token->value[2] = '>';
            lex->token->type = TOKEN_FINAL_OP;
        } else if (c == '-') {
            assert(lex_nextc(lex) == '>');
            lex->token->value[0] = '<';
            lex->token->value[1] = '-';
            lex->token->value[2] = '>';
            lex->token->type = TOKEN_BINOP;
        }

        lex_nextc(lex);
        lex->token->value[3] = '\0';
        return 0;
    }

    if (c == '-') {
        assert(lex_nextc(lex) == '>');
        lex->token->type = TOKEN_BINOP;
        lex->token->value[0] = '-';
        lex->token->value[1] = '>';
        lex->token->value[2] = '\0';
        lex_nextc(lex);
        return 0;
    }

    UNREACHABLE;
}

// end Lexer

// Stack

#define MAX_STACK_SIZE 4096

typedef struct {
    BOOLEAN items[MAX_STACK_SIZE];
    int head;
} Stack;

BOOLEAN pop(Stack *s) {
    assert(s->head > 0 && "pop(Stack *s) on empty stack");
    return s->items[--s->head];
}

void push(Stack *s, BOOLEAN item) {
    assert(s->head < MAX_STACK_SIZE && "push(Stack *s, BOOLEAN item) on fullstack");
    s->items[s->head++] = item;
}

void print_stack_trace(Stack *s) {
    for (int i = s->head-1; i >= 0; i--) {
        printf("[ %d ] %d\n", i, s->items[i]);
    }
}

// end Stack

BOOLEAN NOT(BOOLEAN value) {
    return !value;
}

BOOLEAN OR(BOOLEAN value, BOOLEAN value2) {
    return value | value2;
}

BOOLEAN AND(BOOLEAN value, BOOLEAN value2) {
    return value & value2;
}

BOOLEAN XOR(BOOLEAN value, BOOLEAN value2) {
    return value ^ value2;
}

BOOLEAN COND(BOOLEAN value, BOOLEAN value2) {
    return (!value) | value2;
}

BOOLEAN BCOND(BOOLEAN value, BOOLEAN value2) {
    return !(value ^ value2);
}

typedef BOOLEAN (*BINOP_FUNC)(BOOLEAN, BOOLEAN);

BINOP_FUNC get_binop_operation(char *operation_symbol) {
    if (strncmp(operation_symbol, "v", 1) == 0) {
        return &OR;
    } else if (strncmp(operation_symbol, "^", 1) == 0) {
        return &AND;
    } else if (strncmp(operation_symbol, "+", 1) == 0) {
        return &XOR;
    } else if (strncmp(operation_symbol, "->", 2) == 0) {
        return &COND;
    } else if (strncmp(operation_symbol, "<->", 3) == 0) {
        return &BCOND;
    } else {
        assert(0 && "TODO: not implemented");
    }
}

int evaluate(Lexer *lex, Stack *stack, int initial_parenteses_open_count) {
    BOOLEAN v, v2;
    int parens = initial_parenteses_open_count;
    while ((initial_parenteses_open_count == 0 || parens > (initial_parenteses_open_count-1)) && lex_nextt(lex) != -1) {
        assert(parens >= 0 && "Unbalanced parens");
        switch (lex->token->type) {
            case TOKEN_OPPAREN: parens++; break;
            case TOKEN_CLPAREN: parens--; break;

            case TOKEN_PREP: {
                v = symbols_get(lex->token->value);
                assert(v != EMPTY_SLOT && "token not present in the table of symbols");
                push(stack, v);
            } break;

            case TOKEN_SIGOP: {
                assert(strncmp(lex->token->value, "~", 1) == 0 && "NOT IMPLEMENTED: we have just 'not' operation for now");
                assert(lex_nextt(lex) != -1 && "Expected TOKEN_PREP get EOF");
                if (lex->token->type == TOKEN_PREP) {
                    v = symbols_get(lex->token->value);
                    push(stack, NOT(v));

                } else if (lex->token->type == TOKEN_OPPAREN) {
                    if (evaluate(lex, stack, parens + 1) == TOKEN_FINAL_OP) {
                        assert(0 && "using a final operation to compose expressions");
                    }

                    v = pop(stack);
                    push(stack, NOT(v));

                } else {
                    printf(FMT_TOKEN"\n", ARGS_TOKEN(*lex->token));
                    UNREACHABLE;
                }
            } break;

            case TOKEN_BINOP: {
                BINOP_FUNC operation = get_binop_operation(lex->token->value);
                assert(lex_nextt(lex) != -1 && "Expected TOKEN_PREP get EOF");

                v = pop(stack);
                if (lex->token->type == TOKEN_PREP) {
                    v2 = symbols_get(lex->token->value);
                    push(stack, (*operation)(v, v2));

                } else if (lex->token->type == TOKEN_SIGOP) {
                    assert(strncmp(lex->token->value, "~", 1) == 0 && "NOT IMPLEMENTED: we have just 'not' operation for now");
                    assert(lex_nextt(lex) != -1 && "Expected TOKEN_PREP get EOF");

                    if (lex->token->type == TOKEN_PREP) {
                        push(stack, NOT(symbols_get(lex->token->value)));
                    } else if (lex->token->type == TOKEN_OPPAREN) {
                        if (evaluate(lex, stack, parens + 1) == TOKEN_FINAL_OP) {
                            assert(0 && "using a final operation to compose expressions");
                        }

                        push(stack, NOT(pop(stack)));
                    } else {
                        UNREACHABLE;
                    }

                    v2 = pop(stack);
                    push(stack, (*operation)(v, v2));

                } else if (lex->token->type == TOKEN_OPPAREN) {
                    evaluate(lex, stack, parens + 1);
                    v2 = pop(stack);
                    push(stack, (*operation)(v, v2));

                } else {
                    printf(FMT_TOKEN"\n", ARGS_TOKEN(*lex->token));
                    UNREACHABLE;
                }

            } break;

            case TOKEN_FINAL_OP: {
                Stack stack2 = {0};
                evaluate(lex, &stack2, 0);
                BOOLEAN ha = 1;
                if (stack->head != stack2.head) {
                    ha = 0;
                } else {
                    while(stack->head > 0) {
                        BOOLEAN v1 = pop(stack);
                        BOOLEAN v2 = pop(&stack2);
                        if (v1 != v2) {
                            ha = 0;
                            break;
                        }
                    }
                }

                push(stack, ha);
                return TOKEN_FINAL_OP;
            } break;

            default: {
                UNREACHABLE;
            }
        }
    }

    return 0;
}

/*
| (p -> q) <=> p |
|       1        |

| A | B | COLRE |
| 1 | 1 |   1   |
| 1 | 0 |   0   |
| 0 | 1 |   0   |
| 0 | 0 |   0   |
*/
#define PREFIX        "EXPR"
#define PREFIX_RESULT "CLRES"
void generate_truth_table(Lexer *lex) {
    Stack stack = {0};
    if (evaluate(lex, &stack, 0) == TOKEN_FINAL_OP) {
        int pad = strlen(lex->text);
        printf("| %s |\n", lex->text);
        printf("| %*s%d%*s|\n", pad/2, "", pop(&stack), pad/2 + pad%2, "");
    } else {
        printf("%s => %s\n", PREFIX, lex->text);
        for(int k = 0; k < TABLE.length; k++) {
            printf("| %s ", TABLE.keys[k]);
        }

        printf("| %s |", PREFIX_RESULT);

        printf("\n");
        for(int k = 0; k < TABLE.length; k++) {
            printf("| - ");
        }

        printf("|   -   |");

        printf("\n");

        int len;
        for(int k = 0; k < TABLE.length; k++) {
            BOOLEAN v = symbols_get(TABLE.keys[k]);
            len = strlen(TABLE.keys[k]);
            printf("| %*s%c%d ", len, "", '\b', v);
        }

        len = strlen(PREFIX_RESULT);
        printf("| %*s%c%d |\n", len, "", '\b', pop(&stack));

        for (int j = powi(2, TABLE.length)-2; j >= 0; j--) {
            for(int k = 0; k < TABLE.length; k++) {
                BOOLEAN v = symbols_get(TABLE.keys[k])&j>>(TABLE.length-k-1);
                symbols_insert(TABLE.keys[k], v);
                len = strlen(TABLE.keys[k]);
                printf("| %*s%c%d ", len, "", '\b', v);
            }

            lex_reset(lex);
            evaluate(lex, &stack, 0);

            len = strlen(PREFIX_RESULT);
            printf("| %*s%c%d |\n", len, "", '\b', pop(&stack));
            for (int k = 0; k < TABLE.length; k++) {
                symbols_insert(TABLE.keys[k], 1);
            }
        }
    }
}

char* shift(int* argc, char*** argv) {
    --(*argc);
    return *(*argv)++;
}

void usage(const char *program_name) {
    const char *usage_string =
    "Usage: %s [OPTIONS] EXP [FILE]\n"
    "  -f\tUse expression from file\n"
    "  -i\tUse interactive mode\n";
    printf(usage_string, program_name);
}

void interative_mode() {
    Lexer *lex = NULL;
    const int buffer_size = 4096;
    char buffer[buffer_size];

    while (getline(buffer, buffer_size, stdin) != EOF) {
        if (lex == NULL) {
            lex = lex_make(buffer);
        } else {
            lex_reset(lex);
            lex->text = buffer;
        }

        generate_truth_table(lex);
        symbols_reset();
    }

    lex_free(lex);
}

char* read_file(const char* file_path) {
    FILE *f = fopen(file_path, "r");
    char *content = NULL;
    if (f == NULL) {
        goto ERROR;
    }
    if (fseek(f, 0, SEEK_END) != 0) {
        goto ERROR;
    }

    int size = ftell(f);
    if (size < 0) {
        goto ERROR;
    }
    rewind(f);

    content = malloc(sizeof(char)*size);
    int b_read = fread(content, 1, size, f);
    if (b_read != size) {
        goto ERROR;
    }

    return content;

ERROR:
    if (f) fclose(f);
    if (content) free(content);
    return NULL;
}

int powi(int b, int p) {
    int r = 1;
    while (p-- > 0) r *= b;
    return r;
}

int getline(char *dest, int size, FILE *stream) {
    char c;
    int cnt = 0;
    printf("\r%s> ", PREFIX);
    c = fgetc(stream);
    if (c == EOF) return EOF;
    while(c != EOF && c != '\n') {
        if ((size-1) <= cnt ) break;
        *(dest++) = c;
        cnt++;
        c = fgetc(stream);
    }

    *(dest++) = '\0';
    return 0;
}

// TODO: add proper syntax error (lookahead can be useful)
// TODO: add proper error from command line mistakes
// TODO: add piping mechanism
int main(int argc, char **argv) {
    INIT_SYMBOLS_TABLE();

    char *program_name = shift(&argc, &argv);
    if (argc == 0) {
        usage(program_name);
        return 0;
    }

    char *arg = shift(&argc, &argv);
    if (strncmp(arg, "-i", 2) == 0) {
        interative_mode();
        return 0;
    } else if (strncmp(arg, "-f", 2) == 0) {
        char *file_path = shift(&argc, &argv);
        char *code = read_file(file_path);
        Lexer *lex = lex_make(code);
        generate_truth_table(lex);
    } else {
        Lexer *lex = lex_make(arg);
        generate_truth_table(lex);
    }

    return 0;
}
