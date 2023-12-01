#include "types.h"
#include "stat.h"
#include "fcntl.h"
#include "user.h"
#include "x86.h"
#include "syslog.h"

char*
strcpy(char *s, const char *t)
{
  char *os;

  os = s;
  while((*s++ = *t++) != 0)
    ;
  return os;
}

int
strcmp(const char *p, const char *q)
{
  while(*p && *p == *q)
    p++, q++;
  return (uchar)*p - (uchar)*q;
}

uint
strlen(const char *s)
{
  int n;

  for(n = 0; s[n]; n++)
    ;
  return n;
}

void*
memset(void *dst, int c, uint n)
{
  stosb(dst, c, n);
  return dst;
}

char*
strchr(const char *s, char c)
{
  for(; *s; s++)
    if(*s == c)
      return (char*)s;
  return 0;
}

char*
gets(char *buf, int max)
{
  int i, cc;
  char c;

  for(i=0; i+1 < max; ){
    cc = read(0, &c, 1);
    if(cc < 1)
      break;
    buf[i++] = c;
    if(c == '\n' || c == '\r')
      break;
  }
  buf[i] = '\0';
  return buf;
}

int
stat(const char *n, struct stat *st)
{
  int fd;
  int r;

  fd = open(n, O_RDONLY);
  if(fd < 0)
    return -1;
  r = fstat(fd, st);
  close(fd);
  return r;
}

int
atoi(const char *s)
{
  int n;

  n = 0;
  while('0' <= *s && *s <= '9')
    n = n*10 + *s++ - '0';
  return n;
}

void*
memmove(void *vdst, const void *vsrc, int n)
{
  char *dst;
  const char *src;

  dst = vdst;
  src = vsrc;
  while(n-- > 0)
    *dst++ = *src++;
  return vdst;
}

struct logev logev;
int logfd = -1;

void
openlog(char *sender)
{
  int i = 0;
  while (*sender != '\0' && i < MAX_SENDER)
    logev.sender[i++] = *sender++;
  while (i < MAX_SENDER)
    logev.sender[i++] = '\0' ;		// pad with '\0'
  logfd = open (LOGDEVICE, O_WRONLY);
}

void
closelog(void)
{
  if (logfd != -1)
    close(logfd);
  logfd = -1;
}

void
syslog(int prio, char *msg)
{
  int i = 0;

  if (logfd != -1) {
    logev.prio = prio;
    while (*msg != '\0' && i < MAX_EVENT)
      logev.event [i++] = *msg++;
    while (i < MAX_EVENT)
      logev.event[i++] = '\0' ;		// pad with '\0'
    if (write (logfd, &logev, sizeof logev) != sizeof logev) {
      printf (2, "cannot write to %s\n", LOGDEVICE);
      closelog () ;
    }
  }
  if (logfd == -1) {
    // log not opened: send event to stderr
    while (i < MAX_SENDER && logev.sender[i] != '\0')
      printf(2, "%c", logev.sender[i++]);
    printf(2, ": %s\n", msg);
  }
}
