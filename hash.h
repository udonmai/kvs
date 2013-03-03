#ifndef __HASH_H
#define __HASH_H

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <assert.h>
#include <stdbool.h>

#define HASH_DENSITY 1.3        /* hashの密度 */
#define DEFAULT_SIZE 1024       /* デフォルトのhashテーブルの大きさ */

/* レコード */
typedef struct hash_record_tag {
  struct hash_record_tag *next;
  unsigned int hash;
  char *key;
  char *value;
} hash_record_t;

/* hashテーブルを作成する */
extern void arr_init(void);

/* hashテーブルを初期化する */
extern void arr_free(void);

/* key, value を挿入する */
extern int arr_insert(char *key, char *value);

/* key を取得する */
extern char *arr_get(char *key);

/* key を削除する */
extern int arr_delete(char *key);

/* key を見つける */
extern bool arr_find(char *key);

/* 保存されているkeyの数を出力する */
extern int arr_get_num(void);

#endif // __HASH_H
