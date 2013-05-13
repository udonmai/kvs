
/*******************************
 * kvs - server のクライアント *
 *******************************/
#include "hash_ring.h"
#include "sha1.h"
#include "md5.h"
#include "sort.h"
#include "wrapunix.h"

#define HOST_NAME "localhost"

static int sockfd;
void *send_msg(void *arg);

/* グローバル設定 */
struct {
  int port;
  char *host;
} settings;

/* 本処理 */
void process(int connfd) {
  pthread_t tid;
  int n;
  sockfd = connfd;
  Pthread_create(&tid, NULL, send_msg, NULL);
  
  /* サーバー ⇒ 標準出力 */
  while (1) {
    char recvline[MAXLINE] = "";
    if ((n = read(sockfd, recvline, MAXLINE)) == 0) break;
    fputs(recvline, stdout);
    printf("kvs > "); fflush(stdout);
  }
}

/* サーバー ⇐ 標準入力 */
void *send_msg(void *arg) {
  printf("kvs > ");
  while (1) {
    char sendline[MAXLINE] = "";
    fflush(stdout);
    if (fgets(sendline, MAXLINE - 2, stdin) == NULL) break;
    sendline[strlen(sendline) - 1] = '\0';
    strcat(sendline, "\r\n");
    Write(sockfd, sendline, strlen(sendline));
  }
  shutdown(sockfd, SHUT_WR);
  return NULL;
}

/* 新しい接続 */
int conn_new() {
  struct sockaddr_in servaddr;
  struct hostent *hptr;
  struct in_addr **pptr;
  int connfd;
  connfd = Socket(AF_INET, SOCK_STREAM, 0);
  bzero(&servaddr, sizeof(servaddr));
  servaddr.sin_family = AF_INET;
  servaddr.sin_port = htons(settings.port);
  if ((hptr = gethostbyname(settings.host)) == NULL) {
    err_quit("Invalid host name");
  }
  pptr = (struct in_addr **)hptr->h_addr_list;
  memcpy(&servaddr.sin_addr, *pptr, sizeof(struct in_addr));
  printf("connecting to server %s\n", hptr->h_name);
  Connect(connfd, &servaddr, sizeof(servaddr));
  return connfd;
}

/* 設定の初期化 */
void settings_init(void) {
  settings.port = 9877;
  settings.host = HOST_NAME;
}

/* 使い方 */
void usage(void) {
  printf("-h ... 接続するホストを指定.\n" \
         "-p ... 接続するホストのポート番号を指定.\n");
  exit(1);
}

int main(int argc, char **argv) {
  int connfd;
  char ch;

  settings_init();
  while (-1 != (ch = getopt(argc, argv, "p:h:"))) {
    switch (ch) {
    case 'p':
      settings.port = atoi(optarg);
      break;
    case 'h':
      settings.host = optarg;
      break;
    default:
      fprintf(stderr, "Illigal option \"%c\"\n", ch);
      usage();
      return 1;
    }
  }

	hash_ring_t *ring = hash_ring_create(8, HASH_FUNCTION_SHA1);
	char *slotA = "slotA";
	char *slotB = "slotB";

	char *keyA = "keyA";
	char *keyB = "keyBBBB";
	char *keyC = "keyB_";

	hash_ring_node_t *node;

	assert(hash_ring_add_node(ring, (uint8_t*)slotA, strlen(slotA)) == HASH_RING_OK);
	assert(hash_ring_add_node(ring, (uint8_t*)slotB, strlen(slotB)) == HASH_RING_OK);


	node = hash_ring_find_node(ring, (uint8_t*)keyA, strlen(keyA));
	assert(node != NULL && node->nameLen == strlen(slotA) && memcmp(node->name, slotA, strlen(slotA)) == 0);

	printf("udonmai\n", keyB);
  //connfd = conn_new();
  //process(connfd);
  return 0;
}
