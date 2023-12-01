#include "types.h"
#include "user.h"

int
main(int argc, char *argv[])
{
  int prio;

  if(argc < 3){
    printf(2, "usage: logerr <prio> <message> ... <message>\n");
    exit();
  }

  prio = atoi (argv [1]);
  openlog ("logger");
  for (int i = 2 ; i < argc ; i++)
      syslog (prio, argv [i]);
  closelog () ;
  exit();
}
