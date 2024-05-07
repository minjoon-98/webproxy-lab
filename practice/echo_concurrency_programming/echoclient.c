/*
 * echoclient.c - An echo client
 */
/* $begin echoclientmain */
#include "csapp.h"

/* 클라이언트 프로그램 */

int main(int argc, char **argv)
{
    int clientfd;                    // 클라이언트 소켓 파일 디스크립터
    char *host, *port, buf[MAXLINE]; // 호스트, 포트, 버퍼 선언
    rio_t rio;                       // 리오 버퍼 구조체 선언

    // 명령행 인수가 3개가 아닌 경우 사용 방법 출력 후 종료
    /* 0번째 인자로 실행 파일, 1번째로 서버의 호스트네임, 2번째로 서버의 포트 넘버를 받는다.*/ //"usage: <프로그램 이름> <호스트> <포트>"
    if (argc != 3)
    {
        fprintf(stderr, "usage: %s <host> <port>\n", argv[0]);
        exit(0);
    }
    host = argv[1]; // 첫 번째 인수는 호스트
    port = argv[2]; // 두 번째 인수는 포트

    // 서버에 연결
    /* 서버와의 연결 성공(connect까지)해서 클라이언트 소켓 식별자 리턴. */
    clientfd = Open_clientfd(host, port);

    /* 1. 클라이언트 소켓 파일 식별자와 읽기 버퍼 rio를 연결한다.*/
    Rio_readinitb(&rio, clientfd); // 리오 버퍼 초기화

    // 표준 입력에서 읽어와 서버로 전송하고, 서버로부터의 응답을 출력
    /* 표준 입력에서 텍스트 줄을 반복적으로 읽는다. */
    /* 2. 표준 입력sdtin에서 MAXLINE만큼 바이트를 가져와 buf에 저장한다. */
    while (Fgets(buf, MAXLINE, stdin) != NULL)
    { /* 6. EOF 표준 입력을 만나면 종료한다.*/
        /* 3. buf 메모리 안의 strlen(buf) 바이트 만큼의(사실상 모두)를 clientfd로 보낸다. */
        Rio_writen(clientfd, buf, strlen(buf)); // 서버로 데이터 전송
        /* 4. 서버가 buf에 echo줄을 쓰면 그 buf를 읽어서 읽기 버퍼 rio에 쓴다. */
        Rio_readlineb(&rio, buf, MAXLINE); // 서버로부터 응답 읽기
        /* 5. buf에 받아온 값을 표준 출력으로 인쇄한다. */
        Fputs(buf, stdout); // 응답 출력
    }

    Close(clientfd); // 클라이언트 소켓 닫기 /* 루프가 종료되면 클라이언트 식별자를 닫는다. 서버에 EOF 통지가 전송된다. */
    exit(0);         // 클라이언트 종료
}
/* $end echoclientmain */