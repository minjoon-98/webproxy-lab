/*
 * echoservers.c - Select-based echo server
 */
/* $begin echoserversmain */
#include "csapp.h"

typedef struct
{ /* Represents a pool of connected descriptors */                  /* 연결된 디스크립터들의 풀을 나타냄 */
    int maxfd; /* Largest descriptor in read_set */                 /* read_set에 있는 가장 큰 디스크립터 */
    fd_set read_set; /* Set of all active descriptors */            /* 모든 활성 디스크립터의 집합 */
    fd_set ready_set; /* Subset of descriptors ready for reading */ /* 읽기 준비가 된 디스크립터들의 부분 집합 */
    int nready; /* Number of ready descriptors from select */       /* select에서 준비된 디스크립터 수 */
    int maxi; /* High water index into client array */              /* 클라이언트 배열의 최고 위치 인덱스 */
    int clientfd[FD_SETSIZE]; /* Set of active descriptors */       /* 활성 디스크립터들의 집합 */
    rio_t clientrio[FD_SETSIZE]; /* Set of active read buffers */   /* 활성 읽기 버퍼들의 집합 */
} pool;

int byte_cnt = 0; /* Counts total bytes received by server */ /* 서버가 받은 총 바이트 수 */

/*
 * init_pool 함수는 연결된 디스크립터 풀을 초기화합니다.
 * 이 함수는 연결된 디스크립터의 상태를 추적하고 관리하는 데 사용됩니다.
 *
 * Parameters:
 * - listenfd: 서버가 클라이언트 연결을 수신하는 소켓 디스크립터
 * - p: 연결된 디스크립터 풀을 나타내는 pool 구조체에 대한 포인터
 */
void init_pool(int listenfd, pool *p)
{
    // 처음에는 연결된 디스크립터가 없습니다
    int i;
    p->maxi = -1; // 연결된 디스크립터 배열의 최고 인덱스 초기화
    for (i = 0; i < FD_SETSIZE; i++)
        p->clientfd[i] = -1; // 모든 연결된 디스크립터를 초기화합니다.

    // 처음에는 listenfd가 select의 읽기 집합의 유일한 멤버입니다
    p->maxfd = listenfd;            // 가장 큰 디스크립터는 listenfd입니다.
    FD_ZERO(&p->read_set);          // read_set을 모두 초기화합니다.
    FD_SET(listenfd, &p->read_set); // listenfd를 read_set에 추가합니다.
}

/*
 * add_client 함수는 새로운 클라이언트를 연결된 디스크립터 풀에 추가합니다.
 *
 * Parameters:
 * - connfd: 새로 연결된 클라이언트의 소켓 디스크립터
 * - p: 연결된 디스크립터 풀을 나타내는 pool 구조체에 대한 포인터
 */
void add_client(int connfd, pool *p)
{
    int i;
    p->nready--; // 대기 중인 디스크립터 수를 하나 줄입니다.
    for (i = 0; i < FD_SETSIZE; i++)
    { // 빈 슬롯을 찾습니다.
        if (p->clientfd[i] < 0)
        { // 빈 슬롯을 찾았을 때
            // 연결된 디스크립터를 풀에 추가합니다.
            p->clientfd[i] = connfd;
            // 연결된 디스크립터에 대한 RIO 버퍼를 초기화합니다.
            Rio_readinitb(&p->clientrio[i], connfd);

            // 연결된 디스크립터를 읽기 집합에 추가합니다.
            FD_SET(connfd, &p->read_set);

            // 최대 디스크립터 및 풀의 최고 수준을 업데이트합니다.
            if (connfd > p->maxfd) // 새로운 디스크립터가 현재 최대 디스크립터보다 크면
                p->maxfd = connfd; // 최대 디스크립터를 갱신합니다.
            if (i > p->maxi)       // 현재 슬롯이 최고 인덱스를 초과하면
                p->maxi = i;       // 최고 인덱스를 갱신합니다.
            break;
        }
    }
    if (i == FD_SETSIZE) // 빈 슬롯을 찾지 못한 경우
    {
        app_error("add_client error: Too many clients"); // 오류를 발생시킵니다.
    }
}

/*
 * check_clients 함수는 연결된 클라이언트의 읽기 상태를 확인하고 처리합니다.
 *
 * Parameters:
 * - p: 연결된 디스크립터 풀을 나타내는 pool 구조체에 대한 포인터
 */
void check_clients(pool *p)
{
    int i, connfd, n;
    char buf[MAXLINE];
    rio_t rio;

    // 모든 연결된 디스크립터에 대해 반복합니다.
    for (i = 0; (i <= p->maxi) && (p->nready > 0); i++)
    {
        connfd = p->clientfd[i]; // 현재 인덱스의 연결된 디스크립터를 가져옵니다.
        rio = p->clientrio[i];   // 현재 인덱스의 RIO 버퍼를 가져옵니다.

        // 디스크립터가 준비되었고, 읽기 집합에 해당 디스크립터가 있으면
        if ((connfd > 0) && (FD_ISSET(connfd, &p->ready_set)))
        {
            p->nready--; // 대기 중인 디스크립터 수를 감소시킵니다.

            // 디스크립터로부터 텍스트 라인을 읽어옵니다.
            if ((n = Rio_readlineb(&rio, buf, MAXLINE)) != 0)
            {
                byte_cnt += n; // 받은 바이트 수를 증가시킵니다.
                // 받은 바이트 수와 총 바이트 수를 출력합니다.
                printf("Server received %d (%d total) bytes on fd %d\n", n, byte_cnt, connfd);
                // 읽은 데이터를 다시 클라이언트에게 보냅니다.
                Rio_writen(connfd, buf, n);
            }
            // EOF가 감지되면 디스크립터를 풀에서 제거합니다.
            else
            {
                Close(connfd);                // 디스크립터를 닫습니다.
                FD_CLR(connfd, &p->read_set); // 읽기 집합에서 디스크립터를 제거합니다.
                p->clientfd[i] = -1;          // 풀에서 해당 디스크립터를 제거합니다.
            }
        }
    }
}

int main(int argc, char **argv)
{
    int listenfd, connfd;
    socklen_t clientlen;
    struct sockaddr_storage clientaddr;
    static pool pool; // 연결된 디스크립터들의 풀을 나타내는 pool 구조체 생성

    // 명령행 인수의 개수가 올바른지 확인
    if (argc != 2)
    {
        fprintf(stderr, "usage: %s <port>\n", argv[0]);
        exit(0);
    }
    listenfd = Open_listenfd(argv[1]); // 포트로부터 listenfd를 생성
    init_pool(listenfd, &pool);        // 연결된 디스크립터들의 풀을 초기화

    while (1)
    {
        /* Wait for listening/connected descriptor(s) to become ready */ /* 대기: 수신/연결된 디스크립터가 준비될 때까지 */
        pool.ready_set = pool.read_set;
        pool.nready = Select(pool.maxfd + 1, &pool.ready_set, NULL, NULL, NULL);

        /* If listening descriptor ready, add new client to pool */ /* 대기 중인 디스크립터가 준비되면, 새로운 클라이언트를 풀에 추가 */
        if (FD_ISSET(listenfd, &pool.ready_set))
        {
            clientlen = sizeof(struct sockaddr_storage);
            connfd = Accept(listenfd, (SA *)&clientaddr, &clientlen);
            add_client(connfd, &pool);
        }

        /* Echo a text line from each ready connected descriptor */ /* 준비된 각 연결된 디스크립터로부터 텍스트 라인을 에코 */
        check_clients(&pool);
    }
}
/* $end echoserversmain */