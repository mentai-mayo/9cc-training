#include <ctype.h>
#include <stdio.h>
#include <stdarg.h>
#include <string.h>
#include <stdlib.h>
#include <stdbool.h>

// トークンの種類
typedef enum{
  TK_RESERVED, // 記号
  TK_NUM,      // 整数トークン
  TK_EOF,      // 入力の終わりを表すトークン
} TokenKind;

// トークン型
typedef struct Token{
  TokenKind kind; // トークンの型
  struct Token *next;    // 次の入力トークン
  int val;        // kindがTK_NUMのとき、その数値
  char *str;      // トークン文字列
} Token;

// 現在着目しているトークン
Token *token;

// エラー報告用の関数
void error(char *fmt, ...){
  va_list ap;
  va_start(ap, fmt);
  vfprintf(stderr, fmt, ap);
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
  if(token->kind != TK_RESERVED || token->str[0] != op) error("[9cc] token isnot '%c' operator", op);
  token = token->next;
}

/*
  次のトークンが数値の場合、トークンを1つ読み進めてその数値を返す。
  それ以外の場合にはエラーを報告する。
*/
int expect_number(){
  if(token->kind != TK_NUM) error("[9cc] token isnot number");
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

    if(*p == '+' || *p == '-'){
      cur = new_token(TK_RESERVED, cur, p++);
      continue;
    }

    if(isdigit(*p)){
      cur = new_token(TK_NUM, cur, p);
      cur->val = strtol(p, &p, 10);
      continue;
    }

    error("[9cc] cannot tokenize");
  }

  new_token(TK_EOF, cur, p);
  return head.next;
}

int main(int argc, char *argv[]){

  // 引数をチェックする
  if(argc != 2){
    fprintf(stderr, "[9cc] invalid argument count");
    return 1;
  }

  // トークナイズする
  token = tokenize(argv[1]);

  // アセンブリの前半部分を出力
  printf(".intel_syntax noprefix\n");
  printf(".global main\n");
  printf("main:\n");

  /*
    式の最初は数ではならないので、それをチェックして最初のmovを出力
  */
  printf("  mov rax, %ld\n", expect_number());

  // "+ <num>"または"- <num>"というトークンを消費しつつアセンブリを出力
  while(!at_eof()){
    if(consume('+')){
      printf("  add rax, %d\n", expect_number());
      continue;
    }
    
    expect('-');
    printf("  sub rax, %d\n", expect_number());
  }

  printf("  ret\n");

  return 0;
}
