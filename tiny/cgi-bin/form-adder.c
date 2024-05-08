/*
 *  form-adder.c - a minimal CGI program that adds two numbers together
 */
/* $begin adder */
#include "csapp.h"

int main(void)
{
    char *buf, *p, *method; // 쿼리 문자열 및 문자열 포인터 선언 /* 11.11 */
    char content[MAXLINE];  // 인수 및 응답 내용을 저장할 버퍼 선언
    int n1 = 0, n2 = 0;     // 두 개의 정수 인수 초기화
                            // arg1[MAXLINE], arg2[MAXLINE],
    /* Extract the two arguments */
    /* QUERY_STRING에서 인수 추출 */
    if ((buf = getenv("QUERY_STRING")) != NULL) /* 쿼리 문자열이 존재하는 경우 */ // ex) /cgi-bin/adder?15000&213 에서 ? 뒤에 있는 인수를 추출해서 buf에 저장
    {
        p = strchr(buf, '&'); /* buf에서 &를 찾고 p가 &를 가르킴 */ // ex) p가 buf = 15000&213에서 &를 가리킴
        *p = '\0'; /* &를 NULL로 바꿔주어 두개의 문자열로 나눔 */   // ex) &를 NULL로 변경 -> 문자열의 끝을 위해 사용 -> 15000 NULL 213으로 문자열이 분리됨
        // strcpy(arg1, buf); /* 첫 번째 인수를 복사 */                         // ex) buf = 15000를 arg1에 복사
        // strcpy(arg2, p + 1); /* 두 번째 인수를 복사 */                       // ex) p = NULL 213을 arg2에 복사
        // n1 = atoi(arg1); /* 첫 번째 문자열을 정수로 변환 array to integer */ // ex) n1 = 15000
        // n2 = atoi(arg2); /* 두 번째 문자열을 정수로 변환 array to integer */ // ex) n2 = 213
        sscanf(buf, "first=%d", &n1);
        sscanf(p + 1, "second=%d", &n2);
    }

    /* 11.11 */
    method = getenv("REQUEST_METHOD");

    /* Make the response body */
    /* sprintf = formatted string output to array */                     // 지정된 형식의 문자열을 생성하여 배열에 저장 //printf와의 차이??? -> 출력 대상이 표준 출력이 아니라 문자열 배열이거나 버퍼이다.
    sprintf(content, "QUERY_STRING=%s", buf);                            // 쿼리 문자열을 응답 내용(content)에 추가
    sprintf(content, "Welcome to add.com: ");                            // 환영 메시지를 응답 내용(content)에 추가
    sprintf(content, "%sTHE Internet addition portal.\r\n<p>", content); // 덧셈 포털 정보를 응답 내용(content)에 추가
    sprintf(content, "%sThe answer is: %d + %d = %d\r\n<p>",             // 덧셈 결과를 응답 내용(content)에 추가
            content, n1, n2, n1 + n2);
    sprintf(content, "%sThanks for visiting!\r\n", content); // 방문객에 대한 감사 메시지를 응답 내용(content)에 추가

    /* Generate the HTTP response */
    /* header */
    printf("Connection: close\r\n");                        // 연결을 닫는 헤더를 출력
    printf("Content-length: %d\r\n", (int)strlen(content)); // 응답 내용(content)의 길이를 출력
    printf("Content-type: text/html\r\n\r\n");              // 콘텐츠 유형을 HTML로 설정하는 헤더를 출력 // 헤더와 바디를 구분하는 빈 줄을 위해 CRLF을 두번 사용함
    /* body */
    // printf("%s", content); // 응답 내용(content)을 출력

    /* 11.11 */
    if (strcasecmp(method, "HEAD") != 0)
        printf("%s", content);

    fflush(stdout); // 출력 버퍼를 비움

    exit(0); // 프로그램 종료
}
/* $end adder */

/*
CRLF는 "Carriage Return" "Line Feed"의 약자
Carriage Return(CR)은 다음 줄로 커서(텍스트 입력 위치)를 옮기지 않고, 현재 줄의 첫 번째 열(좌측 끝)로 이동하는 것을 의미  // ASCII 문자로는 '\r'로 나타낸다
Line Feed(LF)은 줄을 바꿀 때 사용되는 개념으로, 다음 줄로 커서를 이동시키는 것을 의미                                   // ASCII 문자로는 '\n'으로 나타낸다
*/

/*

linux> telnet kittyhawk.cmcl.cs.cmu.edu 8000      // Client: open connection
Trying 128.2.194.242...
Connected to kittyhawk.cmcl.cs.cmu.edu.
Escape character is ’^]’.
GET /cgi-bin/adder?15000&213 HTTP/1.0             // Client: request line
                                                  // Client: empty line terminates headers
HTTP/1.0 200 OK                                   // Server: response line
Server: Tiny Web Server                           // Server: identify server
Content-length: 115                               // Adder: expect 115 bytes in response body
Content-type: text/html                           // Adder: expect HTML in response body
                                                  // Adder: empty line terminates headers
Welcome to add.com: THE Internet addition portal. // Adder: first HTML line
<p>The answer is: 15000 + 213 = 15213             // Adder: second HTML line in response body
<p>Thanks for visiting!                           // Adder: third HTML line in response body
Connection closed by foreign host.                // Server: closes connection
linux>                                            // Client: closes connection and terminates

*/