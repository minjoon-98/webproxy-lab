/* $begin tinymain */
/*
 * tiny.c - A simple, iterative HTTP/1.0 Web server that uses the
 *     GET method to serve static and dynamic content.
 *
 * Updated 11/2019 droh
 *   - Fixed sprintf() aliasing issue in serve_static(), and clienterror().
 */
#include "csapp.h"

/*
 * doit - Handle client request
 *
 * Handle a client request by parsing the request line, reading and
 * processing the request headers, parsing the URI, serving static
 * or dynamic content based on the request, and sending the response
 * to the client.
 *
 * Parameters:
 *   fd: File descriptor of the client connection
 *
 * Returns:
 *   None
 */
void doit(int fd);

/*
 * read_requesthdrs - Read and process HTTP request headers
 *
 * Read and process the HTTP request headers from the client.
 * This function is responsible for reading the request headers
 * from the client connection and processing them as needed.
 *
 * Parameters:
 *   rp: Pointer to a rio_t structure for reading from the client connection
 *
 * Returns:
 *   None
 */
void read_requesthdrs(rio_t *rp);

/*
 * parse_uri - Parse URI into filename and CGI args
 *
 * Given a URI, parse_uri extracts the filename and CGI arguments (if any)
 * and stores them in the provided filename and cgiargs buffers.
 *
 * Parameters:
 *   uri: The URI to parse
 *   filename: Buffer to store the extracted filename
 *   cgiargs: Buffer to store the extracted CGI arguments
 *
 * Returns:
 *   -1 if the URI is invalid or contains errors, 0 otherwise
 */
int parse_uri(char *uri, char *filename, char *cgiargs);

/*
 * serve_static - Serve a static content file to the client
 *
 * Serve a static content file specified by filename to the client
 * using the provided file descriptor. This function reads the
 * contents of the file and sends it to the client using the given
 * file descriptor.
 *
 * Parameters:
 *   fd: File descriptor of the client connection
 *   filename: Name of the static content file to serve
 *   filesize: Size of the file to serve
 *
 * Returns:
 *   None
 */
void serve_static(int fd, char *filename, int filesize);

/*
 * get_filetype - Determine the content type of a file based on its extension
 *
 * Determine the content type of the file specified by filename based on
 * its extension. The determined content type is stored in the filetype buffer.
 *
 * Parameters:
 *   filename: Name of the file to determine the content type for
 *   filetype: Buffer to store the determined content type
 *
 * Returns:
 *   None
 */
void get_filetype(char *filename, char *filetype);

/*
 * serve_dynamic - Serve dynamic content to the client
 *
 * Serve dynamic content specified by filename and CGI arguments to the client
 * using the provided file descriptor. This function executes the specified
 * dynamic content program and sends its output to the client using the given
 * file descriptor.
 *
 * Parameters:
 *   fd: File descriptor of the client connection
 *   filename: Name of the dynamic content program to execute
 *   cgiargs: Arguments to pass to the dynamic content program
 *
 * Returns:
 *   None
 */
void serve_dynamic(int fd, char *filename, char *cgiargs);

/*
 * clienterror - Send an error message to the client
 *
 * Send an error message with the specified cause, error number, short message,
 * and long message to the client using the provided file descriptor.
 *
 * Parameters:
 *   fd: File descriptor of the client connection
 *   cause: Cause of the error
 *   errnum: Error number
 *   shortmsg: Short error message
 *   longmsg: Long error message
 *
 * Returns:
 *   None
 */
void clienterror(int fd, char *cause, char *errnum, char *shortmsg, char *longmsg);

int main(int argc, char **argv)
{
  int listenfd, connfd;
  char hostname[MAXLINE], port[MAXLINE];
  socklen_t clientlen;
  struct sockaddr_storage clientaddr;

  /* Check command line args */
  if (argc != 2)
  {
    fprintf(stderr, "usage: %s <port>\n", argv[0]);
    exit(1);
  }

  listenfd = Open_listenfd(argv[1]);
  while (1) // 무한 루프 시작
  {
    clientlen = sizeof(clientaddr);
    connfd = Accept(listenfd, (SA *)&clientaddr, &clientlen);                       // 클라이언트로부터 연결을 수락하고 연결 소켓 생성 // 무한 루프 내에서 반복적으로 연결 요청을 접수
    Getnameinfo((SA *)&clientaddr, clientlen, hostname, MAXLINE, port, MAXLINE, 0); // 클라이언트의 호스트명과 포트 번호 가져오기
    printf("Accepted connection from (%s, %s)\n", hostname, port);                  // 클라이언트 정보 출력
    doit(connfd);                                                                   // 요청 처리 함수 호출 // 트랜잭션을 수행
    Close(connfd);                                                                  // 연결 소켓 닫기
  }
}

void doit(int fd) // 한 개의 HTTP 트랜잭션을 처리한다
{
  int is_static;                                                      // 정적 컨텐츠 여부를 나타내는 변수
  struct stat sbuf;                                                   // 파일 정보를 저장할 구조체
  char buf[MAXLINE], method[MAXLINE], uri[MAXLINE], version[MAXLINE]; // 버퍼 및 요청 정보를 저장할 변수들
  char filename[MAXLINE], cgiargs[MAXLINE];                           // 파일 경로 및 CGI 인자를 저장할 변수들
  rio_t rio;                                                          // Rio 버퍼 구조체

  /* Read request line and headers */ /* 요청 라인 및 헤더 읽기 */
  Rio_readinitb(&rio, fd);            // Rio 버퍼 초기화
  Rio_readlineb(&rio, buf, MAXLINE);  // 요청 라인 읽기
  printf("Request headers:\n");
  printf("%s", buf);                             // 읽은 요청 헤더 출력
  sscanf(buf, "%s %s %s", method, uri, version); // 요청 라인 파싱

  if (strcasecmp(method, "GET")) // GET 이외의 메소드는 에러 메세지를 띄운다 (0이 아닌값은 참으로 간주하기 때문)
  {
    clienterror(fd, method, "501", "Not implemented", "Tiny does not implement this method");
    return;
  }
  read_requesthdrs(&rio); // 요청 헤더 읽기

  /* Parse URI from GET request */               /* GET 요청으로부터 URI 파싱 */
  is_static = parse_uri(uri, filename, cgiargs); // URI 파싱
  if (stat(filename, &sbuf) < 0)                 // 파일 정보 읽기
  {
    clienterror(fd, filename, "404", "Not found", "Tiny couldn't find this file");
    return;
  }

  if (is_static) /* Serve static content */                    /* 정적 컨텐츠 제공 */
  {                                                            // 파일이 일반 파일인지 확인 || 파일 접근 권한 확인
    if (!(S_ISREG(sbuf.st_mode)) || !(S_IRUSR & sbuf.st_mode)) // S_ISREG 파일이 일반 파일이면 1 아니면 0반환, S_IRUSR 사용자의 읽기 권한을 나타내는 flag
    {
      clienterror(fd, filename, "403", "Forbidden", "Tiny couldn't read the file");
      return;
    }
    serve_static(fd, filename, sbuf.st_size); // 정적 컨텐츠 서비스
  }
  else /* Serve dynamic content */ /* 동적 컨텐츠 제공 */
  {
    if (!(S_ISREG(sbuf.st_mode)) || !(S_IXUSR & sbuf.st_mode)) // S_ISREG 파일이 일반 파일이면 1 아니면 0반환, S_IXUSR 사용자의 실행 권한을 나타내는 flag
    {
      clienterror(fd, filename, "403", "Forbidden", "Tiny couldn't run the CGI program");
      return;
    }
    serve_dynamic(fd, filename, cgiargs); // 동적 컨텐츠 서비스
  }
}

// 클라이언트한테 에러 메세지 전송
void clienterror(int fd, char *cause, char *errnum, char *shortmsg, char *longmsg)
{
  char buf[MAXLINE], body[MAXBUF];

  /* Build the HTTP response body */ /* HTTP 응답 본문 작성 */
  sprintf(body, "<html><title>Tiny Error</title>");
  sprintf(body, "%s<body bgcolor="
                "ffffff"
                ">\r\n",
          body);
  sprintf(body, "%s%s: %s\r\n", body, errnum, shortmsg);
  sprintf(body, "%s<p>%s: %s\r\n", body, longmsg, cause);
  sprintf(body, "%s<hr><em>The Tiny Web server</em>\r\n", body);

  /* Print the HTTP response */ /* HTTP 응답 전송 */
  sprintf(buf, "HTTP/1.0 %s %s\r\n", errnum, shortmsg);
  Rio_writen(fd, buf, strlen(buf));
  sprintf(buf, "Content-type: text/html\r\n");
  Rio_writen(fd, buf, strlen(buf));
  sprintf(buf, "Content-length: %d\r\n\r\n", (int)strlen(body));
  Rio_writen(fd, buf, strlen(buf));
  Rio_writen(fd, body, strlen(body));
}

void read_requesthdrs(rio_t *rp)
{
  char buf[MAXLINE];

  // 빈 줄이 나올 때까지 요청 헤더의 각 줄을 읽음
  Rio_readlineb(rp, buf, MAXLINE);
  while (strcmp(buf, "\r\n"))
  {
    Rio_readlineb(rp, buf, MAXLINE);
    printf("%s", buf); // 각 헤더 줄을 출력
  }
  return;
}

int parse_uri(char *uri, char *filename, char *cgiargs)
{
  char *ptr;

  // URI에 "cgi-bin"이 포함되어 있는지 확인하여 동적 콘텐츠인지 판단
  if (!strstr(uri, "cgi-bin")) /* Static content */ /* 정적 콘텐츠 */
  {
    // 정적 콘텐츠에는 CGI 인자가 없음을 나타내는 복사
    strcpy(cgiargs, "");
    // 파일 이름을 요청된 URI로 설정
    strcpy(filename, ".");
    strcat(filename, uri);
    // URI가 '/'로 끝나면 기본 페이지를 제공하기 위해 "home.html" 추가
    if (uri[strlen(uri) - 1] == '/')
      strcat(filename, "home.html");
    return 1; // 정적 콘텐츠임을 나타내는 1 반환
  }
  else /* Dynamic content */ /* 동적 콘텐츠 */
  {
    // URI에서 '?'의 위치를 찾아 파일 이름과 CGI 인자를 분리
    ptr = index(uri, '?');
    if (ptr)
    {
      // URI에서 '?' 다음에 나오는 CGI 인자를 복사 , arg1&arg2&...
      strcpy(cgiargs, ptr + 1);
      *ptr = '\0';
    }
    else
      strcpy(cgiargs, ""); // CGI 인자 없음
    // 파일 이름을 요청된 URI로 설정
    strcpy(filename, ".");
    strcat(filename, uri);
    return 0; // 동적 콘텐츠임을 나타내는 0 반환
  }
}

void serve_static(int fd, char *filename, int filesize) // 동적 컨텐츠 출력
{
  int srcfd;
  char *srcp, filetype[MAXLINE], buf[MAXBUF];

  /* Send response headers to client */
  get_filetype(filename, filetype);
  sprintf(buf, "HTTP/1.0 200 OK\r\n");
  sprintf(buf, "%sServer: Tiny Web Server\r\n", buf);
  sprintf(buf, "%sConnection: close\r\n", buf);
  sprintf(buf, "%sContent-length: %d\r\n", buf, filesize);
  sprintf(buf, "%sContent-type: %s\r\n\r\n", buf, filetype);
  Rio_writen(fd, buf, strlen(buf));
  printf("Response headers:\n");
  printf("%s", buf);

  /* Send response body to client */
  srcfd = Open(filename, O_RDONLY, 0);
  srcp = Mmap(0, filesize, PROT_READ, MAP_PRIVATE, srcfd, 0);
  Close(srcfd);
  Rio_writen(fd, srcp, filesize);
  Munmap(srcp, filesize);
}

/*
 * get_filetype - Derive file type from filename
 */
void get_filetype(char *filename, char *filetype) // MIME 타입 확인 후 반환
{
  if (strstr(filename, ".html"))
    strcpy(filetype, "text/html");
  else if (strstr(filename, ".gif"))
    strcpy(filetype, "image/gif");
  else if (strstr(filename, ".png"))
    strcpy(filetype, "image/png");
  else if (strstr(filename, ".jpg"))
    strcpy(filetype, "image/jpeg");
  else
    strcpy(filetype, "text/plain");
}

void serve_dynamic(int fd, char *filename, char *cgiargs)
{
  char buf[MAXLINE], *emptylist[] = {NULL};

  /* Return first part of HTTP response */
  sprintf(buf, "HTTP/1.0 200 OK\r\n");
  Rio_writen(fd, buf, strlen(buf));
  sprintf(buf, "Server: Tiny Web Server\r\n");
  Rio_writen(fd, buf, strlen(buf));

  if (Fork() == 0)
  { /* Child */
    /* Real server would set all CGI vars here */
    setenv("QUERY_STRING", cgiargs, 1);
    Dup2(fd, STDOUT_FILENO);              /* Redirect stdout to client */
    Execve(filename, emptylist, environ); /* Run CGI program */
  }
  Wait(NULL); /* Parent waits for and reaps child */
}

/*
 * strcmp - 두 문자열을 비교하는 함수
 *
 * 두 문자열을 사전적 순서에 따라 비교하여 같으면 0을 반환합니다.
 * 첫 번째 문자열이 두 번째 문자열보다 사전적으로 앞에 있으면 음수를 반환하고,
 * 첫 번째 문자열이 두 번째 문자열보다 사전적으로 뒤에 있으면 양수를 반환합니다.
 *
 * Parameters:
 *   - s1: 첫 번째 비교할 문자열
 *   - s2: 두 번째 비교할 문자열
 *
 * Returns:
 *   - 두 문자열이 같으면 0을 반환
 *   - s1이 s2보다 사전적으로 앞에 있으면 음수를 반환
 *   - s1이 s2보다 사전적으로 뒤에 있으면 양수를 반환
 *
 * int strcmp(const char *s1, const char *s2);
 */

/*
 * strcasecmp - 대소문자 구분 없이 두 문자열을 비교하는 함수
 *
 * 문자열을 대소문자 구분 없이 비교하여 같으면 0을 반환합니다.
 * 비교는 ASCII 문자 순서로 이루어집니다.
 *
 * Parameters:
 *   - s1: 첫 번째 비교할 문자열
 *   - s2: 두 번째 비교할 문자열
 *
 * Returns:
 *   - 두 문자열이 같으면 0을 반환
 *   - s1이 s2보다 사전적으로 앞에 있으면 음수를 반환
 *   - s1이 s2보다 사전적으로 뒤에 있으면 양수를 반환
 *
 * int strcasecmp(const char *s1, const char *s2);
 */

/*
 * strstr - 문자열에서 부분 문자열을 찾는 함수
 *
 * haystack 문자열에서 needle 부분 문자열이 처음으로 나타나는 위치를 찾아줍니다.
 *
 * Parameters:
 *   - haystack: 대상 문자열
 *   - needle: 찾고자 하는 부분 문자열
 *
 * Returns:
 *   - needle이 haystack에 포함되어 있을 경우, needle 부분 문자열이 시작하는 위치를 가리키는 포인터 반환
 *   - needle이 haystack에 포함되어 있지 않을 경우, NULL 반환
 *
 * char *strstr(const char *haystack, const char *needle);
 */

/*
 * strcpy - 문자열을 다른 문자열로 복사하는 함수
 *
 * 하나의 문자열을 다른 문자열로 복사하여 반환합니다.
 * 복사 대상 문자열의 끝에 NULL 종료 문자('\0')가 있을 때까지 복사를 수행합니다.
 *
 * Parameters:
 *   - dest: 복사 대상이 될 문자열의 주소
 *   - src: 복사될 문자열의 주소
 *
 * Returns:
 *   - 복사된 문자열의 시작 주소를 반환
 *
 * char *strcpy(char *dest, const char *src);
 */

/*
 * strcat - 문자열을 이어붙이는 함수
 *
 * 첫 번째 문자열 끝에 두 번째 문자열을 이어붙입니다.
 * 결과 문자열은 첫 번째 문자열에 반영되며, 첫 번째 문자열의 끝에 NULL 종료 문자('\0')가 추가됩니다.
 *
 * Parameters:
 *   - dest: 이어붙일 대상이 될 문자열의 포인터
 *   - src: 이어붙일 문자열의 포인터
 *
 * Returns:
 *   - 이어붙인 결과 문자열의 시작 주소를 반환
 *
 * char *strcat(char *dest, const char *src);
 */

/*
 * mmap - 파일이나 장치를 메모리에 매핑하는 함수
 *
 * 주어진 파일이나 장치를 메모리에 매핑하여 파일의 내용에 직접 접근할 수 있게 합니다.
 *
 * Parameters:
 *   - addr: 메모리 매핑을 시작할 주소. 보통 0을 전달하여 커널이 알아서 적절한 주소를 선택하도록 합니다.
 *   - length: 매핑할 영역의 크기입니다. 파일의 전체 크기를 매핑하려면 파일의 크기를 전달합니다.
 *   - prot: 메모리 보호 옵션으로 매핑된 메모리에 대한 접근 권한을 설정합니다.
 *   - flags: 메모리 매핑 옵션으로 주로 MAP_SHARED 또는 MAP_PRIVATE 등이 사용됩니다.
 *   - fd: 매핑할 파일을 가리키는 파일 디스크립터입니다.
 *   - offset: 파일 내에서 매핑을 시작할 오프셋을 지정합니다. 매핑할 파일 내의 오프셋으로 보통 0을 사용하여 파일의 처음부터 매핑합니다.
 *
 * Returns:
 *   - 메모리 매핑된 영역의 시작 주소를 반환하거나 오류가 발생하면 MAP_FAILED를 반환합니다.
 *
 * void *mmap(void *addr, size_t length, int prot, int flags, int fd, off_t offset);
 */

/*
 * munmap - 메모리 매핑된 영역을 해제하는 함수
 *
 * 메모리에 매핑된 영역을 해제하여 해당 영역이 프로세스의 가용 메모리에서 제거되고,
 * 해당 영역에 대한 자원이 반환됩니다.
 *
 * Parameters:
 *   - addr: 메모리 매핑된 영역의 시작 주소
 *   - length: 메모리 매핑된 영역의 크기
 *
 * Returns:
 *   - 성공하면 0을 반환하고, 실패하면 -1을 반환합니다.
 *
 * int munmap(void *addr, size_t length);
 */

/*
 * setenv - 환경 변수를 설정하는 함수
 *
 * 현재 프로세스의 환경 변수를 설정합니다.
 *
 * Parameters:
 *   - name: 설정할 환경 변수의 이름
 *   - value: 환경 변수에 설정할 값
 *   - overwrite: 기존에 동일한 이름의 환경 변수가 존재하는 경우 값을 덮어쓸지 여부를 결정하는 플래그
 *
 * Returns:
 *   - 성공하면 0을 반환하고, 실패하면 -1을 반환합니다. 실패할 경우 errno 변수에 오류 코드가 설정됩니다.
 *
 * int setenv(const char *name, const char *value, int overwrite);
 */

/*
 * dup2 - 파일 디스크립터를 복제하는 함수
 *
 * oldfd 파일 디스크립터를 newfd 파일 디스크립터로 복제합니다.
 * 만약 newfd가 이미 열려있는 파일을 가리키고 있다면, 그 파일을 닫고 oldfd가 가리키는 파일을 가리키도록 합니다.
 *
 * Parameters:
 *   - oldfd: 복제할 파일 디스크립터
 *   - newfd: 복제된 파일 디스크립터가 될 파일 디스크립터
 *
 * Returns:
 *   - 성공하면 새로운 파일 디스크립터를 반환하고, 실패하면 -1을 반환합니다.
 *     실패할 경우 errno 변수에 오류 코드가 설정됩니다.
 *
 * int dup2(int oldfd, int newfd);
 */

/*
 * execve - 새로운 프로그램을 실행하는 함수
 *
 * 현재 프로세스의 메모리 공간과 상태를 새로운 프로그램으로 대체하여 새로운 프로그램을 실행합니다.
 *
 * Parameters:
 *   - filename: 실행할 프로그램의 경로 및 파일명
 *   - argv: 실행될 프로그램에 전달될 명령행 인수들을 담고 있는 문자열 배열. 배열의 마지막 요소는 NULL이어야 합니다.
 *   - envp: 실행될 프로그램에 전달될 환경 변수들을 담고 있는 문자열 배열. 이 배열은 NULL로 종료되어야 합니다.
 *           만약 envp가 NULL이면 현재 프로세스의 환경 변수가 그대로 전달됩니다.
 *
 * Returns:
 *   - 성공하면 호출되지 않고 새로운 프로그램이 실행됩니다. 호출되지 않은 경우 현재 프로세스의 상태는 그대로 유지됩니다.
 *     실패하면 -1을 반환하고, errno 변수에 오류 코드가 설정됩니다.
 *
 * int execve(const char *filename, char *const argv[], char *const envp[]);
 */

/*
 * wait - 자식 프로세스가 종료될 때까지 부모 프로세스를 대기시키는 함수
 *
 * 부모 프로세스는 wait() 함수를 호출하여 자식 프로세스의 종료를 기다릴 수 있습니다.
 * 만약 자식 프로세스가 이미 종료되었다면, wait() 함수는 즉시 반환합니다.
 *
 * Parameters:
 *   - status: 자식 프로세스의 종료 상태를 나타내는 변수의 주소. 부모 프로세스는 이 변수를 통해 자식 프로세스의 종료 상태를 확인할 수 있습니다.
 *             만약 이 변수가 NULL이라면 부모 프로세스는 자식 프로세스의 종료 상태를 알 수 없습니다.
 *             정상적으로 종료했는지, 시그널에 의해 종료됐는지에 따라서 어떻게 종료됐는지에 대한 정보를 확인할 수 있음
 *
 * Returns:
 *   - 성공하면 종료한 자식 프로세스의 프로세스 ID를 반환하고, 실패하면 -1을 반환합니다.
 *
 * pid_t wait(int *status);
 */
