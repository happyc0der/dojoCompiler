#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include "../src/lexer.h"

static int tests_run = 0;
static int tests_failed = 0;

#define CHECK(cond, desc) do { \
    tests_run++; \
    if (!(cond)) { tests_failed++; printf("  FAIL: %s (line %d)\n", desc, __LINE__); } \
    else { printf("  PASS: %s\n", desc); } \
} while (0)

static void expect_token_types(const char *src, TokenType *expected, int count, const char *label) {
    printf("Test: %s\n", label);
    Lexer lex = lexer_init(src);
    int i = 0;
    while (1) {
        Token tok = lexer_next(&lex);
        if (i < count) {
            char desc[160];
            snprintf(desc, sizeof(desc), "token %d is %s (got %s, text=\"%s\")",
                     i, token_type_name(expected[i]), token_type_name(tok.type), tok.text);
            CHECK(tok.type == expected[i], desc);
        } else {
            tests_run++; tests_failed++;
            printf("  FAIL: more tokens than expected, got extra %s (\"%s\")\n",
                   token_type_name(tok.type), tok.text);
        }
        i++;
        int done = (tok.type == TOK_EOF);
        token_free(&tok);
        if (done) break;
    }
    if (i < count) {
        tests_run++; tests_failed++;
        printf("  FAIL: expected %d tokens but lexer produced only %d\n", count, i);
    }
    printf("\n");
}

int main(void) {
    /* --- original regression suite --- */
    {
        TokenType expected[] = { TOK_INT, TOK_IDENT, TOK_LPAREN, TOK_VOID, TOK_RPAREN,
            TOK_LBRACE, TOK_RETURN, TOK_NUMBER, TOK_SEMICOLON, TOK_RBRACE, TOK_EOF };
        expect_token_types("int main(void) { return 2; }", expected, 11, "minimal return-2 program");
    }
    {
        TokenType expected[] = { TOK_IDENT, TOK_IDENT, TOK_IDENT, TOK_EOF };
        expect_token_types("returnValue voidType intish", expected, 4, "keyword-lookalikes are identifiers");
    }
    {
        TokenType expected[] = { TOK_RETURN, TOK_NUMBER, TOK_SEMICOLON, TOK_EOF };
        expect_token_types("  return   // this is a comment\n  0 ;  ", expected, 4, "line comments still skipped");
    }
    {
        TokenType expected[] = { TOK_UNKNOWN, TOK_EOF };
        expect_token_types("@", expected, 2, "unknown character yields TOK_UNKNOWN");
    }

    /* --- fixes for previously-found bugs --- */

    /* 1. block comments now supported */
    {
        TokenType expected[] = { TOK_INT, TOK_IDENT, TOK_SEMICOLON, TOK_EOF };
        expect_token_types("/* hello */ int x;", expected, 4, "FIXED: block comments are skipped");
    }
    {
        TokenType expected[] = { TOK_INT, TOK_IDENT, TOK_SEMICOLON, TOK_EOF };
        expect_token_types("int /* multi\nline\ncomment */ x;", expected, 4, "FIXED: multi-line block comment tracks lines");
    }
    {
        TokenType expected[] = { TOK_UNTERMINATED_COMMENT, TOK_EOF };
        expect_token_types("/* never closed", expected, 2, "NEW: unterminated block comment reported as ONE clear token, not silently split into UNKNOWNs, then EOF");
    }

    /* 2. "123abc" is now one invalid token, not two valid ones */
    {
        TokenType expected[] = { TOK_INVALID_NUMBER, TOK_EOF };
        expect_token_types("123abc", expected, 2, "FIXED: digits-then-letters is one INVALID_NUMBER token");
    }
    {
        TokenType expected[] = { TOK_NUMBER, TOK_SEMICOLON, TOK_EOF };
        expect_token_types("123;", expected, 3, "plain number still works fine");
    }

    /* 3. long identifiers (>255 chars) no longer truncate/split */
    {
        char big[600];
        for (int i = 0; i < 500; i++) big[i] = 'a';
        big[500] = '\0';
        Lexer lex = lexer_init(big);
        Token t = lexer_next(&lex);
        tests_run++;
        if (t.type == TOK_IDENT && strlen(t.text) == 500) {
            printf("Test: FIXED: 500-char identifier stays one token, full length preserved\n  PASS: got IDENT of length %zu\n\n", strlen(t.text));
        } else {
            tests_failed++;
            printf("Test: FIXED: 500-char identifier stays one token\n  FAIL: got %s of length %zu\n\n", token_type_name(t.type), strlen(t.text));
        }
        Token t2 = lexer_next(&lex);
        tests_run++;
        if (t2.type == TOK_EOF) {
            printf("  PASS: no spurious second token after the long identifier\n\n");
        } else {
            tests_failed++;
            printf("  FAIL: found a spurious extra token %s (\"%s\")\n\n", token_type_name(t2.type), t2.text);
        }
        token_free(&t); token_free(&t2);
    }

    /* 4. long numbers (>255 digits) no longer truncate/split */
    {
        char bignum[400];
        for (int i = 0; i < 300; i++) bignum[i] = '9';
        bignum[300] = '\0';
        Lexer lex = lexer_init(bignum);
        Token t = lexer_next(&lex);
        tests_run++;
        if (t.type == TOK_NUMBER && strlen(t.text) == 300) {
            printf("Test: FIXED: 300-digit number stays one token\n  PASS: got NUMBER of length %zu\n\n", strlen(t.text));
        } else {
            tests_failed++;
            printf("Test: FIXED: 300-digit number stays one token\n  FAIL: got %s of length %zu\n\n", token_type_name(t.type), strlen(t.text));
        }
        token_free(&t);
    }

    printf("=====================================\n");
    printf("Ran %d checks, %d failed\n", tests_run, tests_failed);
    return tests_failed == 0 ? 0 : 1;
}
