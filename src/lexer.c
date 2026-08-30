#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <ctype.h>
#include "lexer.h"

/* Portable replacement for strdup: strdup is POSIX, not standard C,
 * and behaves inconsistently across -std=c11 strictness levels on
 * different compilers/platforms. Rolling our own avoids that. */
static char *dup_string(const char *s) {
    size_t len = strlen(s) + 1;
    char *copy = malloc(len);
    if (copy) memcpy(copy, s, len);
    return copy;
}

typedef struct {
    char *data;
    size_t len;
    size_t cap;
} GrowBuf;

static void growbuf_init(GrowBuf *b) {
    b->cap = 32;
    b->data = malloc(b->cap);
    b->len = 0;
}

static void growbuf_push(GrowBuf *b, char c) {
    if (b->len + 1 >= b->cap) {
        b->cap *= 2;
        b->data = realloc(b->data, b->cap);
    }
    b->data[b->len++] = c;
}

static char *growbuf_finish(GrowBuf *b) {
    b->data[b->len] = '\0';
    return b->data;
}
/* --- construction --- */

Lexer lexer_init(const char *src) {
    Lexer lex;
    lex.src = src;
    lex.pos = 0;
    lex.length = strlen(src);
    lex.line = 1;
    return lex;
}

/* --- small helpers --- */

static char peek(Lexer *lex) {
    if (lex->pos >= lex->length) return '\0';
    return lex->src[lex->pos];
}

static char peek_at(Lexer *lex, size_t offset) {
    if (lex->pos + offset >= lex->length) return '\0';
    return lex->src[lex->pos + offset];
}

static char advance(Lexer *lex) {
    char c = lex->src[lex->pos];
    lex->pos++;
    if (c == '\n') lex->line++;
    return c;
}

static int skip_block_comment(Lexer *lex) {
    advance(lex);
    advance(lex);
    while (1){
        if (peek(lex)=='\0') return 1;
        if (peek(lex)=='*' && peek_at(lex, 1)=='/') {
            advance(lex);
            advance(lex);
            return 0;
        }
        advance(lex);
    }
}

/* Skip spaces, tabs, newlines, and // line comments so the real
 * tokenizing logic below never has to think about whitespace. */
static int skip_whitespace_and_comments(Lexer *lex) {
    for (;;) {
        char c = peek(lex);
        if (c == ' ' || c == '\t' || c == '\n' || c == '\r') {
            advance(lex);
        } else if (c == '/' && peek_at(lex, 1) == '/') {
            /* line comment: consume until newline or EOF */
            while (peek(lex) != '\n' && peek(lex) != '\0') advance(lex);
        } else if (c == '/' && peek_at(lex, 1) == '*') {
            /* block comment: consume until closing */
            if (skip_block_comment(lex)) {
                /* unterminated block comment */
                return 1;
            }
        } else {
            break;
        }
    }
    return 0;
}

static Token make_token(TokenType type, const char *text, int line) {
    Token tok;
    tok.type = type;
    tok.text = dup_string(text);
    tok.line = line;
    return tok;
}

/* Turn an already-scanned identifier-like string into either a
 * keyword token (int/void/return) or a generic TOK_IDENT. */
static TokenType classify_word(const char *word) {
    if (strcmp(word, "int") == 0) return TOK_INT;
    if (strcmp(word, "void") == 0) return TOK_VOID;
    if (strcmp(word, "return") == 0) return TOK_RETURN;
    return TOK_IDENT;
}

/* --- the main entry point --- */

/* lexer_next scans exactly one token starting at the lexer's current
 * position and advances past it. Call it repeatedly until it returns
 * TOK_EOF. This is the classic "pull" style lexer: the parser will
 * later call this on demand rather than the lexer producing a list
 * up front. */
Token lexer_next(Lexer *lex) {
    int unterminated = skip_whitespace_and_comments(lex);
    int line = lex->line;

    if (unterminated){
        return make_token(TOK_UNTERMINATED_COMMENT, "unterminated comment", line);
    }

    char c = peek(lex);

    if (c == '\0') {
        return make_token(TOK_EOF, "", line);
    }

    /* single-character punctuation */
    switch (c) {
        case '(': advance(lex); return make_token(TOK_LPAREN, "(", line);
        case ')': advance(lex); return make_token(TOK_RPAREN, ")", line);
        case '{': advance(lex); return make_token(TOK_LBRACE, "{", line);
        case '}': advance(lex); return make_token(TOK_RBRACE, "}", line);
        case ';': advance(lex); return make_token(TOK_SEMICOLON, ";", line);
    }

    /* identifiers and keywords: [A-Za-z_][A-Za-z0-9_]* */
    if (isalpha((unsigned char)c) || c == '_') {
        GrowBuf buf;
        growbuf_init(&buf);
        while (isalnum((unsigned char)peek(lex)) || peek(lex) == '_') {
            growbuf_push(&buf, advance(lex));
        }
        char *word = growbuf_finish(&buf);
        Token tok = make_token(classify_word(word), word, line);
        free(word);
        return tok;
    }

    /* integer literals: [0-9]+
     * If a number is immediately followed by letters or underscores,
     * treat the whole sequence as an invalid number token.
     * For example, "123abc" will be a single TOK_INVALID_NUMBER token
     * instead of being split into a number and an identifier.
     */

    if (isdigit((unsigned char)c)) {
        GrowBuf buf;
        growbuf_init(&buf);
        while (isdigit((unsigned char)peek(lex))) {
            growbuf_push(&buf, advance(lex));
        }
        if (isalpha((unsigned char)peek(lex)) || peek(lex) == '_') {
            while (isalnum((unsigned char)peek(lex)) || peek(lex) == '_') {
                growbuf_push(&buf, advance(lex));
            }
            char *word = growbuf_finish(&buf);
            Token tok = make_token(TOK_INVALID_NUMBER, word, line);
            free(word);
            return tok;
        }
        char *word = growbuf_finish(&buf);
        Token tok = make_token(TOK_NUMBER, word, line);
        free(word);
        return tok;
    }

    /* anything else is not part of our tiny language yet */
    char unknown[2] = { c, '\0' };
    advance(lex);
    return make_token(TOK_UNKNOWN, unknown, line);
}

void token_free(Token *tok) {
    free(tok->text);
    tok->text = NULL;
}

const char *token_type_name(TokenType type) {
    switch (type) {
        case TOK_INT: return "INT";
        case TOK_VOID: return "VOID";
        case TOK_RETURN: return "RETURN";
        case TOK_IDENT: return "IDENT";
        case TOK_NUMBER: return "NUMBER";
        case TOK_LPAREN: return "LPAREN";
        case TOK_RPAREN: return "RPAREN";
        case TOK_LBRACE: return "LBRACE";
        case TOK_RBRACE: return "RBRACE";
        case TOK_SEMICOLON: return "SEMICOLON";
        case TOK_EOF: return "EOF";
        case TOK_INVALID_NUMBER: return "INVALID_NUMBER";
        case TOK_UNTERMINATED_COMMENT: return "UNTERMINATED_COMMENT";
        default: return "UNKNOWN";
    }
}