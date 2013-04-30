#include	"unp.h"
#include "data.c"
#include "time.h"

int main(int argc, char **argv)
{
	time_t startTime,now;
	int duration;
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

		if (read(sockfd, recvline, MAXLINE) <= 0)
			err_quit("str_cli: server terminated prematurely");
		getResponse(res,recvline);
		Fputs(res->message, stdout);
		if (res->b == true)
			break;
	}

	/* biding */
	while (Fgets(sendline, MAXLINE, stdin) != NULL) {
		Writen(sockfd, sendline, strlen(sendline));
		if (read(sockfd, recvline, MAXLINE) <= 0)
			err_quit("str_cli: server terminated prematurely");

		getResponse(res,recvline);
		Fputs(res->message, stdout);
		if (res->b == false)
			printf("Your bid is failed!\n");
	}

	exit(0);
}
