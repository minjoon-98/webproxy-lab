/*
 * echoservert.c - Thread-based echo server
 */
#include "csapp.h"

/*
   에코 함수:
   이 함수는 클라이언트로부터 받은 데이터를 다시 에코합니다.
   매개변수:
     - connfd: 클라이언트와의 연결을 나타내는 파일 디스크립터입니다.
   기능:
     - 바이트 수, 버퍼 및 리오 버퍼 구조체를 선언합니다.
     - 리오 버퍼를 초기화합니다.
     - 연결된 클라이언트로부터 데이터를 읽어와 에코하고, 읽은 바이트 수를 출력합니다.
*/
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

/*
   스레드 함수:
   이 함수는 새로운 스레드에서 실행됩니다.
   매개변수:
     - vargp: void 포인터로 캐스팅된 연결 파일 디스크립터를 받습니다.
*/
/* Thread routine */
void *thread(void *vargp)
{
    int connfd = *((int *)vargp);   // void 포인터를 int 포인터로 캐스팅하고 연결 파일 디스크립터를 가져옵니다.
    Pthread_detach(pthread_self()); // 현재 스레드를 분리합니다.
    Free(vargp);                    // 할당된 메모리를 해제합니다.
    echo(connfd);                   // 연결을 처리하는 echo 함수를 호출합니다.
    Close(connfd);                  // 연결을 닫습니다.
    return NULL;                    // 스레드를 종료하기 위해 NULL을 반환합니다.
}

/*
   메인 함수:
   이 함수는 프로그램의 진입점입니다.
   명령행 인수를 받습니다.
   매개변수:
     - argc: 명령행 인수의 개수입니다.
     - argv: 명령행 인수를 포함하는 문자열 배열입니다.
   기능:
     - 명령행 인수에 지정된 포트에서 듣기 소켓을 엽니다.
     - 들어오는 연결을 수락하기 위해 무한 루프에 진입합니다.
     - 각 연결마다 연결 파일 디스크립터를 저장할 정수형 변수에 메모리를 할당합니다.
     - 각 연결을 처리하기 위해 새로운 스레드를 생성합니다.
*/
int main(int argc, char **argv)
{
    int listenfd, *connfdp;
    socklen_t clientlen;                // 클라이언트 주소 길이를 저장하는 socklen_t 형의 변수를 선언합니다.
    struct sockaddr_storage clientaddr; // 클라이언트 주소를 저장하는 구조체 변수를 선언합니다.
    pthread_t tid;                      // 스레드 ID를 저장하는 pthread_t 변수를 선언합니다.

    if (argc != 2)
    {
        fprintf(stderr, "usage: %s <port>\n", argv[0]);
        exit(0);
    }
    listenfd = Open_listenfd(argv[1]); // 지정된 포트에서 듣기 소켓 파일 디스크립터를 엽니다.

    while (1) // 연결을 수락하기 위한 무한 루프입니다.
    {
        clientlen = sizeof(struct sockaddr_storage);                // 클라이언트 주소의 길이를 설정합니다.
        connfdp = Malloc(sizeof(int));                              // 정수 포인터를 위한 메모리를 할당합니다.
        *connfdp = Accept(listenfd, (SA *)&clientaddr, &clientlen); // 연결을 수락하고 파일 디스크립터를 저장합니다.
        Pthread_create(&tid, NULL, thread, connfdp);                // 새로운 스레드를 만들어 연결을 처리합니다.
    }
}