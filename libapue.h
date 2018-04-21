#ifndef LIBAPUE_H_
#define LIBAPUE_H_

#include <stdio.h>
#include <stdarg.h>
#include <string.h>
#include <stdlib.h>
#include <errno.h>
#include <sys/resource.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <syslog.h>
#include <fcntl.h>
#include <sys/wait.h>
#include <unistd.h>
#include <netdb.h>
#include <sys/socket.h>

#define	MAXLINE	4096
#define BUFLEN 128

#ifndef HOST_NAME_MAX
#define HOST_NAME_MAX 256
#endif

// code from the book Advanced Programming in the UNIX Environment 3rd Edition
void print_response(int sockfd);
static void err_doit(int errnoflag, int error, const char *fmt, va_list ap);
void err_sys(const char *fmt, ...);
void err_quit(const char *fmt, ...);
void err_exit(int error, const char *fmt, ...);
void daemonize(const char *cmd);
int set_cloexec(int fd);

#endif /* LIBAPUE_H_ */
