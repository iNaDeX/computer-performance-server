#include <netdb.h>
#include <errno.h>
#include <sys/socket.h>
#include <string.h>
#include <unistd.h>
#include <stdlib.h>

#include "libapue.h"

#define MAXSLEEP 128

// code based from samples from the book http://www.apuebook.com/apue3e.html

void print_response(int sockfd)
{
	int		n;
	char	buf[BUFLEN];

	while ((n = recv(sockfd, buf, BUFLEN, 0)) > 0)
		write(STDOUT_FILENO, buf, n);
	if (n < 0)
		err_sys("recv error");
}

int connect_retry(int domain, int type, int protocol, const struct sockaddr *addr, socklen_t alen)
{
	int numsec, fd;

	/*
	 * Try to connect with exponential backoff.
	 */
	for (numsec = 1; numsec <= MAXSLEEP; numsec <<= 1) {
		if ((fd = socket(domain, type, protocol)) < 0)
			return(-1);
		if (connect(fd, addr, alen) == 0) {
			/*
			 * Connection accepted.
			 */
			return(fd);
		}
		close(fd);

		/*
		 * Delay before trying again.
		 */
		if (numsec <= MAXSLEEP/2)
			sleep(numsec);
	}
	return(-1);
}

int main(int argc, char *argv[])
{
	struct addrinfo	*ailist, *aip;
	struct addrinfo	hint;
	int sockfd, err;

	char hostname[HOST_NAME_MAX] = "localhost";
	char cmd[2];
	int valid = 0;

	int c;
	opterr = 0; /* don’t want getopt() writing to stderr */
    	int res;
    	// parse command line arguments
	while ((c = getopt(argc, argv, "h:sc")) != EOF) { // -s for summary and -c for current data
		switch (c) {
		case 'h':
			strncpy(hostname,optarg,HOST_NAME_MAX);
			break;
		case 's':
			strncpy(cmd, "s",2);
			valid = 1;
			break;
		case 'c':
			strncpy(cmd, "c",2);
			valid = 1;
			break;
		case '?':
			fprintf(stderr,"unrecognized option: -%c\n", optopt);
			break;
		}
	}

	// checking that command line arguments are valid
	if(!valid) {
		fprintf(stderr, "invalid command line arguments, use -h to specify hostname, -s for summary, -c for current data\n");
		return -1;
	}

	memset(&hint, 0, sizeof(hint));
	hint.ai_socktype = SOCK_STREAM;
	hint.ai_canonname = NULL;
	hint.ai_addr = NULL;
	hint.ai_next = NULL;
	if ((err = getaddrinfo(hostname, "processinfo", &hint, &ailist)) != 0)
		err_quit("getaddrinfo error: %s", gai_strerror(err));
	for (aip = ailist; aip != NULL; aip = aip->ai_next) { // try to connect to the specified host and service
		if ((sockfd = connect_retry(aip->ai_family, SOCK_STREAM, 0,
		  aip->ai_addr, aip->ai_addrlen)) < 0) {
			err = errno;
		} else {
			//printf("sent command: %s\n", cmd);
			send(sockfd,cmd,1+strlen(cmd),0); // send the command
			print_response(sockfd); // print the server's response
			exit(0);
		}
	}
	err_exit(err, "can't connect to %s", argv[1]);
}
