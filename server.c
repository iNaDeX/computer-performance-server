#include <netdb.h>
#include <errno.h>
#include <syslog.h>
#include <fcntl.h>
#include <sys/socket.h>
#include <sys/wait.h>
#include <stdarg.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>

#include "libapue.h"

#define QLEN 10

// code based from samples from the book http://www.apuebook.com/apue3e.html

char get_query(int sockfd)
{
	int n;
	char buf[BUFLEN];

	do {
		n = recv(sockfd, buf, BUFLEN, 0);
		if (n < 0)
			err_sys("recv error");
	}while (n == 0); // wait until we get command (while loop in case we get interrupted)
	return buf[0];
}

int initserver(int type, const struct sockaddr *addr, socklen_t alen, int qlen)
{
	int fd, err;
	int reuse = 1;

	if ((fd = socket(addr->sa_family, type, 0)) < 0)
		return(-1);
	if (setsockopt(fd, SOL_SOCKET, SO_REUSEADDR, &reuse,
	  sizeof(int)) < 0)
		goto errout;
	if (bind(fd, addr, alen) < 0)
		goto errout;
	if (type == SOCK_STREAM || type == SOCK_SEQPACKET)
		if (listen(fd, qlen) < 0)
			goto errout;
	return(fd);

errout:
	err = errno;
	close(fd);
	errno = err;
	return(-1);
}

void serve(int sockfd)
{
	int		clfd, status;
	pid_t	pid;
	char command;
	// shell command that lists files in current directory, filters by .dat files,
	//sorts by filename (here by date thanks to filenaming),
	// limits results to latest file, and sends the filename to cat to display it
	char * getLatestSummary = "ls | grep '_proc.dat' | sort -r | head -n 1 | xargs cat"; // relative path OK because server is started next to agent. in real life I would choose a fixed absolute path and use it, but you wouldn't be able to test it easily on your computer.
	char * getCurrentData =   "./getData"; // same

	set_cloexec(sockfd);
	for (;;) {
		if ((clfd = accept(sockfd, NULL, NULL)) < 0) {
			syslog(LOG_ERR, "server: accept error: %s",
			  strerror(errno));
			exit(1);
		}
		command = get_query(clfd);
		if ((pid = fork()) < 0) {
			syslog(LOG_ERR, "server: fork error: %s",
			  strerror(errno));
			exit(1);
		} else if (pid == 0) {	/* child */
			/*
			 * The parent called daemonize ({Prog daemoninit}), so
			 * STDIN_FILENO, STDOUT_FILENO, and STDERR_FILENO
			 * are already open to /dev/null.  Thus, the call to
			 * close doesn't need to be protected by checks that
			 * clfd isn't already equal to one of these values.
			 */
			if (dup2(clfd, STDOUT_FILENO) != STDOUT_FILENO ||
			  dup2(clfd, STDERR_FILENO) != STDERR_FILENO) {
				syslog(LOG_ERR, "server: unexpected error");
				exit(1);
			}
			close(clfd);
			if(command == 'c') {
				execl(getCurrentData, "getCurrentData", (char *)0);
			} else if(command == 's') {
				execl("/bin/sh", "getSummary", "-c", getLatestSummary ,(char *)0);
			} else {
				fprintf(stderr, "error, invalid query"); // sent to socket because of redirection
				exit(1);
			}
			syslog(LOG_ERR, "server: unexpected return from exec: %s", strerror(errno));
		} else {		/* parent */
			close(clfd);
			waitpid(pid, &status, 0);
		}
	}
}

int main(int argc, char *argv[])
{
	struct addrinfo	*ailist, *aip;
	struct addrinfo	hint;
	int				sockfd, err, n;
	char			*host;

	if (argc != 1)
		err_quit("usage: server");
	if ((n = sysconf(_SC_HOST_NAME_MAX)) < 0)
		n = HOST_NAME_MAX;	/* best guess */
	if ((host = malloc(n)) == NULL)
		err_sys("malloc error");
	if (gethostname(host, n) < 0)
		err_sys("gethostname error");
	daemonize("serverDaemon");
	memset(&hint, 0, sizeof(hint));
	hint.ai_flags = AI_CANONNAME;
	hint.ai_socktype = SOCK_STREAM;
	hint.ai_canonname = NULL;
	hint.ai_addr = NULL;
	hint.ai_next = NULL;
	if ((err = getaddrinfo(host, "processinfo", &hint, &ailist)) != 0) {
		syslog(LOG_ERR, "server: getaddrinfo error: %s",
		  gai_strerror(err));
		exit(1);
	}
	for (aip = ailist; aip != NULL; aip = aip->ai_next) {
		if ((sockfd = initserver(SOCK_STREAM, aip->ai_addr,
		  aip->ai_addrlen, QLEN)) >= 0) {
			serve(sockfd);
			exit(0);
		}
	}
	exit(1);
}
