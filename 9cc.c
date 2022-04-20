#include <stdio.h>
#include <stdlib.h>

int main(int argc, char *argv[]){

  // 引数をチェックする
  if(argc != 2){
    fprintf(stderr, "[9cc] invalid argument count");
    return 1;
  }

  char *p = argv[1];

  // コンパイラメイン部分
  printf(".intel_syntax noprefix\n");
  printf(".global main\n");
  printf("main:\n");

  // まずは最初の数をセットする
  printf("  mov rax, %ld\n", strtol(p, &p, 10));

  // 残り
  while(*p){

    // +が来たとき
    if(*p == '+'){
      p++;
      printf("  add rax, %ld\n", strtol(p, &p, 10));
      continue;
    }

    // -が来たとき
    if(*p == '-'){
      p++;
      printf("  sub rax, %ld\n", strtol(p, &p, 10));
      continue;
    }

    // 他
    fprintf(stderr, "[9cc] unexpected character: '%c'(%d)\n", *p, *p);
    return 1;
  }

  printf("  ret\n");

  return 0;
}
