/****************************************************/
/* File: scan.c                                     */
/* The scanner implementation for the TINY compiler */
/* Compiler Construction: Principles and Practice   */
/* Kenneth C. Louden                                */
/****************************************************/

// #include "globals.h"
#include "scan.h"

#include "util.h"

/* states in scanner DFA */
// 2023/4/2:
// typedef enum { START, INASSIGN, INCOMMENT, INNUM, INID, DONE } StateType;
typedef enum {
    START,
    INASSIGN,
    INCOMMENT,
    INNUM,
    INID,
    DONE,
    // 多行注释状态
    S1,
    S2,
    S3,
    // 浮点数状态
    F1,
    F2,
    F3,
    F4,
    F5
} StateType;

/* lexeme of identifier or reserved word */
char tokenString[MAXTOKENLEN + 1];

/* BUFLEN = length of the input buffer for
   source code lines */
#define BUFLEN 256

static char lineBuf[BUFLEN]; /* holds the current line */
static int linepos = 0;      /* current position in LineBuf */
static int bufsize = 0;      /* current size of buffer string */
static int EOF_flag = FALSE; /* corrects ungetNextChar behavior on EOF */

/* getNextChar fetches the next non-blank character
   from lineBuf, reading in a new line if lineBuf is
   exhausted */
static int getNextChar(void) {
    if (!(linepos < bufsize)) {
        lineno++;
        if (fgets(lineBuf, BUFLEN - 1, source)) {
            if (EchoSource) fprintf(listing, "%4d: %s", lineno, lineBuf);
            bufsize = strlen(lineBuf);
            linepos = 0;
            return lineBuf[linepos++];
        } else {
            EOF_flag = TRUE;
            return EOF;
        }
    } else
        return lineBuf[linepos++];
}

/* ungetNextChar backtracks one character
   in lineBuf */
static void ungetNextChar(void) {
    if (!EOF_flag) linepos--;
}

/* lookup table of reserved words */
static struct {
    char* str;
    TokenType tok;
} reservedWords[MAXRESERVED] = {{"if", IF},         {"then", THEN},
                                {"else", ELSE},     {"end", END},
                                {"repeat", REPEAT}, {"until", UNTIL},
                                {"read", READ},     {"write", WRITE},
                                {"int", INT},       {"function", FUNCTION},
                                {"while", WHILE},   {"do", DO},
                                {"return", RETURN}};

/* lookup an identifier to see if it is a reserved word */
/* uses linear search */
static TokenType reservedLookup(char* s) {
    int i;
    for (i = 0; i < MAXRESERVED; i++)
        if (!strcmp(s, reservedWords[i].str)) return reservedWords[i].tok;
    return ID;
}

/****************************************/
/* the primary function of the scanner  */
/****************************************/
/* function getToken returns the
 * next token in source file
 */
TokenType getToken(void) {
    /* index for storing into tokenString */
    int tokenStringIndex = 0;
    /* holds current token to be returned */
    TokenType currentToken;
    /* current state - always begins at START */
    StateType state = START;
    /* flag to indicate save to tokenString */
    int save;
    while (state != DONE) {
        int c = getNextChar();
        save = TRUE;
        switch (state) {
            case START:
                if (isdigit(c)) {
                    state = INNUM;
                } else if (isalpha(c)) {
                    state = INID;
                } else if (c == ':') {
                    state = INASSIGN;
                } else if ((c == ' ') || (c == '\t') || (c == '\n')) {
                    save = FALSE;
                } else if (c == '{') {
                    save = FALSE;
                    state = INCOMMENT;
                } else if (c == '/') {
                    // 多行注释/**/开始
                    // 同时也是除号
                    save = FALSE;
                    state = S1;
                } else {
                    state = DONE;
                    switch (c) {
                        case EOF:
                            save = FALSE;
                            currentToken = ENDFILE;
                            break;
                        case '=':
                            currentToken = EQ;
                            break;
                        case '<':
                            currentToken = LT;
                            break;
                        case '+':
                            currentToken = PLUS;
                            break;
                        case '-':
                            currentToken = MINUS;
                            break;
                        case '*':
                            currentToken = TIMES;
                            break;
                        // case '/':
                        //     currentToken = OVER;
                        //     break;
                        case '(':
                            currentToken = LPAREN;
                            break;
                        case ')':
                            currentToken = RPAREN;
                            break;
                        case ';':
                            currentToken = SEMI;
                            break;
                        /*把逗号作为符号添加进词法分析*/
                        case ',':
                            currentToken = COMMA;
                            break;
                            /*把左中括号作为符号添加进词法分析*/
                        case '[':
                            currentToken = LSQU;
                            break;
                            /*把右中括号作为符号添加进词法分析*/
                        case ']':
                            currentToken = RSQU;
                            break;
                        default:
                            currentToken = ERROR;
                            break;
                    }
                }
                break;
            case S1:
                save = FALSE;
                if (c == '*') {
                    state = S2;
                } else {
                    // 除号被接受
                    ungetNextChar();
                    currentToken = OVER;
                    state = DONE;
                }
                break;
            case S2:
                save = FALSE;
                if (c == '*') {
                    state = S3;
                } else {
                    state = S2;
                }
                break;
            case S3:
                save = FALSE;
                if (c == '*') {
                    state = S3;
                } else if (c == '/') {
                    state = START;
                } else if (c == EOF) {
                    state = DONE;
                    currentToken = ENDFILE;
                } else {
                    state = S2;
                }
                break;
            case INCOMMENT:
                save = FALSE;
                if (c == EOF) {
                    state = DONE;
                    currentToken = ENDFILE;
                } else if (c == '}')
                    state = START;
                break;
            case INASSIGN:
                state = DONE;
                if (c == '=')
                    currentToken = ASSIGN;
                else { /* backup in the input */
                    ungetNextChar();
                    save = FALSE;
                    currentToken = ERROR;
                }
                break;
            case INNUM:
                if (!isdigit(c)) {
                    if (c == 'E') {
                        state = F3;
                    } else if (c == '.') {
                        state = F1;
                    } else { /* backup in the input */
                        ungetNextChar();
                        save = FALSE;
                        state = DONE;
                        currentToken = NUM;
                    }
                }
                break;
            case F1:
                if (isdigit(c)) {
                    state = F2;
                }
                break;
            case F2:
                if (!isdigit(c)) {
                    if (c == 'E') {
                        state = F3;
                    } else {
                        ungetNextChar();
                        save = FALSE;
                        state = DONE;
                        currentToken = FLOAT;
                    }
                }
                break;
            case F3:
                if (isdigit(c)) {
                    state = F5;
                } else {
                    if (c == '+' || c == '-') {
                        state = F4;
                    }
                }
                break;
            case F4:
                if (isdigit(c)) {
                    state = F5;
                }
                break;
            case F5:
                if (!isdigit(c)) {
                    ungetNextChar();
                    save = FALSE;
                    state = DONE;
                    currentToken = FLOAT;
                }
                break;
            case INID:
                if (!isalpha(c)) { /* backup in the input */
                    ungetNextChar();
                    save = FALSE;
                    state = DONE;
                    currentToken = ID;
                }
                break;
            case DONE:
            default: /* should never happen */
                fprintf(listing, "Scanner Bug: state= %d\n", state);
                state = DONE;
                currentToken = ERROR;
                break;
        }
        if ((save) && (tokenStringIndex <= MAXTOKENLEN)) {
            tokenString[tokenStringIndex++] = (char)c;
        }
        if (state == DONE) {
            tokenString[tokenStringIndex] = '\0';
            if (currentToken == ID) {
                currentToken = reservedLookup(tokenString);
            }
        }
    }
    if (TraceScan) {
        fprintf(listing, "\t%d: ", lineno);
        printToken(currentToken, tokenString);
    }
    return currentToken;
} /* end getToken */
