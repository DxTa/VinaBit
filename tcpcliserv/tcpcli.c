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
	int len;
	Response* res;
	res = initResponse();

	if (argc != 2)
		err_quit("usage: tcpcli <IPaddress>");

	if ((childpid = Fork()) == 0) { //time counter (process con)
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
			if (timeout(1) == 0) {
				if (*isLoggedOut == true)
					exit(0);
				if (*isConnected == false)
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
		}

		//sau khi login sau thi cung in thong tin product hien thoi request dc tu phia server
		do {
			if (timeout(1) == 0) {
				Writen(sockfdtime, "AC_GET_CURRENT_PRODUCT\n", 23);
				clearString(recvline);
				if (read(sockfdtime, recvline, MAXLINE) <= 0)
					errorQuit();
				getResponse(res,recvline);
				temp = getValOfStr("val",res->message);
			}
		} while(temp == NULL);
		current_product = atoi(temp);

		/* time sync with server */
		while(true) {
			// cap nhat remaining time cho phien dau gia hien thoi
			do {
				if (timeout(1) == 0) {
					Writen(sockfdtime, "AC_GET_REMAINING_TIME\n", 22);
					clearString(recvline);
					if (read(sockfdtime, recvline, MAXLINE) <= 0)
						errorQuit();
					getResponse(res,recvline);
					temp = getValOfStr("val",res->message);
				}
			} while(temp == NULL);
			*remainingTime = atoi(temp);
			if (timeout(*remainingTime) == 0) {
				/* update new information */
				//pause while loop for *remainingTime seconds chinh luc pause nay thi la thoi gian cho client bid (o day la bid bang process parent)
				//doan tiep theo nay se cap nhat current_product voi gia tri moi tu server
				//doan do while nay theo ly thuyet thi chay 1 phat la dung luon nhung ma nhet vao do while de phong truong hop su co cac kieu
				do { //to make sure previous product is out dated
					if (timeout(1) == 0) {
						do {
							Writen(sockfdtime, "AC_GET_CURRENT_PRODUCT\n", 23);
							clearString(recvline);
							if (read(sockfdtime, recvline, MAXLINE) <= 0)
								errorQuit();
							getResponse(res,recvline);
							temp = getValOfStr("val",res->message);
						} while(temp == NULL);
					}
				} while (current_product == atoi(temp));
				prev_product = current_product;
				current_product = atoi(temp);

				//doan tiep request thong tin ket qua dau gia cho product hien thoi (dau gia xong cai nay roi)
				do {
					sprintf(sendline,"AC_GET_PRODUCT_INFO val=\"%d\"\n",prev_product);
					Writen(sockfdtime, sendline, strlen(sendline));
					clearString(recvline);
					if (read(sockfdtime, recvline, MAXLINE) <= 0)
						errorQuit();
					getResponse(res,recvline);
					Fputs(res->message, stdout);
				} while (strncmp(res->message,"NULL",strlen("NULL")) == 0 || res->b == false);
				// lay product moi' tu phia server, 2s check 1 lan, check den khi ra product thi thoi
				do {
					if (timeout(2) == 0) {
						printf("Getting new product info ....\n");
						sprintf(sendline,"AC_GET_PRODUCT_INFO val=\"%d\"\n",current_product);
						Writen(sockfdtime, sendline, strlen(sendline));
						clearString(recvline);
						if (read(sockfdtime, recvline, MAXLINE) <= 0)
							errorQuit();
						getResponse(res,recvline);
						if (strncmp(res->message,"NULL",strlen("NULL")) != 0) {
							Fputs(res->message, stdout);
						}
					}
				} while (strncmp(res->message,"NULL",strlen("NULL")) == 0 || res->b == false);
			}
		}

	} else { //day la process parent
		sockfd = Socket(AF_INET, SOCK_STREAM, 0);

		bzero(&servaddr, sizeof(servaddr));
		servaddr.sin_family = AF_INET;
		servaddr.sin_port = htons(6666);
		Inet_pton(AF_INET, argv[1], &servaddr.sin_addr);

		Connect(sockfd, (SA *) &servaddr, sizeof(servaddr));

		/* authenticate */
		while (Fgets(sendline, MAXLINE, stdin) != NULL) {

			Writen(sockfd, sendline, strlen(sendline));

			clearString(recvline);
			if (read(sockfd, recvline, MAXLINE) <= 0)
				errorQuit();
			getResponse(res,recvline);
			Fputs(res->message, stdout);
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
			Fputs(res->message, stdout);
			if (strncmp(res->message,"DISCONNECTED",strlen("DISCONNECTED")) == 0)
				break;
			if (res->b == false)
				printf("Your bid is failed!\n");
		}
	}
	*isLoggedOut = true;
	kill(0, SIGTERM); //kill all child processes rudely
	exit(0);
}
