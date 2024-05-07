/*
 * echoserveri.c - An iterative echo server
 */
/* $begin echoserverimain */
#include "csapp.h"

/* 서버 프로그램 */

/*
 * echo - read and echo text lines until client closes connection
 */
// 에코 기능 수행 함수
void echo(int connfd)
{
    size_t n;          // 바이트 수
    char buf[MAXLINE]; // 버퍼
    rio_t rio;         // 리오 버퍼 구조체
    /* 읽기 버퍼 rio와 서버의 연결 소켓 식별자를 연결해준다. clientfd도 연결되어 있다. */
    rio_readinitb(&rio, connfd); // 리오 버퍼 초기화

    /* 읽기 버퍼 rio에서 클라이언트가 보낸 데이터를 읽고, rio에 그 데이터를 그대로 쓴다.*/
    /* 읽기 버퍼 rio에서 MAXLINE만큼의 데이터를 읽어 와 buf에 넣는다. */
    // 연결된 클라이언트로부터 데이터를 읽어와 에코하고, 읽은 바이트 수 출력
    while ((n = rio_readlineb(&rio, buf, MAXLINE)) != 0) // 0이면 클라이언트 식별자가 닫혔다는 소리다.
    {
        printf("server received %d bytes\n", (int)n); // 받은 바이트 수 출력
        rio_writen(connfd, buf, n);                   // 클라이언트에게 데이터 전송
        /* buf 안에는 클라이언트가 보낸 데이터 그대로 있다. */
        /* buf 메모리 안의 클라이언트가 보낸 바이트 만큼의(사실상 모두)를 clientfd로 보낸다. */
    }
    /* 클라이언트 식별자가 닫히면 루프 종료 및 함수도 종료. */
}

/* 서버의 포트 번호를 1번째 인자로 받는다. */
int main(int argc, char **argv)
{
    int listenfd, connfd; // 소켓 파일 디스크립터
    socklen_t clientlen;  // 클라이언트 주소 길이
    /* Accept로 보내지는 client 소켓 구조체. */
    struct sockaddr_storage clientaddr;                  // 클라이언트 주소 구조체  /* sockaddr_storage 구조체: 모든 프로토콜의 주소에 대해 Enough room for any addr */
    char client_hostname[MAXLINE], client_port[MAXLINE]; // 클라이언트 호스트 이름, 포트 번호

    // 명령행 인수가 2개가 아닌 경우 사용 방법 출력 후 종료
    if (argc != 2)
    {
        fprintf(stderr, "usage: %s <port>\n", argv[0]);
        exit(0);
    }

    // 서버 소켓 생성 및 연결 대기
    listenfd = Open_listenfd(argv[1]); /* 해당 포트 번호에 적합한 듣기 식별자를 만들어 준다. */
    while (1)
    {
        /* 클라이언트의 연결 요청을 계속 받아서 연결 식별자를 만든다. */
        clientlen = sizeof(struct sockaddr_storage);                                                      /* Important! 길이가 충분히 커서 프로토콜-독립적!*/
        connfd = Accept(listenfd, (SA *)&clientaddr, &clientlen); /* 클라이언트와 통신하는 연결 식별자 */ // 클라이언트 연결 수락

        /* 클라이언트와 제대로 연결됐다는 것을 출력해준다. */
        Getnameinfo((SA *)&clientaddr, clientlen, client_hostname, MAXLINE, client_port, MAXLINE, 0); // 클라이언트 정보 가져오기
        printf("Connected to (%s, %s)\n", client_hostname, client_port);                              // 클라이언트 정보 출력
        /* 클라이언트가 보낸 데이터를 그대로 버퍼에 저장하고 그 데이터를 그대로 쓴다. */
        echo(connfd); // 에코 기능 실행
        /* 클라이언트 식별자가 닫히면 연결 식별자를 닫아준다. */
        Close(connfd); // 클라이언트 소켓 닫기
    }

    exit(0); // 서버 종료
}
/* $end echoserverimain */