include ../Make.defines

PROGS =	tcpcli2 tcpserv2

all:	${PROGS}


tcpcli2:	tcpcli2.o
		${CC} ${CFLAGS} -o $@ tcpcli2.o ${LIBS}
tcpserv2:	tcpserv2.o sigchldwaitpid.o
		${CC} ${CFLAGS} -o $@ tcpserv2.o sigchldwaitpid.o ${LIBS}

clean:
		rm -f ${PROGS} ${CLEANFILES}
