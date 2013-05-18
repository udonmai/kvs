
/*******************************
 * kvs - server のクライアント *
 *******************************/

#include "hash_ring.h"
#include "sha1.h"
#include "md5.h"
#include "sort.h"
#include "wrapunix.h"

#define HOST_NAME "localhost"

pthread_mutex_t mutex = PTHREAD_MUTEX_INITIALIZER;
int sockfd;
volatile int socknum = 0;
static int num = 0;
static int sockfds[256] = {0};

void *send_msg(void *arg);
void *process(void *arg);
void conn_base();

//一致性哈希相关
hash_ring_t *ring;
hash_ring_node_t *node;

int conn_new(char *host, int port);

/* グローバル設定 */
struct {
  int port;
  char *host;
} settings;

struct thread_p {
  int threadnum;
  char* addr;
  int port;
};

void client_loop() {
  pthread_t tid;
  int flag = 0, n, i;
  //sockfd = connfd;

  //Pthread_create(&tid, NULL, conn_base, NULL);
  //Pthread_create(&tid, NULL, send_msg, NULL);
  
  conn_base();

  printf("kvs > ");
  while (1) {
    char sendline[MAXLINE] = "";
    fflush(stdout);
    if (fgets(sendline, MAXLINE - 2, stdin) == NULL) break;
    sendline[strlen(sendline) - 1] = '\0';
    strcat(sendline, "\r\n");
    Write(sockfds[0], sendline, strlen(sendline));
	for (i = 0; i < 10; i++) {
		printf("%d over?\n", sockfds[i]);
	}
  }
  shutdown(sockfds[0], SHUT_WR);
  
  /*
  while(1) {
    char recvline[MAXLINE] = "";
	//for (i = 0; i < socknum; i++) {
	  if (sockfds[0] == 0) continue;
	  if ((n = read(sockfds[0], recvline, MAXLINE)) == 0) {flag = 1; break;}
	  fputs(recvline, stdout);
	//}
	if (flag) break;

	printf("kvs > asdfasdfa");
	fflush(stdout);
  }
	*/
}

void conn_base() {
  pthread_t tid;

  struct thread_p thread_pt[256];
  thread_pt[0].threadnum = 1;
  thread_pt[1].threadnum = 2;
  thread_pt[2].threadnum = 3;
  thread_pt[0].port = 7001;
  thread_pt[1].port = 7002;
  thread_pt[2].port = 7003;

  Pthread_create(&tid, NULL, process, (void *)&thread_pt[0]);
  Pthread_create(&tid, NULL, process, (void *)&thread_pt[1]);
  Pthread_create(&tid, NULL, process, (void *)&thread_pt[2]);
}

/* サーバー ⇐ 標準入力 */
void *send_msg(void *arg) {
  int i;
 /* 
  printf("kvs > ");
  while (1) {
    char sendline[MAXLINE] = "";
    fflush(stdout);
    if (fgets(sendline, MAXLINE - 2, stdin) == NULL) break;
    sendline[strlen(sendline) - 1] = '\0';
    strcat(sendline, "\r\n");
    Write(sockfds[socknum - 3], sendline, strlen(sendline));
	for (i = -10; i < 1; i++) {
		printf("%d over?\n", sockfds[i]);
	}
  }
  shutdown(sockfds[0], SHUT_WR);
  */
  return NULL;
}

/* 新しい接続 */
int conn_new(char *host, int port) {
  struct sockaddr_in servaddr;
  struct hostent *hptr;
  struct in_addr **pptr;
  int connfd;
  connfd = Socket(AF_INET, SOCK_STREAM, 0);
  bzero(&servaddr, sizeof(servaddr));
  servaddr.sin_family = AF_INET;
  servaddr.sin_port = htons(port);
  if ((hptr = gethostbyname(host)) == NULL) {
    err_quit("Invalid host name");
  }
  pptr = (struct in_addr **)hptr->h_addr_list;
  memcpy(&servaddr.sin_addr, *pptr, sizeof(struct in_addr));
  printf("connecting to server %s:%d\n", hptr->h_name, port);
  Connect(connfd, &servaddr, sizeof(servaddr));
  //socknum ++; //在process中加锁
  printf("socknum has %d\n", socknum);
  return connfd;
}

/* 本処理 */
void *process(void *arg) {
  int n;
  struct thread_p *thread_pt = (struct thread_p *) arg;
  int port = thread_pt->port;
  //Pthread_create(&tid, NULL, send_msg, NULL);
  
  //Pthread_mutex_lock(&mutex);
  sockfd = conn_new(settings.host, port);
  //socknum ++;
  num = thread_pt->threadnum - 1;
  sockfds[num] = sockfd;
  printf("process goes %d times\n sock %d here\n", num, sockfd);
  //Pthread_mutex_unlock(&mutex);

  /* サーバー ⇒ 標準出力 */
  while (1) {
    char recvline[MAXLINE] = "";
	printf("%d is ready\n", num);
    if ((n = read(sockfds[num], recvline, MAXLINE)) == 0) break;
    fputs(recvline, stdout);
	printf("---------------- %d socks ----------------\n", socknum);
    printf("kvs > "); fflush(stdout);
  }

  return NULL;
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
  int connfd, connfd2;
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

  client_loop();

  /*
  hash_ring_t *ring = hash_ring_create(8, HASH_FUNCTION_SHA1);
  hash_ring_node_t *node;

  char *slotA = "slotA";
  char *slotB = "slotB";

  char *keyA = "keyA";
  char *keyB = "keyBBBB";
  char *keyC = "keyB_";
  
  node = hash_ring_find_node(ring, (uint8_t*)keyA, strlen(keyA));

  //assert(hash_ring_add_node(ring, (uint8_t*)slotA, strlen(slotA)) == HASH_RING_OK);
  //assert(node != NULL && node->nameLen == strlen(slotA) && memcmp(node->name, slotA, strlen(slotA)) == 0);
  */ 
 
  //connfd2 = conn_new(7002);
  //process(connfd);
  
  return 0;
}
