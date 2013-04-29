#include	"unp.h"
#include "data.c"

int main(int argc, char **argv)
{
	int					sockfd;
	struct sockaddr_in	servaddr;
	ssize_t		n;
	char	sendline[MAXLINE], recvline[MAXLINE];
	char res_str[MAXLINE*20];
	int len;
	Response* res;
	res = initResponse();

	if (argc != 2)
		err_quit("usage: tcpcli <IPaddress>");

	sockfd = Socket(AF_INET, SOCK_STREAM, 0);

	bzero(&servaddr, sizeof(servaddr));
	servaddr.sin_family = AF_INET;
	servaddr.sin_port = htons(SERV_PORT);
	Inet_pton(AF_INET, argv[1], &servaddr.sin_addr);

	Connect(sockfd, (SA *) &servaddr, sizeof(servaddr));

	/* authenticate */
	while (Fgets(sendline, MAXLINE, stdin) != NULL) {

		Writen(sockfd, sendline, strlen(sendline));

		if (Readline(sockfd, recvline, MAXLINE) == 0)
			err_quit("str_cli: server terminated prematurely");

		Fputs(recvline, stdout);
		getResponse(res,recvline);
		if (res->b == true)
			break;
	}

	write(sockfd,"AC_GET_FIRST_INFO\n",18);
	while (n = read(sockfd, recvline, MAXLINE) > 0) {
		Fputs(recvline, stdout);
		getResponse(res,recvline);
		if (res->b == true) {
			/* write(sockfd,"AC_HEARING\n",11); */
			break;
		}
	}
	if (n < 0)
		err_sys("str_echo: read error");

	/* while ( n = read(sockfd, recvline, MAXLINE) > 0) { */
		/* Fputs(recvline, stdout); */
		/* write(sockfd,"AC_HEARING\n",11); */
	/* } */

	exit(0);
}
