
/******************************************
 * デーモンプロセスになるためのスクリプト *
 ******************************************/

#include <syslog.h>
#include "wrapunix.h"

/* プロセスがデーモンとなる */
void daemonize() {
  pid_t pid;

  /* 親プロセスの終了 */
  if ((pid = fork()) != 0) {
    exit(0);
  }
  
  /* セッションリーダーに昇格 */
  setsid();

  signal(SIGHUP, SIG_IGN);
  return;
}
