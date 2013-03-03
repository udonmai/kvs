
/******************************
 * 文字列解析を行うライブラリ *
 ******************************/

#include "str.h"

/* 空白の数を数える, 行頭 行末の空白を削除する */
int trim(char *string) {
  int length;                   /* string の長さ */
  int counter = 0;              /* 空白の数 */
  int i = 0;
  char *pos;

  /* 改行を削除 */
  if ((pos = strchr(string, '\r')) != NULL) {
    *pos = '\0';
  }
  if ((pos = strchr(string, '\n')) != NULL) {
    *pos = '\0';
  }
  
  /* 空白の数を数える */
  length = strlen(string);
  for (i = 0; i < length; i++) {
    if (' ' == string[i]) {
      string[i] = '\0';
      counter++;
    }
  }  
  return counter;
}

/* input を空白で区切る */
char **split(char *str, int *number) {
  char **string;                /* 空白で区切った文字列を格納 */
  char *input = malloc(sizeof(char) * (strlen(str) + 1));
  int space_number;             /* 空白の数 */
  int i;
  strcpy(input, str);
  space_number = trim(input);
  *number = (space_number + 1);
  string = malloc(sizeof(char *) * *number);
  for(i = 0; i < *number; i++) {
    string[i] = input;
    input += strlen(input) + 1;
  }

  return string;
}
