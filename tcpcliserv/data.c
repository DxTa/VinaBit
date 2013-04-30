#include <stdlib.h>
#include "stdbool.h"
#include "string.h"
#include "unp.h"
#include "regex.h"
#include "myregex.c"

//response from server
typedef struct Response {
	bool b;
	char header[10][MAXLINE];
	int cnt_header;
	char message[MAXLINE];
} Response;


//message from client in this form
//METHOD key="value"
//ex: AC_LOGIN name="duc"
typedef enum {
	AC_LOGIN,
	AC_LOGOUT,
	AC_BID,
	AC_GET_INFO_OF_CUR_BID,
	AC_GET_FIRST_INFO,
	AC_HEARING,
	AC_UNKNOWN,
} Action;

char* getMessage(Action a) {
	switch(a) {
		case AC_LOGIN:
			return "AC_LOGIN";
		case AC_LOGOUT:
			return "AC_LOGOUT";
		case AC_BID:
			return "AC_BID";
		case AC_GET_INFO_OF_CUR_BID:
			return "AC_GET_INFO_OF_CUR_BID";
		case AC_GET_FIRST_INFO:
			return "AC_GET_FIRST_INFO";
		case AC_HEARING:
			return "AC_HEARING";
		default:
			return NULL;
	}
}

Action toAction(char* s) {
	if (strncmp(s,"AC_LOGIN",strlen("AC_LOGIN")) == 0)
		return AC_LOGIN;
	else if (strncmp(s,"AC_LOGOUT",strlen("AC_LOGOUT")) == 0)
		return AC_LOGOUT;
	else if (strncmp(s,"AC_HEARING",strlen("AC_HEARING")) == 0)
		return AC_HEARING;
	else if (strncmp(s,"AC_BID",strlen("AC_BID")) == 0)
		return AC_BID;
	else if (strncmp(s,"AC_GET_INFO_OF_CUR_BID",strlen("AC_GET_INFO_OF_CUR_BID")) == 0)
		return AC_GET_INFO_OF_CUR_BID;
	else if (strncmp(s,"AC_GET_FIRST_INFO",strlen("AC_GET_FIRST_INFO")) == 0)
		return AC_GET_FIRST_INFO;
	else return AC_UNKNOWN;
}

Response* initResponse() {
	Response *r;
	r = (Response*)malloc(sizeof(Response));
	r->cnt_header = 0;
	return r;
}

void resetResponse(Response* r) {
	int i,j;
	r->b = false;
	for (i=0; i<10; i++)
		for (j=0;j<MAXLINE;j++)
			r->header[i][j] = '\0';
	r->cnt_header = 0;
	for (j=0;j<MAXLINE;j++)
		r->message[j] = '\0';
}

void addHeader(Response* r, char* h) {
	strncpy(r->header[r->cnt_header],h,MAXLINE);
	r->cnt_header += 1;
}

int responseToString(Response* r, char* buf) {
	char header[MAXLINE*10];
	sprintf(header," %s, %s, %s, %s, %s, %s, %s, %s, %s, %s",r->header[0],r->header[1],r->header[2],r->header[3],r->header[4],r->header[5],r->header[6],r->header[7],r->header[8],r->header[9]);
	sprintf(buf,"%s;%s;%s\n",r->b ? "true" : "false",header,r->message);
	return strlen(buf);
}

void getResponse(Response* r,char* s) {
	char *tok;
	int count = 0;
	resetResponse(r);

	tok = strtok(s,",;");
	while(tok != NULL) {
		if (count == 0) {
			if (strcmp(tok,"true") == 0)
				r->b = true;
			else r->b = false;
		} else if (0 < count && count < 10) {
			addHeader(r,strdup(tok));
		} else if (count >= 10) {
			strcpy(r->message,strdup(tok));
		}
		tok=strtok(NULL,",;");
		count++;
	}
	return;
}

char* getValOfActionStr(char* k,char* s) {
	regex_t regex;
	char regex_text[MAXLINE];
	sprintf(regex_text,"%s%s",k,"=\"(.*)\"");
	compile_regex(&regex,regex_text);
	char* rs;
	rs = match_regex(&regex,s);
	regfree(&regex);
	return rs;
}
