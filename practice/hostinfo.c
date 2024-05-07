#include "csapp.h"

// 도메인 이름과 연관된 IP 주소 반환
int main(int argc, char **argv)
{
    struct addrinfo *p, *listp, hints;
    char buf[MAXLINE];
    int rc, flags;

    /* hint 구조체 세팅 */
    memset(&hints, 0, sizeof(struct addrinfo)); // hint 구조체 초기화
    hints.ai_family = AF_INET;                  /* IPv4 only */
    hints.ai_socktype = SOCK_STREAM;            /* Connections only */

    /*
        getaddrinfo()로 해당 도메인에 대한 소켓 주소의 리스트 addrinfo를 반환한다.
        입력받은 main()의 인자 중 2번째(1번째는 실행파일명)의 도메인 주소로부터,
        서비스는 도메인 이름만을 변환하기 위해 NULL, 힌트 추가, listp 반환
    */
    if ((rc = getaddrinfo(argv[1], NULL, &hints, &listp)) != 0)
    {
        fprintf(stderr, "getaddrinfo error: %s\n", gai_strerror(rc));
        exit(1);
    }

    /*
        addrinfo 리스트 하나하나 살펴보면서 ai_addr, 즉 IP주소를 반환한다.
        domain은 getaddrinfo에서 받았고,
        해당 domain에 대응되는 많은 IP주소들이 addrinfo 리스트에 나온다.
        각각의 IP주소를 domain으로 변환시키지 않고 그냥 출력한다.
    */
    flags = NI_NUMERICHOST; /* 도메인 이름을 리턴하지 않고 10진수 주소 스트링을 대신 리턴한다. */
    for (p = listp; p; p = p->ai_next)
    {
        Getnameinfo(p->ai_addr, p->ai_addrlen, // addrinfo 안의 IP주소(소켓 주소 구조체)를 찾아
                    buf, MAXLINE,              // 호스트 이름. flag를 썼으니 10진수 주소로.
                    NULL, 0,                   // service는 안받아오는듯
                    flags);
        printf("%s\n", buf); // input IP주소를 출력한다.
    }

    /* addrinfo 구조체를 free한다. */
    freeaddrinfo(listp);

    exit(0);
}