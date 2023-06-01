/****************************************************/
/* File: parse.c                                    */
/* The parser implementation for the TINY compiler  */
/* Compiler Construction: Principles and Practice   */
/* Kenneth C. Louden                                */
/****************************************************/

// #include "globals.h"
#include "parse.h"

#include "scan.h"
#include "util.h"

static TokenType token; /* holds current token */

/* function prototypes for recursive calls */
static TreeNode* stmt_sequence(void);
static TreeNode* statement(void);
static TreeNode* if_stmt(void);
static TreeNode* repeat_stmt(void);
static TreeNode* assign_stmt(void);
static TreeNode* read_stmt(void);
static TreeNode* write_stmt(void);
static TreeNode* expp(void);
static TreeNode* simple_exp(void);
static TreeNode* term(void);
static TreeNode* factor(void);
/* 添加变量声明语句，含一维数组 */
static TreeNode* declare_stmt(void);  // 变量声明
static TreeNode* type(void);          // 变量类型，只有int
static TreeNode* kind(void);
static TreeNode* id_lists(void);  // 变量列表
static TreeNode* id_list(void);
static TreeNode* dec_tmp1(void);
static TreeNode* dec_tmp2(void);
static TreeNode* arr_de(void);
/* 添加函数声明语句 */
static TreeNode* function_stmt(void);  // 函数声明
static TreeNode* para_lists(void);     // 参数列表
static TreeNode* para_list(void);
static TreeNode* body(void);  // 函数体

static TreeNode* return_stmt(void);  // return语句
/* 添加函数调用 */
static TreeNode* X(void);
static TreeNode* Q(void);
static TreeNode* QQ(void);
/* 关于数组与函数的引用 */
static TreeNode* infactor(void);
static TreeNode* params(void);
static TreeNode* inparams(void);
static TreeNode* inparam(void);
/* while 和 dowhile语句 */
static TreeNode* while_stmt(void);
static TreeNode* dowhile_stmt(void);

static void syntaxError(char* message) {
    fprintf(listing, "\n>>> ");
    fprintf(listing, "Syntax error at line %d: %s", lineno, message);
    Error = TRUE;
}

static void match(TokenType expected) {
    if (token == expected)
        token = getToken();
    else {
        syntaxError("unexpected token -> ");
        printToken(token, tokenString);
        fprintf(listing, "      ");
    }
}

TreeNode* stmt_sequence(void) {
    TreeNode* t = statement();
    TreeNode* p = t;
    while ((token != ENDFILE) && (token != END) && (token != ELSE) &&
           (token != UNTIL)) {
        TreeNode* q;
        match(SEMI);
        q = statement();
        if (q != NULL) {
            if (t == NULL)
                t = p = q;
            else /* now p cannot be NULL either */
            {
                p->sibling = q;
                p = q;
            }
        }
    }
    return t;
}

/* 添加变量声明 */
TreeNode* statement(void) {
    TreeNode* t = NULL;
    switch (token) {
        case IF:
            t = if_stmt();
            break;
        case REPEAT:
            t = repeat_stmt();
            break;
        case ID:
            t = assign_stmt();
            break;
        case READ:
            t = read_stmt();
            break;
        case WRITE:
            t = write_stmt();
            break;
        /* 变量声明语句 */
        case INT:
            t = declare_stmt();
            break;
        /* 函数 */
        case FUNCTION:
            t = function_stmt();
            break;
        /* while语句 */
        case WHILE:
            t = while_stmt();
            break;
        /* return语句 */
        case RETURN:
            t = return_stmt();
            break;
        default:
            syntaxError("unexpected token -> ");
            printToken(token, tokenString);
            token = getToken();
            break;
    } /* end case */
    return t;
}

TreeNode* if_stmt(void) {
    TreeNode* t = newStmtNode(IfK);
    match(IF);
    if (t != NULL) t->child[0] = expp();
    match(THEN);
    if (t != NULL) t->child[1] = stmt_sequence();
    if (token == ELSE) {
        match(ELSE);
        if (t != NULL) t->child[2] = stmt_sequence();
    }
    match(END);
    return t;
}

TreeNode* repeat_stmt(void) {
    TreeNode* t = newStmtNode(RepeatK);
    match(REPEAT);
    if (t != NULL) t->child[0] = stmt_sequence();
    match(UNTIL);
    if (t != NULL) t->child[1] = expp();
    return t;
}

TreeNode* assign_stmt(void) {
    TreeNode* t = newStmtNode(AssignK);
    if ((t != NULL) && (token == ID)) t->attr.name = copyString(tokenString);
    match(ID);
    match(ASSIGN);
    if (t != NULL) t->child[0] = expp();
    return t;
}

TreeNode* read_stmt(void) {
    TreeNode* t = newStmtNode(ReadK);
    match(READ);
    if ((t != NULL) && (token == ID)) t->attr.name = copyString(tokenString);
    match(ID);
    return t;
}

TreeNode* write_stmt(void) {
    TreeNode* t = newStmtNode(WriteK);
    match(WRITE);
    if (t != NULL) t->child[0] = expp();
    return t;
}

/* return 语句 */
TreeNode* return_stmt(void) {
    TreeNode* t = newStmtNode(ReturnK);
    match(RETURN);
    if (t != NULL) t->child[0] = expp();
    return t;
}

// while 语句
TreeNode* while_stmt(void) {
    TreeNode* t = newStmtNode(WhileK);
    match(WHILE);
    if (t != NULL) t->child[0] = expp();
    match(DO);
    if (t != NULL) t->child[1] = stmt_sequence();
    match(END);
    return t;
}

/* 变量声明语句 */
TreeNode* declare_stmt(void) {
    type();
    id_lists();
    return NULL;
}

TreeNode* id_lists(void) {
    if (token == ID) {
        match(ID);
        id_list();
    }
    return NULL;
}

TreeNode* id_list(void) {
    if (token == ASSIGN) {
        match(token);
        kind();
        dec_tmp1();
    } else {
        arr_de();
        X();
    }
    return NULL;
}

TreeNode* X(void) {
    if (token == COMMA) {
        dec_tmp1();
    } else if (token == ASSIGN) {
        match(token);
        match(LSQU);
        Q();
        match(RSQU);
        dec_tmp1();
    }
    return NULL;
}

TreeNode* dec_tmp1(void) {
    if (token == COMMA) {
        match(COMMA);
        match(ID);
        dec_tmp2();
    }
    return NULL;
}

TreeNode* dec_tmp2(void) {
    if (token == ASSIGN) {
        match(token);
        kind();
        dec_tmp1();
    } else {
        arr_de();
        X();
    }
    return NULL;
}

TreeNode* arr_de(void) {
    if (token == LSQU) {
        match(token);
        match(NUM);
        match(RSQU);
        arr_de();
    }
    return NULL;
}

TreeNode* Q(void) {
    if (token == NUM) {
        match(token);
        QQ();
    }
    return NULL;
}

TreeNode* QQ(void) {
    if (token == COMMA) {
        match(token);
        match(NUM);
        QQ();
    }
    return NULL;
}

/* 函数 */
TreeNode* function_stmt(void) {
    match(FUNCTION);
    type();
    match(ID);
    match(LPAREN);
    para_lists();
    match(RPAREN);
    body();
    return NULL;
}

TreeNode* body(void) {
    if (token == THEN) {
        match(THEN);
        if (token != END) {
            stmt_sequence();
        }
        match(END);
    }
    return NULL;
}

/* 参数列表 */
TreeNode* para_lists(void) {
    if (token == INT) {
        match(INT);
        match(ID);
        para_list();
    } else if (token == COMMA) {
        para_list();
    }
    return NULL;
}

TreeNode* para_list(void) {
    if (token == COMMA) {
        match(token);
        type();
        match(ID);
        para_list();
    }
    return NULL;
}

/* 变量类型 */
TreeNode* type(void) {
    if (token == INT) {
        match(token);
    }
    return NULL;
}

TreeNode* kind(void) {
    if (token == NUM || token == ID) {
        match(token);
    }
    return NULL;
}

TreeNode* expp(void) {
    TreeNode* t = simple_exp();
    if ((token == LT) || (token == EQ)) {
        TreeNode* p = newExpNode(OpK);
        if (p != NULL) {
            p->child[0] = t;
            p->attr.op = token;
            t = p;
        }
        match(token);
        if (t != NULL) t->child[1] = simple_exp();
    }
    return t;
}

TreeNode* simple_exp(void) {
    TreeNode* t = term();
    while ((token == PLUS) || (token == MINUS)) {
        TreeNode* p = newExpNode(OpK);
        if (p != NULL) {
            p->child[0] = t;
            p->attr.op = token;
            t = p;
            match(token);
            t->child[1] = term();
        }
    }
    return t;
}

TreeNode* term(void) {
    TreeNode* t = factor();
    while ((token == TIMES) || (token == OVER)) {
        TreeNode* p = newExpNode(OpK);
        if (p != NULL) {
            p->child[0] = t;
            p->attr.op = token;
            t = p;
            match(token);
            p->child[1] = factor();
        }
    }
    return t;
}

TreeNode* factor(void) {
    TreeNode* t = NULL;
    switch (token) {
        case NUM:
            t = newExpNode(ConstK);
            if ((t != NULL) && (token == NUM)) t->attr.val = atoi(tokenString);
            // match(NUM);
            infactor();
            break;
        case ID:
            t = newExpNode(IdK);
            if ((t != NULL) && (token == ID))
                t->attr.name = copyString(tokenString);
            // match(ID);
            // if (token == LPAREN) {
            //     // 函数调用
            //     match(LPAREN);
            //     fun_call();
            //     match(RPAREN);
            // } else if (token == LSQU) {
            //     // 数组元素引用
            //     match(LSQU);
            //     if (token == NUM) {
            //         match(NUM);
            //     } else if (token == ID) {
            //         match(ID);
            //     }
            //     match(RSQU);
            // }
            infactor();
            break;
        case LPAREN:
            match(LPAREN);
            t = expp();
            match(RPAREN);
            break;
        default:
            syntaxError("unexpected token -> ");
            printToken(token, tokenString);
            token = getToken();
            break;
    }
    return t;
}

TreeNode* infactor(void) {
    if (token == ID) {
        match(token);
        params();
    } else if (token == NUM) {
        match(token);
    }
    return NULL;
}

TreeNode* params(void) {
    if (token == LSQU) {
        match(token);
        kind();
        match(RSQU);
        params();
    } else if (token == LPAREN) {
        match(token);
        inparams();
        match(RPAREN);
    }
    return NULL;
}

TreeNode* inparams(void) {
    if (token == ID || token == NUM) {
        infactor();
        inparam();
    }
    return NULL;
}

TreeNode* inparam(void) {
    if (token == COMMA) {
        match(token);
        infactor();
        inparam();
    }
    return NULL;
}

/****************************************/
/* the primary function of the parser   */
/****************************************/
/* Function parse returns the newly
 * constructed syntax tree
 */
TreeNode* parse(void) {
    TreeNode* t;
    token = getToken();
    t = stmt_sequence();
    if (token != ENDFILE) syntaxError("Code ends before file\n");
    return t;
}
