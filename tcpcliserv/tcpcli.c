#include	"unp.h"
#include <sys/types.h>
#include <sys/ipc.h>
#include <sys/shm.h>
#include "stdbool.h"
#include "data.c"
#include "string.h"
#include "time.h"

char* name;
bool* isConnected;
bool* isLoggedOut;
int* remainingTime;

void clearString(char c[]) {
	int i;
	for (i=0; i<MAXLINE; i++)
		c[i] = '\0';
}

void errorQuit() {
	kill(0, SIGTERM); //kill all child processes rudely
	/* err_quit("str_cli: server terminated prematurely"); */
	*isLoggedOut = true;
	printf("Server error.\n");
	exit(0);
}

int main(int argc, char **argv)
{

	struct pollfd fds;
	int ret;
	fds.fd = 0; /* this is STDIN */
	fds.events = POLLIN;

	/* share memory */
	int name_shm_id = shmget(IPC_PRIVATE,sizeof(char)*50,0600);
	name = shmat(name_shm_id,NULL,0);
	int isConnected_shm_id = shmget(IPC_PRIVATE,sizeof(bool),0600);
	isConnected = shmat(isConnected_shm_id,NULL,0);
	int isLoggedOut_shm_id = shmget(IPC_PRIVATE,sizeof(bool),0600);
	isLoggedOut = shmat(isLoggedOut_shm_id,NULL,0);
	*isLoggedOut = false;
	*isConnected = false;
	int remainingTime_shm_id = shmget(IPC_PRIVATE,sizeof(int),0600);
	remainingTime = shmat(remainingTime_shm_id,NULL,0);
	*remainingTime = -1;

	time_t now,start;
	int current_product,prev_product;

	pid_t				childpid;
	int					sockfd;
	int					sockfdtime;
	struct sockaddr_in	servaddr;
	ssize_t		n;
	char	sendline[MAXLINE], recvline[MAXLINE], *temp;
	int len,i;
	bool print = argv[2];
	Response* res;
	res = initResponse();

	if (argc < 2)
		err_quit("usage: tcpcli <IPaddress> [print=true]");

	if ((childpid = Fork()) == 0) { //time counter (process con)

		struct timespec tim;
		tim.tv_sec = 1;
		tim.tv_nsec = 0;

		sockfdtime = Socket(AF_INET, SOCK_STREAM, 0);

		bzero(&servaddr, sizeof(servaddr));
		servaddr.sin_family = AF_INET;
		servaddr.sin_port = htons(6666);
		Inet_pton(AF_INET, argv[1], &servaddr.sin_addr);

		Connect(sockfdtime, (SA *) &servaddr, sizeof(servaddr));

		/* authenticate */
		// do client co 2 ket noi den server (2process) nen ca 2 ket not nay can duoc login vao server
		// process con nay se duoc login vao khi process me login vao truoc
		// tuc la khi ton tai *isConnected = true
		while (true) {
			nanosleep(&tim,NULL);
			if (*isLoggedOut == true)
				exit(0);
			if (*isConnected == false) // khi process chinh da login thi process con nay moi login
				continue;
			clearString(recvline);
			sprintf(sendline,"AC_LOGIN name=\"%s\"\n",name);
			Writen(sockfdtime, sendline, strlen(sendline));

			if (read(sockfdtime, recvline, MAXLINE) <= 0)
				errorQuit();
			getResponse(res,recvline);
			if (res->b == true)
				break;
		}
		//sau khi login sau thi cung cap nhat id product hien thoi request dc tu phia server
		do {
			nanosleep(&tim,NULL);
			Writen(sockfdtime, "AC_GET_CURRENT_PRODUCT\n", 23);
			clearString(recvline);
			if (read(sockfdtime, recvline, MAXLINE) <= 0)
				errorQuit();
			getResponse(res,recvline);
			temp = getValOfStr("val",res->message);
		} while(temp == NULL);
		current_product = atoi(temp);

		/* time sync with server */
		i = 0;
		while(true) {
			// cap nhat remaining time cho phien dau gia hien thoi
			nanosleep(&tim,NULL);
			// request tiep theo se tra ve thong tin day du cua san pham
			// hoac thong tin thang cuoc cua san pham neu san pham do' da~ het' han. bid ben phia server
			sprintf(sendline,"AC_GET_PRODUCT_INFO val=\"%d\"\n",current_product);
			Writen(sockfdtime, sendline, strlen(sendline));
			clearString(recvline);
			if (read(sockfdtime, recvline, MAXLINE) <= 0)
				errorQuit();
			getResponse(res,recvline);
			// cap nhat gia tri remainingTime moi' duoc gui tu server, nam trong header
			temp = getValOfStr("remainingTime",res->header);
			if (temp != NULL) {
				*remainingTime = atoi(temp);
			}
			// server gui gia tri current_product khi co su thay doi san pham ben phia server
			// vi vay can cap nhat id san pham moi o phia client
			temp = getValOfStr("current_product",res->header);
			if (temp != NULL) {
				// set = 2 de in 2 lan, lan 1 se la thong tin san pham truoc (nguoi thang cuoc)
				// lan 2 se la thong tin san pham moi
				// vi 2 lan co current_product khac nhau
				i = 2;
				prev_product = current_product;
				current_product = atoi(temp);
			}
			/* print information */
			if (print == true || i > 0) {
				system("clear");
				printf("%s",res->message);
				Fputs("-----------------\nCommand : ",stdout);
				fflush(stdout);
			}
			i > 0 ? i-- : i;
		}

	} else { //day la process parent
		sockfd = Socket(AF_INET, SOCK_STREAM, 0);

		bzero(&servaddr, sizeof(servaddr));
		servaddr.sin_family = AF_INET;
		servaddr.sin_port = htons(6666);
		Inet_pton(AF_INET, argv[1], &servaddr.sin_addr);

		Connect(sockfd, (SA *) &servaddr, sizeof(servaddr));

		/* authenticate */
		Fputs("-----------------\nCommand : ",stdout);
		while (Fgets(sendline, MAXLINE, stdin) != NULL) {

			Writen(sockfd, sendline, strlen(sendline));

			clearString(recvline);
			if (read(sockfd, recvline, MAXLINE) <= 0)
				errorQuit();
			getResponse(res,recvline);
			Fputs(res->message, stdout);
			Fputs("-----------------\nCommand : ",stdout);
			if (res->b == true) {
				strcpy(name,getValOfStr("name",sendline));
				*isConnected = true;
				break;
			}
		}

		/* biding */
		// cai nay la phan chinh cua process nay
		// nhan lenh tu ban phim roi send den server
		// server nhan dc phan hoi lai
		// client bat' bang ham read roi in ra
		// cac command nhu:
		// AC_LOGIN name="a"
		// AC_BID val="15.00"
		// AC_LOGOUT
		while (Fgets(sendline, MAXLINE, stdin) != NULL) {
			Writen(sockfd, sendline, strlen(sendline));
			clearString(recvline);
			if (read(sockfd, recvline, MAXLINE) <= 0)
				errorQuit();
			getResponse(res,recvline);
			system("clear");
			Fputs(res->message, stdout);
			Fputs("-----------------\nCommand : ",stdout);
			if (strstr(res->message,"DISCONNECTED") != NULL) {
				break;
			}
			if (res->b == false)
				printf("Your bid is failed!\n");
		}
	}
	*isLoggedOut = true;
	kill(0, SIGTERM); //kill all child processes rudely
	exit(0);
}
