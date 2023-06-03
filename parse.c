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
static void kind(TreeNode* t);
static TreeNode* id_lists(void);  // 变量列表
static TreeNode* id_list(TreeNode* p);
static TreeNode* dec_tmp1(void);
static void arr_de(TreeNode* t);
/* 添加函数声明语句 */
static TreeNode* function_stmt(void);  // 函数声明
static TreeNode* para_lists(void);     // 参数列表
static TreeNode* para_list(void);
static TreeNode* body(void);  // 函数体

static TreeNode* return_stmt(void);  // return语句
/* 添加函数调用 */
static TreeNode* X(TreeNode* p);
static void Q(TreeNode* t);
static void QQ(TreeNode* t);
/* 关于数组与函数的引用 */
static TreeNode* infactor(void);
static void params(TreeNode* t);
static TreeNode* inparams(void);
static TreeNode* inparam(void);
static void inpara(TreeNode* t);
/* while 和 dowhile语句 */
static TreeNode* while_stmt(void);

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
    TreeNode* t = newStmtNode(DeclareK);
    if (t) {
        t->child[0] = type();
        t->child[1] = id_lists();
    }
    return t;
}

TreeNode* id_lists(void) {
    TreeNode* t = newStmtNode(IdListK);
    if (token == ID) {
        TreeNode* p = newExpNode(VarK);
        if (p && token == ID) {
            p->attr.name = copyString(tokenString);
        }
        match(ID);
        p->sibling = id_list(p);
        if (t) t->child[0] = p;
    }
    return t;
}

TreeNode* id_list(TreeNode* p) {
    TreeNode* t = NULL;
    if (token == ASSIGN) {
        p->kind.exp = VarInK;
        match(token);
        kind(p);
        t = dec_tmp1();
    } else {
        arr_de(p);
        t = X(p);
    }
    return t;
}

TreeNode* X(TreeNode* p) {
    TreeNode* t = NULL;
    if (token == COMMA) {
        t = dec_tmp1();
    } else if (token == ASSIGN) {
        match(token);
        match(LSQU);
        Q(p);
        match(RSQU);
        t = dec_tmp1();
    }
    return t;
}

TreeNode* dec_tmp1(void) {
    TreeNode* t = NULL;
    if (token == COMMA) {
        t = newExpNode(VarK);
        match(COMMA);
        if (t && token == ID) {
            t->attr.name = copyString(tokenString);
        }
        match(ID);
        t->sibling = id_list(t);
    }
    return t;
}

void arr_de(TreeNode* t) {
    if (token == LSQU) {
        t->kind.exp = ArrK;
        match(token);
        if (t && token == NUM) {
            t->attr.dem[t->attr.pos++] = atoi(tokenString);
        }
        match(NUM);
        match(RSQU);
        arr_de(t);
    }
}

void Q(TreeNode* t) {
    if (token == NUM) {
        t->kind.exp = ArrInK;
        if (t) t->attr.init_val[t->attr.ipos++] = atoi(tokenString);
        match(token);
        QQ(t);
    }
}

void QQ(TreeNode* t) {
    if (token == COMMA) {
        match(token);
        if (t && token == NUM) {
            t->attr.init_val[t->attr.ipos++] = atoi(tokenString);
        }
        match(NUM);
        QQ(t);
    }
}

/* 函数 */
TreeNode* function_stmt(void) {
    TreeNode* t = newStmtNode(FuncK);
    match(FUNCTION);
    if (t) t->child[0] = type();
    if (t && token == ID) {
        t->attr.name = copyString(tokenString);
    }
    match(ID);
    match(LPAREN);
    if (t) t->child[1] = para_lists();
    match(RPAREN);
    if (t) t->child[2] = body();
    return t;
}

/* 函数体 */
TreeNode* body(void) {
    TreeNode* t = newStmtNode(BodyK);
    if (token == THEN) {
        match(THEN);
        if (token != END) {
            if (t) t->child[0] = stmt_sequence();
        }
        match(END);
    }
    return t;
}

/* 参数列表 */
TreeNode* para_lists(void) {
    TreeNode* t = newStmtNode(ListK);
    TreeNode* p = NULL;
    if (token == INT) {
        p = newExpNode(ParamK);
        if (p && token == INT) {
            p->attr.type = copyString(tokenString);
        }
        match(INT);
        if (p && token == ID) {
            p->attr.name = copyString(tokenString);
        }
        match(ID);
        p->sibling = para_list();
    } else if (token == COMMA) {
        p->sibling = para_list();
    }
    if (t) t->child[0] = p;
    return t;
}

TreeNode* para_list(void) {
    TreeNode* p = NULL;
    if (token == COMMA) {
        match(token);
        p = newExpNode(ParamK);
        if (p && token == INT) {
            p->attr.type = copyString(tokenString);
        }
        match(INT);
        if (p && token == ID) {
            p->attr.name = copyString(tokenString);
        }
        match(ID);
        p->sibling = para_list();
    }
    return p;
}

/* 变量类型 */
TreeNode* type(void) {
    TreeNode* t = newStmtNode(TypeK);
    if (token == INT) {
        if (t) t->attr.type = copyString(tokenString);
        match(token);
    }
    return t;
}

void kind(TreeNode* t) {
    if (token == NUM || token == ID) {
        if (t && token == NUM) {
            t->attr.val = atoi(tokenString);
        } else if (t && token == ID) {
            t->attr.type = copyString(tokenString);
        }
        match(token);
    }
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
        case ID:
            t = infactor();
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
    TreeNode* t = NULL;
    if (token == ID) {
        t = newExpNode(IdK);
        if (t) t->attr.name = copyString(tokenString);
        match(token);
        params(t);
    } else if (token == NUM) {
        t = newExpNode(ConstK);
        if ((t != NULL) && (token == NUM)) t->attr.val = atoi(tokenString);
        match(token);
    }
    return t;
}

void params(TreeNode* t) {
    if (token == LSQU) {
        t->kind.exp = ArrCK;
        match(token);
        if (token == ID || token == NUM) {
            if (t) t->attr.invo[t->attr.ppos++] = copyString(tokenString);
            match(token);
        }
        match(RSQU);
        inpara(t);
    } else if (token == LPAREN) {
        match(token);
        t->kind.exp = FunCK;
        t->child[0] = newExpNode(IdK);
        t->child[0]->attr.name = t->attr.name;
        t->child[1] = inparams();
        match(RPAREN);
    }
}

TreeNode* inparams(void) {
    TreeNode *t = NULL, *p = NULL;
    if (token == ID || token == NUM) {
        t = newStmtNode(ListK);
        p = infactor();
        if (token == COMMA) {
            p->sibling = inparam();
        }
    }
    if (t) t->child[0] = p;
    return t;
}

TreeNode* inparam(void) {
    TreeNode* t = NULL;
    if (token == COMMA) {
        match(token);
        t = infactor();
        if (token == COMMA) {
            t->sibling = inparam();
        }
    }
    return t;
}

void inpara(TreeNode* t) {
    if (token == LSQU) {
        match(token);
        if (token == ID || token == NUM) {
            if (t) t->attr.invo[t->attr.ppos++] = copyString(tokenString);
            match(token);
        }
        match(RSQU);
        inpara(t);
    }
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
