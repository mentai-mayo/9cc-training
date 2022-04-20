#include <ctype.h>
#include <stdio.h>
#include <stdarg.h>
#include <string.h>
#include <stdlib.h>
#include <stdbool.h>

// ----- 型定義 -----

// トークンの種類
typedef enum{
  TK_RESERVED, // 記号
  TK_NUM,      // 整数トークン
  TK_EOF,      // 入力の終わりを表すトークン
} TokenKind;

// 抽象構文木のノードの種類
typedef enum{
  ND_ADD, // +
  ND_SUB, // -
  ND_MUL, // *
  ND_DIV, // /
  ND_NUM, // 整数
} NodeKind;

typedef struct Node{
  NodeKind kind;    // ノードの型
  struct Node *lhs; // 左辺
  struct Node *rhs; // 右辺
  int val;          // kindがND_NUMのとき使う
} Node;

// トークン型
typedef struct Token{
  TokenKind kind;     // トークンの型
  struct Token *next; // 次の入力トークン
  int val;            // kindがTK_NUMのとき、その数値
  char *str;          // トークン文字列
} Token;

// ----- グローバル変数 -----

// 現在着目しているトークン
Token *token;

// 入力プログラム
char *user_input;

// ----- 関数宣言 -----

void error_at(char *loc, char *fmt, ...);
bool consume(char op);
void expect(char op);
int expect_number();
bool at_eof();
Token *new_token(TokenKind kind, Token *cur, char *str);
Token *tokenize(char *p);
Node *new_node(NodeKind kind, Node *lhs, Node *rhs);
Node *new_node_num(int val);
Node *expr();
Node *mul();
Node *primary();
void gen(Node *node);

// ----- 関数実装 -----

// エラー報告用の関数
void error_at(char *loc, char *fmt, ...){
  va_list ap;
  va_start(ap, fmt);

  int pos = loc - user_input;
  fprintf(stderr, "%s\n", user_input);
  fprintf(stderr, "%*s", pos, " ");
  fprintf(stderr, "^ ");
  fprintf(stderr, fmt, ap);
  fprintf(stderr, "\n");
  exit(1);
}

/*
  次のトークンが期待している記号のときは、トークンを1つ読み進めてtrueを返す。
  それ以外の場合にはfalseを返す。
*/
bool consume(char op){
  if (token->kind != TK_RESERVED || token->str[0] != op) return false;
  token = token->next;
  return true;
}

/*
  次のトークンが期待している記号のときは、トークンを1つ読み進める。
  それ以外のときはエラーを報告する。
*/
void expect(char op){
  if(token->kind != TK_RESERVED || token->str[0] != op) error_at(token->str, "[9cc] token isnot '%c' operator", op);
  token = token->next;
}

/*
  次のトークンが数値の場合、トークンを1つ読み進めてその数値を返す。
  それ以外の場合にはエラーを報告する。
*/
int expect_number(){
  if(token->kind != TK_NUM) error_at(token->str, "[9cc] token isnot number");
  int val = token->val;
  token = token->next;
  return val;
}

bool at_eof(){
  return token->kind == TK_EOF;
}

// 新しいトークンを作成してcurにつなげる
Token *new_token(TokenKind kind, Token *cur, char *str){
  Token *tok = calloc(1, sizeof(Token));
  tok->kind = kind;
  tok->str = str;
  cur->next = tok;
  return tok;
}

// 入力文字をトークナイズしてそれを返す
Token *tokenize(char *p){
  Token head;
  head.next = NULL;
  Token *cur = &head;

  while(*p){

    // 空白文字をスキップ
    if(isspace(*p)){
      p++;
      continue;
    }

    if(*p == '+' || *p == '-' || *p == '*' || *p == '/' || *p == ')' || *p == '('){
      cur = new_token(TK_RESERVED, cur, p++);
      continue;
    }

    if(isdigit(*p)){
      cur = new_token(TK_NUM, cur, p);
      cur->val = strtol(p, &p, 10);
      continue;
    }

    error_at(token->str, "[9cc] cannot tokenize");
  }

  new_token(TK_EOF, cur, p);
  return head.next;
}

// 新しいノードを作成してrhs, lhsにつなげる
Node *new_node(NodeKind kind, Node *lhs, Node *rhs){
  Node *node = calloc(1, sizeof(Node));
  node->kind = kind;
  node->lhs = lhs;
  node->rhs = rhs;
  node->val = 0;
  return node;
}

// 整数ノードを作成する
Node *new_node_num(int val){
  Node *node = calloc(1, sizeof(Node));
  node->kind = ND_NUM;
  node->lhs = NULL;
  node->rhs = NULL;
  node->val = val;
  return node;
}

// 演算子のパース
Node *expr(){
  Node *node = mul();

  while(1){
    if(consume('+'))
      node = new_node(ND_ADD, node, mul());
    else if(consume('-'))
      node = new_node(ND_SUB, node, mul());
    else
      return node;
  }
}

// mul
Node *mul(){
  Node *node = primary();

  while(1){
    if(consume('*'))
      node = new_node(ND_MUL, node, primary());
    else if(consume('/'))
      node = new_node(ND_DIV, node, primary());
    else
      return node;
  }
}

// primary
Node *primary(){
  // 次のトークンが'('なら'(' expr ')'のはず
  if(consume('(')){
    Node *node = expr();
    expect(')');
    return node;
  }

  // そうでないなら整数のはず
  return new_node_num(expect_number());
}

// アセンブリ生成
void gen(Node *node){
  if(node->kind == ND_NUM){
    printf("  push %d\n", node->val);
    return;
  }

  // 左右の計算を済ませる
  gen(node->lhs);
  gen(node->rhs);

  // スタックから左辺と右辺(オペランド)を引っ張ってくる
  printf("  pop rdi\n");
  printf("  pop rax\n");

  switch(node->kind){
    case ND_ADD:
      printf("  add rax, rdi\n");
      break;
    case ND_SUB:
      printf("  sub rax, rdi\n");
      break;
    case ND_MUL:
      printf("  imul srax, rdi\n");
      break;
    case ND_DIV:
      printf("  cqo\n");
      printf("  idiv rdi\n");
      break;
  }

  printf("  push rax\n");
}

// ----- メイン関数 -----

int main(int argc, char *argv[]){

  // 引数をチェックする
  if(argc != 2){
    fprintf(stderr, "[9cc] invalid argument count");
    return 1;
  }

  // トークナイズしてパースする
  user_input = argv[1];
  token = tokenize(argv[1]);
  Node *node = expr();

  // アセンブリの前半部分を出力
  printf(".intel_syntax noprefix\n");
  printf(".global main\n");
  printf("main:\n");

  // 抽象構文木を下りながらコード生成
  gen(node);

  /*
    スタックトップに式全体の値が残っているはずなので、
    それをraxにロードして関数からの戻り値とする
  */
  printf("  pop rax\n");
  printf("  ret\n");

  return 0;
}
