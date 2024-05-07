/*
 * echoserverp.c - Process-based echo server
 */
/* $begin echoserverpmain */
#include "csapp.h"

/* 서버 프로그램 */

/*
 * echo - read and echo text lines until client closes connection
 */
// 에코 기능 수행 함수
void echo(int connfd)
{
    size_t n;                    // 바이트 수
    char buf[MAXLINE];           // 버퍼
    rio_t rio;                   // 리오 버퍼 구조체
    rio_readinitb(&rio, connfd); // 리오 버퍼 초기화

    // 연결된 클라이언트로부터 데이터를 읽어와 에코하고, 읽은 바이트 수 출력
    while ((n = rio_readlineb(&rio, buf, MAXLINE)) != 0) // 0이면 클라이언트 식별자가 닫혔다는 소리다.
    {
        printf("server received %d bytes\n", (int)n); // 받은 바이트 수 출력
        rio_writen(connfd, buf, n);                   // 클라이언트에게 데이터 전송
    }
}

// SIGCHLD 신호 핸들러: 좀비 프로세스 처리
void sigchld_handler(int sig)
{
    // 모든 종료된 자식 프로세스를 회수
    while (waitpid(-1, 0, WNOHANG) > 0)
        ;   // Reap zombie processes // 좀비 프로세스를 처리하기 위해 대기하고 있는 자식 프로세스들을 모두 회수합니다.
    return; // 핸들러 종료
}

/* 서버의 포트 번호를 1번째 인자로 받는다. */
int main(int argc, char **argv)
{
    int listenfd, connfd; // 소켓 파일 디스크립터
    socklen_t clientlen;  // 클라이언트 주소 길이
    /* Accept로 보내지는 client 소켓 구조체. */
    struct sockaddr_storage clientaddr; // 클라이언트 주소 구조체  /* sockaddr_storage 구조체: 모든 프로토콜의 주소에 대해 Enough room for any addr */

    // 명령행 인수가 2개가 아닌 경우 사용 방법 출력 후 종료
    if (argc != 2)
    {
        fprintf(stderr, "usage: %s <port>\n", argv[0]);
        exit(0);
    }

    // SIGCHLD 신호에 대한 핸들러 등록
    Signal(SIGCHLD, sigchld_handler); // Register SIGCHLD handler to reap zombie processes // 좀비 프로세스를 처리하기 위해 SIGCHLD 핸들러 등록

    // 서버 소켓 생성 및 연결 대기
    listenfd = Open_listenfd(argv[1]); /* 해당 포트 번호에 적합한 듣기 식별자를 만들어 준다. */
    while (1)
    {
        /* 클라이언트의 연결 요청을 계속 받아서 연결 식별자를 만든다. */
        clientlen = sizeof(struct sockaddr_storage);              // 클라이언트 주소 길이 설정 /* Important! 길이가 충분히 커서 프로토콜-독립적!*/
        connfd = Accept(listenfd, (SA *)&clientaddr, &clientlen); // 연결 수락 및 클라이언트 식별자 설정 /* 클라이언트와 통신하는 연결 식별자 */

        if (Fork() == 0) // 자식 프로세스 생성
        {
            Close(listenfd); /* Child closes its listening socket */ // 자식은 듣기 소켓을 닫음
            echo(connfd); /* Child services client */                // 클라이언트 서비스
            Close(connfd); /* Child closes connection with client */ // 클라이언트와의 연결 종료
            exit(0); /* Child exits */                               // 자식 종료
        }
        Close(connfd); /* Parent closes connected socket (important!) */ // 부모는 연결된 소켓을 닫음 (중요)
    }
}
/* $end echoserverpmain */