
/*******************************
 * kvs - server のクライアント *
 *******************************/

#include <string.h>
#include "hash_ring.h"
#include "sha1.h"
#include "md5.h"
#include "sort.h"
#include "wrapunix.h"

#define HOST_NAME "localhost"
#define LINE 1024

//pthread_mutex_t mutex = PTHREAD_MUTEX_INITIALIZER;
int sockfd;
static int glonum = 0;
static int sockfds[256] = {0};
char servlist[256][24];

void *send_msg(void *arg);
void *process(void *arg);
void conn_base();
int servlist_init(hash_ring_t *ring);
int key_node(char *sendline, hash_ring_t *ring);
int conn_new(char *host, int port);

//一致性哈希相关
hash_ring_t *ring;
hash_ring_node_t *node;

/* グローバル設定 */
struct {
  int port;
  char *host;
} settings;

struct thread_p {
  int threadnum;
  char *addr;
  int port;
};

void client_loop() {
  pthread_t tid;
  int flag = 0, n, i, keynode;

  ring = hash_ring_create(8, HASH_FUNCTION_SHA1);
  /*
  hash_ring_t *ring = hash_ring_create(8, HASH_FUNCTION_SHA1);
  hash_ring_node_t *node;

  char *slotA = "slotA";
  char *slotB = "slotB";

  char *keyA = "keyA";
  char *keyB = "keyBBBB";
  char *keyC = "ke)B_";
  
  node = hash_ring_find_node(ring, (uint8_t*)keyA, strlen(keyA));

  //assert(hash_ring_add_node(ring, (uint8_t*)slotA, strlen(slotA)) == HASH_RING_OK);
  //assert(node != NULL && node->nameLen == strlen(slotA) && memcmp(node->name, slotA, strlen(slotA)) == 0);
  */

  //初始化链接
  conn_base(ring);

  sleep(1);
  printf("kvs > ");
  while (1) {
    char sendline[MAXLINE] = "";
    fflush(stdout);
    if (fgets(sendline, MAXLINE - 2, stdin) == NULL) break;
    sendline[strlen(sendline) - 1] = '\0';
	//一致性哈希选择节点
    keynode = key_node(sendline, ring);
	printf("%d\n", keynode);
    strcat(sendline, "\r\n");
	printf("sock %d 接收哒...\n", sockfds[keynode + 1]);
    Write(sockfds[keynode + 1], sendline, strlen(sendline));
  }
  shutdown(sockfds[1], SHUT_WR);
  
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

int key_node(char *sendline, hash_ring_t *ring) {
  int i;
  char *key;
  char s[] = "                        ";
  char *d = " ";
  char *next = NULL;
  char *tmp = NULL;
  
  strcpy(s, sendline);
  //切割第一次
  next = strtok_r(s, d, &tmp);
  printf("%s\n", next);
  //切割第二次
  next = strtok_r(NULL, d, &tmp);
  printf("%s\n", next);
  if(next != NULL) key = next;
	else strcpy(key, d);

  node = hash_ring_find_node(ring, (uint8_t*)key, strlen(key));
  for (i = 0; i < glonum; i++) {
    if(!strcmp((char *)node->name, servlist[i]))
      return i;
  }
  
  return 0;
}

int servlist_init(hash_ring_t *ring) {
  int i = 0;
  FILE *fp;
  char *buf, *p;

  if ((fp = fopen("server.conf","r")) == NULL) {
    printf("Cannot open file!\n");
    exit(0);
  }

  buf = (char *)malloc(LINE*sizeof(char));
  p = fgets(buf, LINE, fp);  //将每行的内容读到buf中

  while (p) {
	strcpy(servlist[i], p);
	hash_ring_add_node(ring, (uint8_t*)servlist[i], strlen(servlist[i]));
	i++;
    p = fgets(buf, LINE, fp);  //指针移到下一行
  }

  fclose(fp);
  return i;
}

void conn_base(hash_ring_t *ring) {
  int i, servnum;
  char s[] = "                        ";
  char *d = " ";
  struct thread_p thread_pt[256];
  pthread_t tid;

  servnum = servlist_init(ring);
  glonum = servnum;
  for (i = 0; i < servnum; i++) {
	char *next = NULL;
	char *tmp = NULL;
    strcpy(s, servlist[i]);
	//切割第一次
	next = strtok_r(s, d, &tmp);
    thread_pt[i].threadnum = i;	  
    thread_pt[i].addr = next;
	//切割第二次
	next = strtok_r(NULL, d, &tmp);
	thread_pt[i].port = atoi(next);
	//printf("%s on %d\n", thread_pt[i].addr, thread_pt[i].port);

	//创建线程来连接服务器
	Pthread_create(&tid, NULL, process, (void *)&thread_pt[i]);
  }
  
  //Pthread_create(&tid, NULL, process, (void *)&thread_pt[0]);
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
  //printf("socknum has %d\n", socknum);
  return connfd;
}

/* 本処理 */
void *process(void *arg) {
  int n, i, num;
  struct thread_p *thread_pt = (struct thread_p *) arg;
  char *host = thread_pt->addr;
  int port = thread_pt->port;

  //printf("%s---%d---%s\n", host, port, settings.host);
  //Pthread_create(&tid, NULL, send_msg, NULL);
  
  //Pthread_mutex_lock(&mutex);
  sockfd = conn_new(settings.host, port);
  //socknum ++;
  num = thread_pt->threadnum + 1;
  sockfds[num] = sockfd;
  printf("process goes %d times\n sock %d here\n", num, sockfds[num]);
  //Pthread_mutex_unlock(&mutex);

  /* サーバー ⇒ 標準出力 */
  while (1) {
    char recvline[MAXLINE] = "";
	//for (i = 0; i < 5; i++) {
	//	printf("%d ------> %d over?\n", num, sockfds[i]);
	//}
	printf("sock %d with %d is ready\n", sockfds[num], num);
    if ((n = read(sockfds[num], recvline, MAXLINE)) == 0) break;
    fputs(recvline, stdout);
	//printf("---------------- %d socks ----------------\n", socknum);
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
  //servlist_init();

  //connfd2 = conn_new(7002);
  //process(connfd);
  
  return 0;
}
