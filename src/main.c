#include <stdio.h>
#include <stdlib.h>
#include <ctype.h>
#include <assert.h>
#include <stdbool.h>

#define ARRAY_SIZE(arr) (sizeof(arr) / sizeof((arr)[0]))

#define MAX_NUMBER_OF_SYMBOLS 256
#define MAX_TOKEN_SIZE        256

// Table
#define BOOLEAN __int8_t

struct _SYMBOLS_TABLE {
    BOOLEAN symbols[MAX_NUMBER_OF_SYMBOLS];
    int length;
};

struct _SYMBOLS_TABLE TABLE;

BOOLEAN symbols_get(char *symbol);
void symbols_insert(char *symbol, BOOLEAN value);
void symbols_delete(char *symbol);

#define EMPTY_SLOT (-1)
void INIT_SYMBOLS_TABLE() {
    for (int i = 0; i < MAX_NUMBER_OF_SYMBOLS; i++) {
        TABLE.symbols[i] = EMPTY_SLOT;
    }
}

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
    assert(TABLE.symbols[i] == EMPTY_SLOT && "TODO: Collision not implemented");

    TABLE.symbols[i] = value;
    TABLE.length++;
}

void symbols_delete(char *symbol) {
    assert(TABLE.length > 0 && "Trying to delete a symbol in an empty table");
    int i = hash(symbol) % MAX_NUMBER_OF_SYMBOLS;
    TABLE.symbols[i] = EMPTY_SLOT;
    TABLE.length--;
}

void print_symbols_table() {
    for (int i = 0; i < MAX_NUMBER_OF_SYMBOLS; i++) {
        if (TABLE.symbols[i] == EMPTY_SLOT) {
            printf("[%03d] EMPTY_SLOT(%d)\n", i, TABLE.symbols[i]);
        } else {
            printf("[%03d] %d\n", i, TABLE.symbols[i]);
        }
    }
}

// end Table

// Lexer
char* read_file(const char* file_path);

typedef enum {
    TOKEN_PREP = 0    ,
    TOKEN_SIGOP       ,
    TOKEN_BINOP       ,
    TOKEN_OPPAREN     ,
    TOKEN_CLPAREN     ,
    _TOTAL_TOKEN_TYPES,
} TOKEN_TYPE;

char* TOKEN_TYPE_DESC[] = {
    [TOKEN_PREP]    = "TOKEN_PREP"   ,
    [TOKEN_SIGOP]   = "TOKEN_SIGOP"  ,
    [TOKEN_BINOP]   = "TOKEN_BINOP"  ,
    [TOKEN_OPPAREN] = "TOKEN_OPPAREN",
    [TOKEN_CLPAREN] = "TOKEN_CLPAREN",
};

typedef struct {
    char value[MAX_TOKEN_SIZE];
    TOKEN_TYPE type;
} Token;

_Static_assert(ARRAY_SIZE(TOKEN_TYPE_DESC) == _TOTAL_TOKEN_TYPES, "assert that you have implemented the description of all the tokens");

#define FMT_Token "Token(%s, %s)"
#define ARGS_Token(t) (t).value, TOKEN_TYPE_DESC[(t).type]

typedef struct {
    char *text;
    Token *token;
    int cursor;
} Lexer;

Lexer* lex_make(const char *code_path) {
    char *code = read_file(code_path);
    assert(code != NULL && "cannot read the code!");
    Lexer *lex = malloc(sizeof(Lexer));

    lex->cursor = 0;
    lex->text = code;
    lex->token = malloc(sizeof(Token));
    return lex;
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
    static char chars_that_are_single_tokens[] = {
        '^',
        'v',
        '~',
        '(',
        ')',
        '+'
    };

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
            symbols_insert(lex->token->value, 0);
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
            default: {
                assert(false && "unreachable");
            }
        }
    }

    // TODO: add token for => and <=>
    if (c == '-' || c == '<') {
        char first_char = c;
        c = lex_nextc(lex);
        if (c == '>' || c == '-') {
            lex->token->type = TOKEN_BINOP;
            lex->token->value[0] = first_char;
            lex->token->value[1] = c;
            lex->token->value[2] = '\0';
            lex_nextc(lex);
            return 0;
        }
    }

    assert(false && "unreachable");
}

// end Lexer

int main(void) {
    INIT_SYMBOLS_TABLE();
    const char *file_path = "teste.lc";
    Lexer *lex = lex_make(file_path);
    while (lex_nextt(lex) != -1) {
        printf(FMT_Token"\n", ARGS_Token(*lex->token));
    }

    lex_free(lex);
    print_symbols_table();
    return 0;
}

char* read_file(const char* file_path) {
    FILE *f = fopen(file_path, "r");
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

    char *content = malloc(sizeof(char)*size);
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