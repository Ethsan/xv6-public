#include "types.h"
#include "fcntl.h"
#include "user.h"
#include "syslog.h"

#define	NBEV	30		// lecture de 30 événements à la fois

// Les chaînes dans les événements ne sont pas forcément terminées
// par un octet nul. Par exemple, si le nom de l'émetteur fait
// exactement 8 octets alors que le tableau fait 8 caractères.
// Cette fonction affiche donc au plus max octets.
void
printmax(char *s, int max)
{
  int i = 0;
  while (i < max && s[i] != '\0')
    i++;
  write(1, s, i);
}

int
main(int argc, char *argv[])
{
  int min, max;
  int fd;
  struct logev logev [NBEV];
  int nlus;

  if(argc != 3){
    printf(2, "usage: syslogd <min-prio> <max-prio>\n");
    exit();
  }

  min = atoi (argv [1]);
  max = atoi (argv [2]);

  if (! (min <= max)) {
    printf(2, "%d > %d\n", min, max);
    exit();
  }

  fd = open(LOGDEVICE, O_RDONLY);
  if(fd == -1) {
    printf(2, "cannot open %s\n", LOGDEVICE);
    exit();
  }

  // pour éviter de bloquer le terminal en lançant ce programme,
  // on se comporte comme un vrai démon et on lance un fils qui ne
  // s'arrêtera pas. Le père, lui, peut s'arrêter tout de suite
  // après le fork.
  switch (fork()) {
    case -1:
      printf(2, "cannot fork\n");
      exit();
    case 0:
      // on essaye de lire plein d'événements à la fois pour voir
      // si les spécifications sont respectées
      while ((nlus = read (fd, &logev, NBEV * sizeof *logev)) > 0) {
	if (nlus % sizeof *logev != 0) {
	  printf(2, "read invalid number %d of bytes from %s\n", nlus, LOGDEVICE);
	  exit();
	}
	for (int i = 0 ; i < nlus / sizeof *logev ; i++) {
	  // cas spécial : si prio == 0, on s'arrête (ça évite de
	  // faire un kill)
	  if (logev[i].prio == 0)
	    exit();
	  if (logev[i].prio >= min && logev[i].prio <= max) {
	    printmax(logev[i].sender, MAX_SENDER);
	    printmax(": ", 2);
	    printmax(logev[i].event, MAX_EVENT);
	    printmax("\n", 1);
	  }
	}
      }
    default:
      break;
  }
  exit();
}
