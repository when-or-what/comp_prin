/****************************************************/
/* File: util.c                                     */
/* Utility function implementation                  */
/* for the TINY compiler                            */
/* Compiler Construction: Principles and Practice   */
/* Kenneth C. Louden                                */
/****************************************************/

// #include "globals.h"
#include "util.h"

/* Procedure printToken prints a token
 * and its lexeme to the listing file
 */
void printToken(TokenType token, const char *tokenString) {
    switch (token) {
        case IF:
        case THEN:
        case ELSE:
        case END:
        case REPEAT:
        case UNTIL:
        case READ:
        case WRITE:
        case INT:
        case FUNCTION:
        case WHILE:
        case DO:
        case RETURN:
            fprintf(listing, "reserved word: %s\n", tokenString);
            break;
        case ASSIGN:
            fprintf(listing, ":=\n");
            break;
        case LT:
            fprintf(listing, "<\n");
            break;
        case EQ:
            fprintf(listing, "=\n");
            break;
        case LPAREN:
            fprintf(listing, "(\n");
            break;
        case RPAREN:
            fprintf(listing, ")\n");
            break;
        case SEMI:
            fprintf(listing, ";\n");
            break;
        case PLUS:
            fprintf(listing, "+\n");
            break;
        case MINUS:
            fprintf(listing, "-\n");
            break;
        case TIMES:
            fprintf(listing, "*\n");
            break;
        case OVER:
            fprintf(listing, "/\n");
            break;
            /* 添加逗号以及中括号 */
        case COMMA:
            fprintf(listing, ",\n");
            break;
        case LSQU:
            fprintf(listing, "[\n");
            break;
        case RSQU:
            fprintf(listing, "]\n");
            break;
        case ENDFILE:
            fprintf(listing, "EOF\n");
            break;
        case NUM:
            fprintf(listing, "NUM, val= %s\n", tokenString);
            break;
        case ID:
            fprintf(listing, "ID, name= %s\n", tokenString);
            break;
        case ERROR:
            fprintf(listing, "ERROR: %s\n", tokenString);
            break;
        case FLOAT:
            fprintf(listing, "FLOAT, val= %s\n", tokenString);
            break;
        default: /* should never happen */
            fprintf(listing, "Unknown token: %d\n", token);
    }
}

/* Function newStmtNode creates a new statement
 * node for syntax tree construction
 */
TreeNode *newStmtNode(StmtKind kind) {
    TreeNode *t = (TreeNode *)malloc(sizeof(TreeNode));
    int i;
    if (t == NULL)
        fprintf(listing, "Out of memory error at line %d\n", lineno);
    else {
        for (i = 0; i < MAXCHILDREN; i++) t->child[i] = NULL;
        t->sibling = NULL;
        t->nodekind = StmtK;
        t->kind.stmt = kind;
        t->lineno = lineno;
    }
    return t;
}

/* Function newExpNode creates a new expression
 * node for syntax tree construction
 */
TreeNode *newExpNode(ExpKind kind) {
    TreeNode *t = (TreeNode *)malloc(sizeof(TreeNode));
    int i;
    if (t == NULL)
        fprintf(listing, "Out of memory error at line %d\n", lineno);
    else {
        for (i = 0; i < MAXCHILDREN; i++) t->child[i] = NULL;
        t->sibling = NULL;
        t->nodekind = ExpK;
        t->kind.exp = kind;
        t->lineno = lineno;
        t->attr.dem = (int *)malloc(MAX_DEM * sizeof(int));
        t->attr.pos = 0;
        t->attr.init_val = (int *)malloc(MAX_NUM * sizeof(int));
        t->attr.ipos = 0;
        t->attr.invo = (char **)malloc(MAX_DEM * sizeof(char *));
        t->attr.ppos = 0;
        t->type = Void;
    }
    return t;
}

/* Function copyString allocates and makes a new
 * copy of an existing string
 */
char *copyString(char *s) {
    int n;
    char *t;
    if (s == NULL) return NULL;
    n = strlen(s) + 1;
    t = malloc(n);
    if (t == NULL)
        fprintf(listing, "Out of memory error at line %d\n", lineno);
    else
        strcpy(t, s);
    return t;
}

/* Variable indentno is used by printTree to
 * store current number of spaces to indent
 */
static int indentno = 0;

/* macros to increase/decrease indentation */
#define INDENT indentno += 2
#define UNINDENT indentno -= 2

/* printSpaces indents by printing spaces */
static void printSpaces(void) {
    int i;
    for (i = 0; i < indentno; i++) fprintf(listing, " ");
}

/* procedure printTree prints a syntax tree to the
 * listing file using indentation to indicate subtrees
 */
char *str, *ss;
void printTree(TreeNode *tree) {
    int i;
    INDENT;
    while (tree != NULL) {
        printSpaces();
        if (tree->nodekind == StmtK) {
            switch (tree->kind.stmt) {
                case IfK:
                    fprintf(listing, "If\n");
                    break;
                case RepeatK:
                    fprintf(listing, "Repeat\n");
                    break;
                case AssignK:
                    fprintf(listing, "Assign to: %s\n", tree->attr.name);
                    break;
                case ReadK:
                    fprintf(listing, "Read: %s\n", tree->attr.name);
                    break;
                case WriteK:
                    fprintf(listing, "Write\n");
                    break;
                case WhileK:
                    fprintf(listing, "While\n");
                    break;
                case ReturnK:
                    fprintf(listing, "Return\n");
                    break;
                case FuncK:
                    fprintf(listing, "Function: %s\n", tree->attr.name);
                    break;
                case TypeK:
                    fprintf(listing, "Type: %s\n", tree->attr.type);
                    break;
                case BodyK:
                    fprintf(listing, "Function-Body: \n");
                    break;
                case ListK:
                    fprintf(listing, "Parameter-List: \n");
                    break;
                case DeclareK:
                    fprintf(listing, "Declare: \n");
                    break;
                case IdListK:
                    fprintf(listing, "Variable-List: \n");
                    break;
                default:
                    fprintf(listing, "Unknown ExpNode kind\n");
                    break;
            }
        } else if (tree->nodekind == ExpK) {
            switch (tree->kind.exp) {
                case OpK:
                    fprintf(listing, "Op: ");
                    printToken(tree->attr.op, "\0");
                    break;
                case ConstK:
                    fprintf(listing, "Const: %d\n", tree->attr.val);
                    break;
                case IdK:
                    fprintf(listing, "Id: %s\n", tree->attr.name);
                    break;
                case ParamK:
                    fprintf(listing, "Param (%s): %s\n", tree->attr.name,
                            tree->attr.type);
                    break;
                case ArrCK:
                    str = (char *)malloc(BUF_SIZE);
                    memset(str, 0, sizeof(str));
                    for (int i = 0; i < tree->attr.ppos; i++) {
                        sprintf(str + strlen(str), "[%s]", tree->attr.invo[i]);
                    }
                    fprintf(listing, "Array-Call: %s%s\n", tree->attr.name,
                            str);
                    break;
                case FunCK:
                    fprintf(listing, "Function-Call: \n");
                    break;
                case VarK:
                    fprintf(listing, "Var (%s): uninitialized\n",
                            tree->attr.name);
                    break;
                case VarInK:
                    if (tree->attr.type) {
                        fprintf(listing, "Var (%s): %s\n", tree->attr.name,
                                tree->attr.type);
                    } else {
                        fprintf(listing, "Var (%s): %d\n", tree->attr.name,
                                tree->attr.val);
                    }
                    break;
                case ArrK:
                    str = (char *)malloc(BUF_SIZE);
                    memset(str, 0, sizeof(str));
                    for (int i = 0; i < tree->attr.pos; i++) {
                        sprintf(str + strlen(str), "[%d]", tree->attr.dem[i]);
                    }
                    fprintf(listing, "Array (%s%s): uninitialized\n",
                            tree->attr.name, str);
                    free(str);
                    break;
                case ArrInK:
                    str = (char *)malloc(BUF_SIZE);
                    memset(str, 0, sizeof(str));
                    for (int i = 0; i < tree->attr.pos; i++) {
                        sprintf(str + strlen(str), "[%d]", tree->attr.dem[i]);
                    }

                    ss = (char *)malloc(BUF_SIZE);
                    memset(ss, 0, sizeof(ss));
                    sprintf(ss + strlen(ss), "initialized with: {");
                    for (int i = 0; i < tree->attr.ipos - 1; i++) {
                        sprintf(ss + strlen(ss), "%d, ",
                                tree->attr.init_val[i]);
                    }
                    if (tree->attr.ipos - 1 >= 0) {
                        sprintf(ss + strlen(ss), "%d}",
                                tree->attr.init_val[tree->attr.ipos - 1]);
                    }

                    fprintf(listing, "Array (%s%s): %s\n", tree->attr.name, str,
                            ss);
                    free(str);
                    free(ss);
                    break;
                default:
                    fprintf(listing, "Unknown ExpNode kind\n");
                    break;
            }
        } else
            fprintf(listing, "Unknown node kind\n");
        for (i = 0; i < MAXCHILDREN; i++) printTree(tree->child[i]);
        tree = tree->sibling;
    }
    UNINDENT;
}
