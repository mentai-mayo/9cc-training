#include <stdio.h>
#include <stdlib.h>

int main(int argc, char *argv[]){

  // 引数をチェックする
  if(argc != 2){
    fprintf(stderr, "[9cc] invalid argument count");
    return 1;
  }

  // コンパイラメイン部分
  printf(".intel_syntax noprefix\n");
  printf(".global main\n");
  printf("main:\n");
  printf("  mov rax, %d\n", atoi(argv[1]));
  printf("  ret\n");

  return 0;
}
