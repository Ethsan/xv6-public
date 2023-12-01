#include "types.h"
#include "defs.h"
#include "memlayout.h"
#include "mmu.h"
#include "param.h"
#include "proc.h"
#include "spinlock.h"
#include "sleeplock.h"
#include "fs.h"
#include "file.h"
#include "traps.h"
#include "fcntl.h"

#include "syslog.h"

#define NEVENTS 16

#define MIN(a, b) ((a) < (b) ? (a) : (b))

static struct {
  struct spinlock lock;
  int count;
} readers;

static struct {
  struct spinlock lock;
  int r; // read index
  int w; // write index
  struct logev ev[NEVENTS];
} events;

int klogwrite(struct inode *ip, char *buf, int n) {
  struct logev *ev = (void *)buf;
  int nlog = (n + sizeof(struct logev) - 1) / sizeof(struct logev);
  int remain = n;

  if (nlog == 0)
    return 0;

  acquire(&events.lock);
  for (int i = 0; i < nlog; i++) {
    memmove(&events.ev[events.w % NEVENTS], ev, MIN(sizeof(struct logev), remain));

    if (events.w - events.r >= NEVENTS)
      events.r++;

    remain -= sizeof(struct logev);
    ev++;
    wakeup(&events.r);
  }
  release(&events.lock);

  return n;
}

int klogread(struct inode *ip, char *dst, int n) {
  struct logev *ev = (void *)dst;
  int nlog = (n + sizeof(struct logev) - 1) / sizeof(struct logev);
  int remain = n, nread = 0;

  if (nlog == 0)
    return 0;

  acquire(&events.lock);
  while (events.r == events.w) {
    if (myproc()->killed) {
      release(&events.lock);
      return -1;
    }
    sleep(&events.r, &events.lock);
  }

  for (int i = 0; i < nlog; i++) {
    if (events.r == events.w)
      break;

    memmove(ev, &events.ev[events.r % NEVENTS], MIN(sizeof(struct logev), remain));

    nread += MIN(sizeof(struct logev), remain);
    remain -= sizeof(struct logev);
    events.r++;
  }
  release(&events.lock);

  return nread;
}

int klogopen(struct inode *ip, int omode) {
  if (omode | O_RDONLY) {
    acquire(&readers.lock);
    readers.count++;
    release(&readers.lock);
  }

  return 0;
}

void klogclose(struct inode *ip, struct file *f) {
  if (f->readable) {
    acquire(&readers.lock);
    readers.count--;
    release(&readers.lock);
  }
}

void kloginit(void) {
  initlock(&events.lock, "klog");
  initlock(&readers.lock, "readers");
  events.r = events.w = 0;

  devsw[KLOG] = (struct devsw){
      .read = klogread,
      .write = klogwrite,
      .open = klogopen,
      .close = klogclose,
  };
}

char kernet_str[] = "kernel";

int ksyslog(int prio, char *msg) {
  struct logev ev;
  int size;

  acquire(&readers.lock);
  if (readers.count == 0) {
    release(&readers.lock);
    cprintf(msg);
    return 0;
  }

  size = MIN(strlen(msg), sizeof(ev.event) - 1);

  ev.prio = prio;
  memmove(ev.sender, kernet_str, sizeof(kernet_str));
  memmove(ev.event, msg, size);
  ev.event[size] = '\0';

  klogwrite(0, (char *)&ev, sizeof(ev));
  release(&readers.lock);

  return 0;
}
