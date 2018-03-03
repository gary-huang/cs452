#ifndef SHELL_H
#define SHELL_H

#include <io.h>
#include <lib/circular_buffer.h>
#include <lib/parse.h>
#include <user/syscalls.h>
#include <user/nameserver.h>
#include <user/terminal/terminal_manager.h>

#define CMD_MAX 20
#define CMD_BUF_MAX 512

CIRCULAR_BUFFER_DEC(cmd_cb, char, CMD_BUF_MAX);


typedef struct shell {
  char cmd[CMD_MAX];
  cmd_cb buf;
  int cur_pos;
  int len;
  int count;
} shell;

void Shell();

#endif
