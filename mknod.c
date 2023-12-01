#include "types.h"
#include "stat.h"
#include "user.h"

int
main(int argc, char *argv[])
{
  int maj, min;

  if(argc != 4){
    printf(2, "Usage: mknod path major minor\n");
    exit();
  }

  maj = atoi (argv [2]);
  min = atoi (argv [3]);
  if (mknod (argv[1], maj, min) < 0){
    printf(2, "mkdir: %s failed to create\n", argv[1]);
  }

  exit();
}
