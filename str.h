#ifndef __STRING_H
#define __STRING_H

#include <stdio.h>
#include <string.h>
#include <stdlib.h>

/* 改行の数を数える, 行頭 行末の空白を削除する */
extern int trim(char *string);

/* input を空白で区切る */
extern char **split(char *input, int *number);

#endif // __STRING_H
