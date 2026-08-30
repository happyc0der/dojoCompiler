#ifndef LEXER_H
#define LEXER_H

typedef enum {
    TOK_INT,        /* keyword: int */
    TOK_VOID,       /* keyword: void */
    TOK_RETURN,     /* keyword: return */
    TOK_IDENT,      /* identifier, e.g. main, x, foo */
    TOK_NUMBER,     /* integer literal, e.g. 2, 42 */
    TOK_LPAREN,     /* ( */
    TOK_RPAREN,     /* ) */
    TOK_LBRACE,     /* { */
    TOK_RBRACE,     /* } */
    TOK_SEMICOLON,  /* ; */
    TOK_EOF,        /* end of input */
    TOK_UNKNOWN,     /* anything we can't lex -> caller should error */
    TOK_INVALID_NUMBER, /* digits followed by letters, e.g. 123abc */
    TOK_UNTERMINATED_COMMENT, /* block comment that was never closed */
    
} TokenType;

typedef struct {
    TokenType type;
    char *text;   /* the raw text of the token, heap-allocated */
    int line;     /* line number, for error messages */
} Token;

/* A Lexer walks over a null-terminated source string and hands out
 * one Token at a time via lexer_next(). It owns no memory except its
 * own bookkeeping; each Token.text is a fresh malloc the caller must
 * free with token_free(). */
typedef struct {
    const char *src;   /* full source text, not owned */
    size_t pos;        /* current index into src */
    size_t length;     /* strlen(src), cached */
    int line;           /* current line number, starts at 1 */
} Lexer;

Lexer lexer_init(const char *src);
Token lexer_next(Lexer *lex);
void token_free(Token *tok);
const char *token_type_name(TokenType type);

#endif