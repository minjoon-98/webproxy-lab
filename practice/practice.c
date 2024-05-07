#include <sys/types.h>
#include <sys/socket.h>
#include <netdb.h>
#include <stdio.h>
int main(int argc, char *argv[])
{
    int status;
    struct addrinfo hints;
    struct addrinfo *servinfo;        // 결과를 저장할 변수
    memset(&hints, 0, sizeof(hints)); // hints 구조체의 모든 값을 0으로 초기화
    hints.ai_family = AF_UNSPEC;      // IPv4와 IPv6 상관하지 않고 결과를 모두 받겠다
    hints.ai_socktype = SOCK_STREAM;  // TCP stream sockets

    status = getaddrinfo("www.google.com", "80", &hints, &servinfo);
    printf(status);
}