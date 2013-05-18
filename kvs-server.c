
/************************************
 * kvs-server - 簡易key value store *
 ************************************/

#include "kvs-server.h"
#include "readline.h"
#include "wrapunix.h"
#include "hash.h"
#include "str.h"

pthread_mutex_t mutex = PTHREAD_MUTEX_INITIALIZER;
int line_num = 0;

/* クライアントとの対話メッセージ */
typedef struct {
  int connfd;
  char *recvline;
  char *sendline;
  int argc;
  char **argv;
  bool fin_flg;
} client_t;

/* コマンドの統計 */
struct {
  unsigned int get;
  unsigned int set;
  unsigned int delete;
  unsigned int find;
  unsigned int mem;
} hash_state = {
  0, 0, 0, 0, 0,
};

/* 全体の設定 */
struct {
  bool do_daemon;
  int port;
  char *inter;
  double uptime;
} settings;

/* 関数プロトタイプ宣言 */
void cmd_set(client_t *client);
void cmd_get(client_t *client);
void cmd_find(client_t *client);
void cmd_delete(client_t *client);
void cmd_status(client_t *client);
void cmd_error(client_t *client);
void cmd_help(client_t *client);
void cmd_quit(client_t *client);
void cmd_purge(client_t *client);

/* コマンドテーブル */
struct {
  char *name;
  void (*func)(client_t *client);
} cmd_tb[] = {
  { "set", cmd_set },
  { "get", cmd_get },
  { "find", cmd_find },
  { "delete", cmd_delete },
  { "status", cmd_status },
  { "error", cmd_error },
  { "purge", cmd_purge },
  { "help", cmd_help },
  { "quit", cmd_quit },
};
int cmd_tb_size = sizeof(cmd_tb) / sizeof(cmd_tb[0]);

/* 現在の時間を設定する */
double get_time() {
  struct timeval tv;
  gettimeofday(&tv, NULL);
  return tv.tv_sec + tv.tv_usec * 1e-6;
}

/* メッセージを設定する */
void msg_set(client_t *client, char *fmt, ...) {
  va_list ap;
  char line[MAXLINE] = "";
  va_start(ap, fmt);
  vsnprintf(line, MAXLINE - 2, fmt, ap);
  strcat(line, "\r\n");
  client->sendline = malloc(sizeof(char) * (strlen(line) + 1));
  strcpy(client->sendline, line);
  va_end(ap);
}

/* key value を設定する */
void cmd_set(client_t *client) {
  char *key, *value;
  if (client->argc < 3) {
    msg_set(client, "set のフォーマットが不正です. [set <key> <value>]");
    return;
  }
  key = client->argv[1];
  value = client->argv[2];
  Pthread_mutex_lock(&mutex);
  arr_insert(key, value);
  hash_state.set++;
  hash_state.mem +=
    sizeof(hash_record_t) + strlen(key) + strlen(value) + 2;
  Pthread_mutex_unlock(&mutex);
  msg_set(client, "OK. %s - %s", key, value);
}

/* key value を取得する */
void cmd_get(client_t *client) {
  char *value = NULL, *key = NULL;
  if (client->argc < 2) {
    msg_set(client, "get のフォーマットが不正です。 [get <key>]");
  }
  key = client->argv[1];
  Pthread_mutex_lock(&mutex);
  value = arr_get(key);
  hash_state.get++;
  Pthread_mutex_unlock(&mutex);
  if (value == NULL) {
    msg_set(client, "%s : no value", key);
  }
  else {
    msg_set(client, "(key, value): (%s, %s)", key, value);
  }
}

/* key が存在するかどうかを確認する */
void cmd_find(client_t *client) {
  bool check;
  if (client->argc < 2) {
    msg_set(client, "find のフォーマットが不正です。 [find <key>]");
  }
  check = arr_find(client->argv[1]);
  Pthread_mutex_lock(&mutex);
  hash_state.find++;
  Pthread_mutex_unlock(&mutex);
  if (check) {
    msg_set(client, "true");
  }
  else {
    msg_set(client, "false");
  }
}

/* key に紐づく value を削除する */
void cmd_delete(client_t *client) {
  int check;
  char *value = NULL, *key = NULL;
  if (client->argc < 2) {
    msg_set(client, "delete のフォーマットが不正です。 [delete <key>]");
  }
  key = client->argv[1];
  value = arr_get(key);
  check = arr_delete(client->argv[1]);
  if (check) {
  Pthread_mutex_lock(&mutex);
  hash_state.delete++;
  hash_state.mem -=
    sizeof(hash_record_t) + strlen(key) + strlen(value) + 2;
  Pthread_mutex_unlock(&mutex);
    msg_set(client, "success");
  }
  else {
    msg_set(client, "fail");
  }
}

/* 現在のステータスを取得する */
void cmd_status(client_t *client) {
  int number = arr_get_num();
  msg_set(client,
          "uptime        ... %9.2lf sec\r\n" \
          "port number   ... %9d\r\n" \
          "key numbers   ... %9d\r\n" \
          "get cmd       ... %9d\r\n" \
          "set cmd       ... %9d\r\n" \
          "delete cmd    ... %9d\r\n" \
          "find cmd      ... %9d\r\n" \
          "memory alloc  ... %9d",
          (get_time() - settings.uptime), settings.port,
          number, hash_state.get, hash_state.set,
          hash_state.delete, hash_state.find,
          hash_state.mem);
}

/* help を表示する */
void cmd_help(client_t *client) {
  msg_set(client,
          "set <key> <value> ... <key> <value>の値を保存する.\r\n"  \
          "get <key> ... <key> に対応する値を取得する\r\n" \
          "delete <key> ... <key> に対応する値を削除する\r\n" \
          "find <key> ... <key> が存在するかどうかを確認する\r\n" \
          "purge ... キャッシュを削除する\r\n" \
          "help ... このメッセージを出力する\r\n" \
          "status  ... サーバーの状態を表示する\r\n" \
          "quit ... サーバーとの接続を切る" \
          );
}

/* キャッシュを削除する */
void cmd_purge(client_t *client) {  
  Pthread_mutex_lock(&mutex);
  arr_free();
  arr_init();
  Pthread_mutex_unlock(&mutex);
}

/* 接続を切る */
void cmd_quit(client_t *client) {
  client->fin_flg = true;
}

/* エラーメッセージ */
void cmd_error(client_t *client) {
  msg_set(client, "---- error ----");
}

/* メッセージの初期化 */
void message_init(client_t *client, int connfd) {
  client->recvline = NULL;
  client->sendline = NULL;
  client->argv = NULL;
  client->argc = 0;
  client->connfd = connfd;
  client->fin_flg = false;

  printf("%d is connected.\n", connfd);
}

/* メッセージを取得 */
void message_get(client_t *client) {
  int n;
  client->recvline = malloc(sizeof(char) * MAXLINE);
  if ((n = readline(client->connfd, client->recvline, MAXLINE)) <= 0) {    
    /* 相手がcloseした */
    client->fin_flg = true;
  }
  else {
    if (client->recvline != NULL) {
      Pthread_mutex_lock(&mutex);
      line_num++;
      //printf("## total = %d, line = %s\n", line_num, client->recvline);
      Pthread_mutex_unlock(&mutex);
    }
  }
}

/* メッセージを解析 */
void message_parse(client_t *client) {
  char **recvline, *cmd;
  int recv_size;
  int i;
  if (client->fin_flg == true || client->recvline == NULL) return;
  recvline = split(client->recvline, &recv_size);
  client->argc = recv_size;
  client->argv = recvline;
  cmd = recvline[0];

  /* コマンドを実行*/
  for (i = 0; i < cmd_tb_size; i++) {
    if (strcmp(cmd_tb[i].name, cmd) == 0) {
      cmd_tb[i].func(client);
      break;
    }
  }
  if (i == cmd_tb_size) {
    cmd_error(client);
  }
}

/* メッセージを送信 */
void message_send(client_t *client) {
  if (client->sendline != NULL) {
    write(client->connfd, client->sendline, strlen(client->sendline));
	printf("%s on %d\n", client->sendline, client->connfd);
	printf("receive %s \n", client->recvline);
  }
}

/* ポインタを開放 */
void message_free(client_t *client) {
  free(client->recvline);
  free(client->sendline);
  if (client->argv != NULL) {    
    free(client->argv);
  }
  client->argc = 0;
}

/* クライアントとの対話 */
void process_client(int connfd) {
  client_t *client = malloc(sizeof(client_t));

  while (1) {
    message_init(client, connfd);
    message_get(client);
    message_parse(client);
    message_send(client);
    message_free(client);
    if (client->fin_flg) break;
  }
  
  free(client);
}

/* クライアント個別処理(本処理) */
void *process(void *arg) {
  int connfd;
  connfd = *((int *)arg);
  free(arg);
  Pthread_detach(pthread_self());
  process_client(connfd);
  Close(connfd);
  return NULL;
}

/* listenfd を用意する */
int conn_listen(void) {
  struct sockaddr_in servaddr;
  int listenfd;
  listenfd = Socket(AF_INET, SOCK_STREAM, 0);
  bzero(&servaddr, sizeof(servaddr));
  servaddr.sin_family = AF_INET;
  servaddr.sin_port = htons(settings.port);
  if (settings.inter != NULL) {
    Inet_pton(AF_INET, settings.inter, &servaddr.sin_addr.s_addr);
  }
  else {
    servaddr.sin_addr.s_addr = htonl(INADDR_ANY);
  }
  Bind(listenfd, &servaddr, sizeof(servaddr));
  Listen(listenfd, LISTENQ);
  return listenfd;
}

/* settingを初期化する */
void settings_init() {
  settings.do_daemon = false;
  settings.port = 9877;
  settings.uptime = get_time();
  settings.inter = NULL;
}

/* 使い方を表示する */
void usage() {
  printf("-p ... listenするポート番号を指定する.\n" \
         "-l ... listenするローカルアドレスを指定する.\n" \
         "-h ... helpメッセージを出力する.\n" \
         "-d ... daemonモードで動作させる\n");
}

int main(int argc, char *argv[]) {
  struct sockaddr_in *cliaddr = NULL;
  int listenfd, i = 0;
  int *connfd = NULL;
  socklen_t len;
  pthread_t tid;
  char ch;

  settings_init();
  while (-1 != (ch = getopt(argc, argv, "dhp:l:"))) {
    switch (ch) {
    case 'p' :
      settings.port = atoi(optarg);
      break;
    case 'h':
      usage();
      exit(0);
      break;
    case 'd':
      settings.do_daemon = true;
      break;
    case  'l':
      settings.inter = optarg;
      break;
    default:
      fprintf(stderr, "Illegal argument \"%c\"", ch);
      return 1;
    }
  }

  if (settings.do_daemon) {
    daemonize();
  }
  
  signal(SIGPIPE, SIG_IGN); 
  listenfd = conn_listen();
  arr_init();
  while (1) {
    len = sizeof(*cliaddr);
    connfd = malloc(sizeof(int));
    *connfd = Accept(listenfd, NULL, NULL);
    Pthread_create(&tid, NULL, &process, connfd);
	i++;
	printf("%d\n", i);
  }
  arr_free();
  return 0;
}
