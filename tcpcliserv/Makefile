include ../Make.defines

PROGS =	tcpcli tcpserv

all:	${PROGS}


tcpcli:	tcpcli.o
		${CC} ${CFLAGS} -o $@ tcpcli.o ${LIBS}
tcpserv:	tcpserv.o sigchldwaitpid.o
		${CC} ${CFLAGS} -o $@ tcpserv.o sigchldwaitpid.o ${LIBS}

clean:
		rm -f ${PROGS} ${CLEANFILES}
