#include	"unp.h"
#include <sys/types.h>
#include <sys/ipc.h>
#include <sys/shm.h>
#include "stdbool.h"
#include "data.c"
#include "string.h"
#include "time.h"

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
	int duration;
	int userId;
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
int* remainingTime;

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
	time_t now;
	ssize_t		n;
	char		buf[MAXLINE],*temp;
	char res_str[MAXLINE*20];
	bool isLoggedIn = false;
	int id = -1;
	int len,i,j;
	float mbid;
	Response* res;
	res = initResponse();
	(*noCli)++;
	/* printf("------%d--------\n",*noCli); */
	while (((n = read(sock, buf, MAXLINE)) > 0 || (n < 0 && errno == EINTR)))
	{
		now = time(NULL);
		resetResponse(res);
		clearString(res_str);
		clearString(res->message);
		switch(toAction(buf)) {
			case AC_LOGIN:
				if((temp = getValOfStr("name",buf)) == NULL)
					goto LINE199;
				id = findConnectedUserByName(temp);
				if (id<0)
					id = addNewConnectedUser(temp);
				if (id >= 0) {
					isLoggedIn = true;
					res->b = true;
					if (*current_product < PRODUCT_NO) {
						sprintf(res_str,"\nCurrent Product is %s\nstart bid: %s\nmin bid: %s\nmax bid: %s\ncurrent bid: %s\nNumber of bidder: %d\nCurrent User: %s\nTime Remaining: %ld\n",
								products[*current_product].name,products[*current_product].start_bid,products[*current_product].min_bid,
								products[*current_product].max_bid, products[*current_product].current_bid,
								cus->length,
								products[*current_product].userId == -1 ? "NULL" : cus->connectedUsers[products[*current_product].userId].name,
								*remainingTime
							   );
					} else {
						sprintf(res_str,"\nDo not have any product to bid!\n");
					}
					strcpy(res->message,res_str);
					clearString(res_str);
				}
				else res->b = false;
				len = responseToString(res,res_str);
				write(sock, res_str, len);
				break;
			case AC_LOGOUT:
				if (id < 0) break;
				printf("%s is disconnected\n",cus->connectedUsers[id].name);
				res->b = true;
				strcpy(res->message,"DISCONNECTED");
				isLoggedIn = false;
				len = responseToString(res,res_str);
				write(sock, res_str, len);
				return;
			case AC_BID:
				if (id < 0) break;
				if((temp = getValOfStr("val",buf)) == NULL)
					goto LINE199;
				if (*current_product < PRODUCT_NO) {
					mbid = atof(temp)- atof(products[*current_product].current_bid);
					if (atof(products[*current_product].min_bid) < mbid && mbid < atof(products[*current_product].max_bid)) {
						strcpy(products[*current_product].current_bid,temp);
						products[*current_product].userId = id;
					}
					res->b = true;
					sprintf(res_str,"\nCurrent Product is %s\nstart bid: %s\nmin bid: %s\nmax bid: %s\ncurrent bid: %s\nNumber of bidder: %d\nCurrent User: %s\nTime Remaining: %ld\n",
							products[*current_product].name,products[*current_product].start_bid,products[*current_product].min_bid,
							products[*current_product].max_bid, products[*current_product].current_bid,
							cus->length,
							products[*current_product].userId == -1 ? "NULL" : cus->connectedUsers[products[*current_product].userId].name,
							*remainingTime
						   );
				} else {
					res->b = true;
					sprintf(res_str,"\nDo not have any product to bid!\n");
				}

				strcpy(res->message,res_str);
				clearString(res_str);
				len = responseToString(res,res_str);
				write(sock, res_str, len);
				break;
			case AC_GET_REMAINING_TIME:
				if (id < 0) break;
				if (*current_product < PRODUCT_NO) {
					sprintf(res_str,"val=\"%d\"",*remainingTime);
				} else {
					sprintf(res_str,"\nDo not have any product to bid!\n");
				}
				res->b = true;
				strcpy(res->message,res_str);
				clearString(res_str);
				len = responseToString(res,res_str);
				write(sock, res_str, len);
				break;
			case AC_GET_CURRENT_PRODUCT:
				if (id < 0) break;
				sprintf(res_str,"val=\"%d\"",*current_product);
				res->b = true;
				strcpy(res->message,res_str);
				clearString(res_str);
				len = responseToString(res,res_str);
				write(sock, res_str, len);
				break;
			case AC_GET_PRODUCT_INFO:
				if (id < 0) break;
				if((temp = getValOfStr("val",buf)) == NULL)
					goto LINE199;
				res->b = true;
				i = atoi(temp);
				if (i < 0 || i >= PRODUCT_NO) {
					sprintf(res_str,"NULL\n");
				} else {
					if (i == *current_product) {
						sprintf(res_str,"\nCurrent Product is %s\nstart bid: %s\nmin bid: %s\nmax bid: %s\ncurrent bid: %s\nNumber of bidder: %d\nCurrent User: %s\nTime Remaining: %ld\n",
								products[*current_product].name,products[*current_product].start_bid,products[*current_product].min_bid,
								products[*current_product].max_bid, products[*current_product].current_bid,
								cus->length,
								products[*current_product].userId == -1 ? "NULL" : cus->connectedUsers[products[*current_product].userId].name,
								*remainingTime
							   );
					} else {
						sprintf(res_str,"\nProduct name: %s\nstart bid: %s\nmin bid: %s\nmax bid: %s\ncurrent bid: %s\nNumber of bidder: %d\nCurrent Winner: %s\n",
								products[i].name,products[i].start_bid,products[i].min_bid,
								products[i].max_bid, products[i].current_bid,
								cus->length,
								products[i].userId == -1 ? "NULL" : cus->connectedUsers[products[i].userId].name
							   );
					}
				}
				strcpy(res->message,res_str);
				clearString(res_str);
				len = responseToString(res,res_str);
				write(sock, res_str, len);
				break;
LINE199:
			default:
				res->b = false;
				strcpy(res->message,"your message is in wrong format");
				len = responseToString(res,res_str);
				write(sock, res_str, len);
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
	strcpy(products[0].min_bid,"4.00");
	strcpy(products[0].max_bid,"100.00");
	strcpy(products[0].current_bid,"10.00");
	products[0].userId = -1;
	products[0].duration = 30;
	*current_product = 0;
	*noCli = 0;

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
	int remainingTime_shm_id = shmget(IPC_PRIVATE,sizeof(int),0600);
	remainingTime = shmat(remainingTime_shm_id,NULL,0);
	time_t now,start;

	int					listenfd, connfd, on;
	pid_t				childpid;
	socklen_t			clilen;
	struct sockaddr_in	cliaddr, servaddr;
	void				sig_chld(int);

	listenfd = Socket(AF_INET, SOCK_STREAM, 0);
	setsockopt( listenfd, SOL_SOCKET, SO_REUSEADDR, &on, sizeof(on) );

	bzero(&servaddr, sizeof(servaddr));
	servaddr.sin_family      = AF_INET;
	servaddr.sin_addr.s_addr = htonl(INADDR_ANY);
	servaddr.sin_port        = htons(6666);

	Bind(listenfd, (SA *) &servaddr, sizeof(servaddr));

	Listen(listenfd, LISTENQ);

	Signal(SIGCHLD, sig_chld);	/* must call waitpid() */

	initBid();
	if ( (childpid = Fork()) == 0) { //time counter
		start = time(NULL);
		*remainingTime = products[*current_product].duration - (now - start);
		while(true){
			if (timeout(1) == 0) { //interval 1 second
				now = time(NULL);
				int rmt = products[*current_product].duration - (now - start);
				if (rmt < 0) { //change to next product
					(*current_product)++;
					start = time(NULL);
				} else {
					*remainingTime = rmt;
				}
				if (*current_product >= PRODUCT_NO)
					exit(0);
			}
		}
	} else {
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
				exit(0);
			}
			Close(connfd);			/* parent closes connected socket */
		}
	}

}
