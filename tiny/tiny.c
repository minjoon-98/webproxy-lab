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
 * doit - 클라이언트 요청을 처리합니다.
 *
 * 클라이언트 요청을 파싱하고, 요청 헤더를 읽고 처리하며,
 * URI를 파싱하고, 요청에 따라 정적 또는 동적 콘텐츠를 제공하고,
 * 응답을 클라이언트에게 보냅니다.
 *
 * 매개변수:
 *   fd: 클라이언트 연결의 파일 디스크립터
 *
 * 반환값:
 *   없음
 */
void doit(int fd);

/*
 * read_requesthdrs - HTTP 요청 헤더를 읽고 처리합니다.
 *
 * 클라이언트로부터 HTTP 요청 헤더를 읽고 처리합니다.
 * 이 함수는 클라이언트 연결로부터 요청 헤더를 읽어들이고 필요한 처리를 수행합니다.
 *
 * 매개변수:
 *   rp: 클라이언트 연결에서 읽기 위한 rio_t 구조체의 포인터
 *
 * 반환값:
 *   없음
 */
void read_requesthdrs(rio_t *rp);

/*
 * parse_uri - URI를 파일 이름과 CGI 인수로 파싱합니다.
 *
 * 주어진 URI에서 파일 이름과 CGI 인수(있는 경우)를 추출하고,
 * 제공된 파일 이름 및 cgiargs 버퍼에 저장합니다.
 *
 * 매개변수:
 *   uri: 파싱할 URI
 *   filename: 추출된 파일 이름을 저장할 버퍼
 *   cgiargs: 추출된 CGI 인수를 저장할 버퍼
 *
 * 반환값:
 *   URI가 잘못되었거나 오류가 포함되어 있는 경우 -1을 반환하고, 그렇지 않으면 0을 반환합니다.
 */
int parse_uri(char *uri, char *filename, char *cgiargs);

/*
 * serve_static - 정적 콘텐츠 파일을 클라이언트에 제공합니다.
 *
 * 주어진 파일 이름으로 지정된 정적 콘텐츠 파일을 클라이언트에 제공합니다.
 * 이 함수는 파일의 내용을 읽어들이고 주어진 파일 디스크립터를 사용하여 클라이언트에게 보냅니다.
 *
 * 매개변수:
 *   fd: 클라이언트 연결의 파일 디스크립터
 *   filename: 제공할 정적 콘텐츠 파일의 이름
 *   filesize: 제공할 파일의 크기
 *   method: HTTP 요청 메서드 (GET, HEAD) // 11.11
 *
 * 반환값:
 *   없음
 */
void serve_static(int fd, char *filename, int filesize, char *method, char *version);

/*
 * get_filetype - 확장자에 기반하여 파일의 콘텐츠 유형을 결정합니다.
 *
 * 주어진 파일 이름에 있는 확장자를 기반으로 파일의 콘텐츠 유형을 결정합니다.
 * 결정된 콘텐츠 유형은 filetype 버퍼에 저장됩니다.
 *
 * 매개변수:
 *   filename: 콘텐츠 유형을 결정할 파일 이름
 *   filetype: 결정된 콘텐츠 유형을 저장할 버퍼
 *
 * 반환값:
 *   없음
 */
void get_filetype(char *filename, char *filetype);

/*
 * serve_dynamic - 동적 콘텐츠를 클라이언트에 제공합니다.
 *
 * 주어진 파일 이름과 CGI 인수를 사용하여 동적 콘텐츠를 클라이언트에 제공합니다.
 * 이 함수는 지정된 동적 콘텐츠 프로그램을 실행하고 그 출력을 주어진 파일 디스크립터를 사용하여 클라이언트에게 보냅니다.
 *
 * 매개변수:
 *   fd: 클라이언트 연결의 파일 디스크립터
 *   filename: 실행할 동적 콘텐츠 프로그램의 이름
 *   cgiargs: 동적 콘텐츠 프로그램에 전달할 인수
 *   method: HTTP 요청 메서드 (GET, HEAD) // 11.11
 *
 * 반환값:
 *   없음
 */
void serve_dynamic(int fd, char *filename, char *cgiargs, char *method, char *version);

/*
 * clienterror - 클라이언트에게 오류 메시지를 전송합니다.
 *
 * 지정된 원인, 오류 번호, 짧은 메시지 및 긴 메시지로 오류 메시지를 클라이언트에게 전송합니다.
 *
 * 매개변수:
 *   fd: 클라이언트 연결의 파일 디스크립터
 *   cause: 오류의 원인
 *   errnum: 오류 번호
 *   shortmsg: 짧은 오류 메시지
 *   longmsg: 긴 오류 메시지
 *
 * 반환값:
 *   없음
 */
void clienterror(int fd, char *cause, char *errnum, char *shortmsg, char *longmsg, char *version); /* 11.6 C */

/* 11.8 */
/*
 * sigchild_handler - SIGCHLD 시그널을 처리하는 핸들러 함수
 *
 * 이 함수는 SIGCHLD 시그널을 처리하여 자식 프로세스의 종료를 감지하고 처리합니다.
 *
 * Parameters:
 *   - sig: 핸들러가 처리하는 시그널
 */
void sigchild_handler(int sig);

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

  /* 11.8 */                                        // SIGCHLD 시그널을 처리하기 위한 시그널 핸들러를 설정합니다.
  if (Signal(SIGCHLD, sigchild_handler) == SIG_ERR) // 이 핸들러는 자식 프로세스가 종료될 때 발생하는 시그널을 처리합니다.
    unix_error("signal child handler error");       // 만약 핸들러 설정에 실패하면 오류 메시지를 출력합니다.

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

void sigchild_handler(int sig)
{
  // 현재의 errno 값을 저장하여 나중에 복원할 수 있도록 합니다.
  int old_errno = errno;
  int status;
  pid_t pid;

  // 모든 종료된 자식 프로세스를 비동기적으로 대기합니다.
  while ((pid = waitpid(-1, &status, WNOHANG)) > 0)
  {
    // 자식 프로세스가 종료될 때마다 해당 프로세스의 PID를 반환하고 루프를 통해 모든 종료된 자식 프로세스를 대기합니다.
  }

  // 이전에 저장된 errno 값을 복원합니다.
  errno = old_errno;
}

void doit(int fd) // 한 개의 HTTP 트랜잭션을 처리한다
{
  int is_static;                                                      // 정적 컨텐츠 여부를 나타내는 변수
  struct stat sbuf;                                                   // 파일 정보를 저장할 구조체
  char buf[MAXLINE], method[MAXLINE], uri[MAXLINE], version[MAXLINE]; // 버퍼 및 요청 정보를 저장할 변수들
  char filename[MAXLINE], cgiargs[MAXLINE];                           // 파일 경로 및 CGI 인자를 저장할 변수들
  rio_t rio;                                                          // Rio 버퍼 구조체

  /* Read request line and headers */       /* 요청 라인 및 헤더 읽기 */
  Rio_readinitb(&rio, fd);                  // Rio 버퍼 초기화
  if (!(Rio_readlineb(&rio, buf, MAXLINE))) // 요청을 받아오지 못했다면 바로 return하여 doit을 종료
    return;                                 // 무한루프 문제 해결...?
  printf("Request headers:\n");
  printf("%s", buf);                             // 읽은 요청 헤더 출력
  sscanf(buf, "%s %s %s", method, uri, version); // 요청 라인 파싱

  /* 11.11 */
  if (!(strcasecmp(method, "GET") == 0 || strcasecmp(method, "HEAD") == 0)) // GET과 HEAD 메소드만 지원
  {
    clienterror(fd, method, "501", "Not implemented", "Tiny does not implement this method", version); /* 11.6 C */
    return;
  }
  read_requesthdrs(&rio); // 요청 헤더 읽기

  /* Parse URI from GET request */               /* GET 요청으로부터 URI 파싱 */
  is_static = parse_uri(uri, filename, cgiargs); // URI 파싱
  if (stat(filename, &sbuf) < 0)                 // 파일 정보 읽기
  {
    clienterror(fd, filename, "404", "Not found", "Tiny couldn't find this file", version); /* 11.6 C */
    return;
  }

  if (is_static) /* Serve static content */                    /* 정적 컨텐츠 제공 */
  {                                                            // 파일이 일반 파일인지 확인 || 파일 접근 권한 확인
    if (!(S_ISREG(sbuf.st_mode)) || !(S_IRUSR & sbuf.st_mode)) // S_ISREG 파일이 일반 파일이면 1 아니면 0반환, S_IRUSR 사용자의 읽기 권한을 나타내는 flag
    {
      clienterror(fd, filename, "403", "Forbidden", "Tiny couldn't read the file", version); /* 11.6 C */
      return;
    }
    serve_static(fd, filename, sbuf.st_size, method, version); // 정적 컨텐츠 서비스 /* 11.11 */ /* 11.6 C */
  }
  else /* Serve dynamic content */ /* 동적 컨텐츠 제공 */
  {
    if (!(S_ISREG(sbuf.st_mode)) || !(S_IXUSR & sbuf.st_mode)) // S_ISREG 파일이 일반 파일이면 1 아니면 0반환, S_IXUSR 사용자의 실행 권한을 나타내는 flag
    {
      clienterror(fd, filename, "403", "Forbidden", "Tiny couldn't run the CGI program", version); /* 11.6 C */
      return;
    }
    serve_dynamic(fd, filename, cgiargs, method, version); // 동적 컨텐츠 서비스 /* 11.11 */ /* 11.6 C */
  }
}

// 클라이언트한테 에러 메세지 전송
void clienterror(int fd, char *cause, char *errnum, char *shortmsg, char *longmsg, char *version) /* 11.6 C */
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

  /* Print the HTTP response */                            /* HTTP 응답 전송 */
  sprintf(buf, "%s %s %s\r\n", version, errnum, shortmsg); /* 11.6 C */
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

void serve_static(int fd, char *filename, int filesize, char *method, char *version) // 정적 컨텐츠 출력 /* 11.11 *//* 11.6 C */
{
  int srcfd;
  char *srcp, filetype[MAXLINE], buf[MAXBUF];

  /* Send response headers to client */
  get_filetype(filename, filetype);       /* 11.6 C */
  sprintf(buf, "%s 200 OK\r\n", version); /* 11.6 C */
  sprintf(buf, "%sServer: Tiny Web Server\r\n", buf);
  sprintf(buf, "%sConnection: close\r\n", buf);
  sprintf(buf, "%sContent-length: %d\r\n", buf, filesize);
  sprintf(buf, "%sContent-type: %s\r\n\r\n", buf, filetype);
  Rio_writen(fd, buf, strlen(buf));
  printf("Response headers:\n");
  printf("%s", buf);

  /* 11.11 */
  /* 만약 HTTP 요청 메서드가 "HEAD"일 경우, */ // HEAD 메서드일 때, 서버는 실제 리소스 본문을 제외한 응답 헤더만을 전송한다.(메타데이터만을 요청한다.)
  if (strcasecmp(method, "HEAD") == 0)         // 따라서 아래 응답 바디는 출력하지 않고 종료
    return;

  /* Send response body to client */
  srcfd = Open(filename, O_RDONLY, 0); // O_RDONLY 파일을 읽기 전용으로 열려고 할 때 사용하는 플래그
  // srcp = Mmap(0, filesize, PROT_READ, MAP_PRIVATE, rcfsd, 0); // PROT_READ 페이지에 대한 읽기 권한을 허용하는 플래그 // MAP_PRIVATE 매핑된 메모리 영역이 다른 프로세스와 공유되지 않음을 지정하는 플래그
  srcp = (char *)Malloc(filesize);  /* 11.9 */
  Rio_readn(srcfd, srcp, filesize); /* 11.9 */
  Close(srcfd);
  Rio_writen(fd, srcp, filesize);
  // Munmap(srcp, filesize);
  free(srcp); /* 11.9 */
}

/** 11.9
 * mmap() / munmap() 과 malloc() / free()
 *
 * - 파일 접근 방식:
 *    mmap(): 파일을 메모리에 매핑하여 메모리를 파일의 내용으로 채운다. 이 경우 파일의 내용을 직접 메모리에서 읽거나 쓸 수 있다.
 *    malloc(): 메모리를 동적으로 할당하고, 해당 메모리 공간은 파일과 직접적으로 연결되어 있지 않다.
 * - 동작 방식:
 *    mmap(): 파일을 메모리에 매핑하면 파일의 내용이 메모리에 로드된다. 이로 인해 파일의 내용을 읽거나 쓰는 데에는 파일을 직접 참조할 수 있다. 수정된 데이터는 바로 파일에 반영된다.
 *    malloc(): 메모리를 할당하면 빈 메모리 블록이 생성되며, 파일의 내용을 직접 메모리에 로드하지 않는다. 파일의 내용을 읽거나 쓰려면 별도의 파일 I/O 작업이 필요하다.
 * - 용도:
 *    mmap(): 대용량 파일을 메모리에 매핑하여 메모리 매핑 I/O를 수행할 때 유용하다. 특히 대용량 파일을 효율적으로 처리해야 하는 경우나 파일을 수정해야 하는 경우에 사용한다.
 *    malloc(): 작은 크기의 메모리 블록을 할당하고 해제할 때 주로 사용된다. 주로 일반적인 메모리 할당 및 해제 작업에 사용된다.
 * - 메모리 해제:
 *    munmap(): mmap() 함수로 매핑된 메모리 영역을 해제한다.
 *    free(): malloc() 함수로 할당된 메모리 블록을 해제한다.
 */

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
  else if (strstr(filename, ".mpeg")) /* 11.7 */
    strcpy(filetype, "video/mpeg");
  else if (strstr(filename, ".mp4")) /* 11.7 */
    strcpy(filetype, "video/mp4");
  else
    strcpy(filetype, "text/plain");
}

void serve_dynamic(int fd, char *filename, char *cgiargs, char *method, char *version) // 동적 컨텐츠 출력 /* 11.11 */ /* 11.6 C */
{
  char buf[MAXLINE], *emptylist[] = {NULL};

  /* Return first part of HTTP response */
  sprintf(buf, "%s 200 OK\r\n", version); /* 11.6 C */
  Rio_writen(fd, buf, strlen(buf));
  sprintf(buf, "Server: Tiny Web Server\r\n");
  Rio_writen(fd, buf, strlen(buf));

  if (Fork() == 0) /* Child */
  {
    /* Real server would set all CGI vars here */
    setenv("QUERY_STRING", cgiargs, 1);   // QUERY_STRING에 cgi인자를 덮어 씌운다
    setenv("REQUEST_METHOD", method, 1);  // REQUEST_METHOD에 클라이언트가 요청한 method를 덮어 씌운다 /* 11.11 */
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

/*
 * open - 파일을 열거나 생성하는 함수
 *
 * filename 경로에 있는 파일을 열거나 생성합니다.
 *
 * Parameters:
 *   - filename: 열거나 생성할 파일의 경로 및 이름
 *   - flags: 파일을 열거나 생성할 때의 옵션을 지정합니다. 여기에는 파일을 읽기 전용으로 열거나 쓰기 전용으로 열거나 새로운 파일을 생성하는 등의 옵션이 포함됩니다.
 *   - mode: 파일을 생성할 때 사용되는 파일의 권한을 지정합니다. 파일을 열 때는 이 매개변수를 무시합니다.
 *
 * Returns:
 *   - 파일 디스크립터를 반환합니다. 실패할 경우 -1을 반환하고, errno 변수에 오류 코드가 설정됩니다.
 *
 * int open(const char *filename, int flags, mode_t mode);
 */
