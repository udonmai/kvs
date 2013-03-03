
/***************************************************************/
/* 目的 - エラー処理を含めた一般的なネットワーク関数を定義する */
/* 機能 - ラッパー関数は全て一文字目が大文字である             */
/***************************************************************/

#include "wrapunix.h"

int
Socket (int family, int type, int protocol) {
  int n;
  if ((n = socket(family, type, protocol)) < 0)
    err_ret("socket error");
  return n;
}

void
Bind(int listenfd, void *servaddr, int size) {
  if (bind(listenfd, servaddr, size) < 0)
    err_ret("bind error");
}

void
Listen(int listenfd, int backlog) {
  char *ptr;
  if ((ptr = getenv("LISTENQ")) != NULL)
    backlog = atoi(ptr);
  
  if (listen(listenfd, backlog) < 0)
    err_ret("listen error");
}

int
Accept(int sockfd, void *servaddr, socklen_t *size) {
  int connfd;
  if ((connfd = accept(sockfd, servaddr, size)) < 0) {
    err_ret("connect error");
  }
  return connfd;
}

void
Write(int sockfd, char *buff, int size) {
  if (write(sockfd, buff, size) < 0) {
    err_ret("write error");
  }
}

int
Read(int sockfd, char *buff, int size) {
  int n;
  if ((n = read(sockfd, buff, size)) < 0) {
    err_ret("read error");
  }
  return n;
}

void
Close(int fd) {
  if (close(fd) < 0) {
    err_ret("close error");
  }
}

const char *
Inet_ntop(int family, const void *addr, char *line, size_t line_size) {
  const char *string;
  if ((string = inet_ntop(family, addr, line, line_size)) == NULL)
    err_ret("inet_ntop error");
  printf("string = %s", string);
  return string;
}

void
Inet_pton(int family, char *ip_str, void *s_addr) {
  if (inet_pton(family, ip_str, s_addr) < 0)
    err_ret("inet_pton error");
}

int Recv(int sockfd, char *line, int *len) {
  if (recv(sockfd, line, *len, MSG_WAITALL) <= 0)
    err_ret("recv error: ");
  return *len;
}

int Connect(int sockfd, void *servaddr, socklen_t len) {
  if (connect(sockfd, servaddr, len) < 0)
    err_quit("connection error: ");
  return len;
}

int Select(int nfds, fd_set *rfds, fd_set *wrds, fd_set *efds, struct timeval *timeout) {
  int flg;
  flg = select(nfds, rfds, wrds, efds, timeout);
  if (0 > flg) {
    err_ret("select error");
  }
  else if (0 == flg) {
    err_ret("timeout");
  }
  return flg;
}

int Fcntl(int fd, int cmd, int arg) {
  int n;
  if ((n = fcntl(fd, cmd, arg)) == -1) {
    err_ret("fcntl");
  }
  return n;
}

pid_t Fork() {
  pid_t pid;
  if ((pid = fork()) < 0) {
    err_ret("fork error");
  }
  return pid;
}

void *Malloc(size_t size) {
  void *ptr;
  if ((ptr = malloc(size)) == NULL) {
    err_ret("malloc error");
  }
  return ptr;
}

void Pthread_create(pthread_t *tid, const pthread_attr_t *attr, void *(*func)(void *), void *arg) {
  if (pthread_create(tid, attr, func, arg) != 0) {
    err_ret("pthread create error");
  }
}

void Pthread_detach(pthread_t tid) {
  if (pthread_detach(tid) != 0) {
    err_ret("pthread detach error");
  }
}

void Pthread_mutex_lock(pthread_mutex_t *mptr) {
  if (pthread_mutex_lock(mptr) != 0) {
    err_ret("pthread mutex lock error");
  }
}

void Pthread_mutex_unlock(pthread_mutex_t *mptr) {
  if (pthread_mutex_unlock(mptr) != 0) {
    err_ret("pthread mutex unlock error");
  }
}


/***************************************/
/* error.c - エラー処理を定義          */
/* 目的 - 汎用的なエラー処理を定義する */
/***************************************/

static void err_doit(int errnoflg, const char *fmt, va_list ap);

/* エラー文を出力 (errno あり） */
void
err_ret(const char *fmt, ...) {
  va_list ap;
  va_start(ap, fmt);
  err_doit(1, fmt, ap);
  va_end(ap);
  return;
}

/* エラー文を出力 (errno なし） */
void
err_msg(const char *fmt, ...) {
  va_list ap;
  va_start(ap, fmt);
  err_doit(0, fmt, ap);
  va_end(ap);
  return ;
}

/* エラー文を出力し強制終了 */
void
err_quit(const char *fmt, ...) {
  va_list ap;
  va_start(ap, fmt);
  err_doit(0, fmt, ap);
  va_end(ap);
  exit(1);
}

static void
err_doit(int errnoflg, const char *fmt, va_list ap) {
  int errno_save, n;
  char buf[MAXLINE + 1];
  errno_save = errno;
  vsprintf(buf, fmt, ap);
  if (errnoflg) {
    n = strlen(buf);
    snprintf(&buf[n], MAXLINE - n, ": %s", strerror(errno_save));
  }
  strcat(buf, "\n");
  fflush(stdout);
  fputs(buf, stderr);
  fflush(stderr);
  return;
}

