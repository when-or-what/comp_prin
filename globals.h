/****************************************************/
/* File: globals.h                                  */
/* Global types and vars for TINY compiler          */
/* must come before other include files             */
/* Compiler Construction: Principles and Practice   */
/* Kenneth C. Louden                                */
/****************************************************/

#ifndef _GLOBALS_H_
#define _GLOBALS_H_

#include <ctype.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#ifndef FALSE
#define FALSE 0
#endif

#ifndef TRUE
#define TRUE 1
#endif

/* MAXRESERVED = the number of reserved words */
/* 添加一个类型保留字int，函数关键字 function return while语句关键字 do while*/
#define MAXRESERVED 13

/* 变量的数值类型 */
// #define MAX_TYPES 1
// extern char* types[MAX_TYPES] = {"int"};

typedef enum
/* book-keeping tokens */
{ ENDFILE,
  ERROR,
  /* reserved words */
  IF,
  THEN,
  ELSE,
  END,
  REPEAT,
  UNTIL,
  READ,
  WRITE,
  INT,      /* 保留字int */
  FUNCTION, /* 保留字 function */
  WHILE,    /* 保留字while */
  DO,       /* 保留字do */
  RETURN,   /* 保留字return */
  /* multicharacter tokens */
  ID,
  NUM,
  /* special symbols */
  ASSIGN,
  EQ,
  LT,
  PLUS,
  MINUS,
  TIMES,
  OVER,
  LPAREN,
  RPAREN,
  SEMI,
  COMMA, /* 逗号 */
  LSQU,  /* 左中括号 */
  RSQU,  /* 右中括号 */
  FLOAT } TokenType;

extern FILE* source;  /* source code text file */
extern FILE* listing; /* listing output text file */
extern FILE* code;    /* code text file for TM simulator */

extern int lineno; /* source line number for listing */

/**************************************************/
/***********   Syntax tree for parsing ************/
/**************************************************/

typedef enum { StmtK, ExpK } NodeKind;
/* 增加变量声明的定义语句 */
typedef enum {
    IfK,
    RepeatK,
    AssignK,
    ReadK,
    WriteK,
    DeclareK, /* 变量声明语句类型 */
    WhileK,   /* while语句类型 */
    FuncK,    /* 函数 */
    ReturnK,  /* return语句 */
    TypeK,    /* 变量类型/函数返回值类型 */
    BodyK,    /* 函数体 */
    ListK,    /* 参数列表 */
    IdListK,  /* 变量列表 */
} StmtKind;
// typedef enum { IfK, RepeatK, AssignK, ReadK, WriteK } StmtKind;

typedef enum {
    OpK,
    ConstK,
    IdK,
    ParamK,
    VarK,
    VarInK,
    ArrK,
    ArrInK,
    ArrCK,
    FunCK
} ExpKind;

/* ExpType is used for type checking */
typedef enum { Void, Integer, Boolean } ExpType;

#define MAXCHILDREN 3
#define MAX_DEM 10    // 假设数组的维数最大为10维
#define MAX_NUM 20    // 假设数组的初值最多为20个整数
#define BUF_SIZE 100  // 临时字符串缓冲区长度

typedef struct treeNode {
    struct treeNode* child[MAXCHILDREN];
    struct treeNode* sibling;
    int lineno;
    NodeKind nodekind;
    union {
        StmtKind stmt;
        ExpKind exp;
    } kind;
    struct {
        TokenType op;
        int val;
        char* name;
        char* type;
        int* dem;       // 数组维数
        int pos;        // 维数数组最后一个位置下标
        int* init_val;  // 数组初值
        int ipos;       // 初值数组最后一个下标
        char** invo;    // 数组引用的下标
        int ppos;
    } attr;
    ExpType type; /* for type checking of exps */
} TreeNode;

/**************************************************/
/***********   Flags for tracing       ************/
/**************************************************/

/* EchoSource = TRUE causes the source program to
 * be echoed to the listing file with line numbers
 * during parsing
 */
extern int EchoSource;

/* TraceScan = TRUE causes token information to be
 * printed to the listing file as each token is
 * recognized by the scanner
 */
extern int TraceScan;

/* TraceParse = TRUE causes the syntax tree to be
 * printed to the listing file in linearized form
 * (using indents for children)
 */
extern int TraceParse;

/* TraceAnalyze = TRUE causes symbol table inserts
 * and lookups to be reported to the listing file
 */
extern int TraceAnalyze;

/* TraceCode = TRUE causes comments to be written
 * to the TM code file as code is generated
 */
extern int TraceCode;

/* Error = TRUE prevents further passes if an error occurs */
extern int Error;
#endif
