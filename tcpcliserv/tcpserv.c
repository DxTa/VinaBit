#include	"unp.h"
#include <sys/types.h>
#include <sys/ipc.h>
#include <sys/shm.h>
#include "stdbool.h"
#include "data.c"
#include "string.h"

#define INTERVAL 10
#define MAX_USER 50
#define PRODUCT_NO 1

typedef struct Product {
	char name[50];
	bool isSold;
	char start_bid[50];
	char min_bid[50];
	char max_bid[50];
	char current_bid[50];
} Product;

typedef struct ConnectedUser {
	char name[50];
} ConnectedUser;

typedef struct ConnectedUsers {
	ConnectedUser connectedUsers[MAX_USER];
	int length;
} ConnectedUsers;

ConnectedUsers* cus;
Product* products;
int* current_product;
int* noCli;

int addNewConnectedUser(char name[50]) {
	strcpy(cus->connectedUsers[cus->length].name,name);
	cus->length++;
	return cus->length-1;
}

int findConnectedUserByName(char name[]) {
	int i;
	for (i=0; i<cus->length; i++) {
		if (strcmp(cus->connectedUsers[i].name,name) == 0)
			return i;
	}
	return -1;
}

void clearString(char c[]) {
	int i;
	for (i=0; i<strlen(c); i++)
		c[i] = '\0';
}

void doprocessing (int sock)
{
	ssize_t		n;
	char		buf[MAXLINE],*temp;
	char res_str[MAXLINE*20];
	bool isLoggedIn = false;
	int id = -1;
	int len;
	Response* res;
	res = initResponse();
	(*noCli)++;
	printf("------%d--------\n",*noCli);
	while ( (n = read(sock, buf, MAXLINE)) > 0 || (n < 0 && errno == EINTR))
	{
		resetResponse(res);
		clearString(res_str);
		switch(toAction(buf)) {
			case AC_LOGIN:
				temp = getValOfActionStr("name",buf);
				id = findConnectedUserByName(temp);
				if (id<0)
					id = addNewConnectedUser(temp);
				if (id >= 0) {
					isLoggedIn = true;
					res->b = true;
				}
				else res->b = false;
				len = responseToString(res,res_str);
				write(sock, res_str, len);
				break;
			case AC_LOGOUT:
				printf("%s is disconnected\n",cus->connectedUsers[id].name);
				res->b = true;
				strcpy(res->message,"you are disconected");
				isLoggedIn = false;
				len = responseToString(res,res_str);
				write(sock, res_str, len);
				return;
			case AC_GET_FIRST_INFO:
				res->b = true;
				sprintf(res_str,"\nCurrent Product is %s\nstart bid: %s\nmin bid: %s\nmax bid: %s\ncurrent bid: %s\nNumber of bidder: %d\n",
						products[*current_product].name,products[*current_product].start_bid,products[*current_product].min_bid,
						products[*current_product].max_bid, products[*current_product].current_bid,
						cus->length
						);
				strcpy(res->message,res_str);
				clearString(res_str);
				len = responseToString(res,res_str);
				write(sock, res_str, len);
				break;
			case AC_HEARING:
				write(sock, "aloha\n",6);
				break;
			default:
				break;

		}
	}
	(*noCli)--;
	if (n < 0)
		err_sys("str_echo: read error");
}

int initBid() {
	cus->length = 0;
	strcpy(products[0].name,"fixie");
	products[0].isSold = false;
	strcpy(products[0].start_bid,"10.00");
	strcpy(products[0].min_bid,"10.00");
	strcpy(products[0].max_bid,"100.00");
	strcpy(products[0].current_bid,"10.00");
	*current_product = 0;

	return 0;
}

int main(int argc, char **argv)
{
	/* share memory */
	int cus_shm_id = shmget(IPC_PRIVATE,sizeof(ConnectedUsers),0600);
	cus = shmat(cus_shm_id,NULL,0);
	int products_shm_id = shmget(IPC_PRIVATE,sizeof(Product)*PRODUCT_NO,0600);
	products = shmat(products_shm_id,NULL,0);
	int current_product_shm_id = shmget(IPC_PRIVATE,sizeof(int),0600);
	current_product = shmat(current_product_shm_id,NULL,0);
	int noCli_shm_id = shmget(IPC_PRIVATE,sizeof(int),0600);
	noCli = shmat(noCli_shm_id,NULL,0);

	int					listenfd, connfd;
	pid_t				childpid;
	socklen_t			clilen;
	struct sockaddr_in	cliaddr, servaddr;
	void				sig_chld(int);

	listenfd = Socket(AF_INET, SOCK_STREAM, 0);

	bzero(&servaddr, sizeof(servaddr));
	servaddr.sin_family      = AF_INET;
	servaddr.sin_addr.s_addr = htonl(INADDR_ANY);
	servaddr.sin_port        = htons(SERV_PORT);

	Bind(listenfd, (SA *) &servaddr, sizeof(servaddr));

	Listen(listenfd, LISTENQ);

	Signal(SIGCHLD, sig_chld);	/* must call waitpid() */

	initBid();

	for ( ; ; ) {
		clilen = sizeof(cliaddr);
		if ( (connfd = accept(listenfd, (SA *) &cliaddr, &clilen)) < 0) {
			if (errno == EINTR)
				continue;		/* back to for() */
			else
				err_sys("accept error");
		}
		if ( (childpid = Fork()) == 0) {	/* child process */
			Close(listenfd);	/* close listening socket */
			doprocessing(connfd);	// process the request
			/* printf("%s \n", cus.connectedUsers[cus.length-1].name); */
			exit(0);
		}
		Close(connfd);			/* parent closes connected socket */
	}
}
