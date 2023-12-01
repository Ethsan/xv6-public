// event priorities
#define	LOG_EMERG	0	// system is unusable
#define	LOG_ALERT	1	// action must be taken immediately
#define	LOG_CRIT	2	// critical conditions
#define	LOG_ERR		3	// error conditions
#define	LOG_WARNING	4	// warning conditions
#define	LOG_NOTICE	5	// normal but significant condition
#define	LOG_INFO	6	// informational
#define	LOG_DEBUG	7	// debug-level messages

#define	MAX_SENDER	8	// maximum size of sender
#define	MAX_EVENT	68	// maximum size of event message

#define	LOGDEVICE	"/klog"	// device to use

struct logev {			// 80 bytes (4 + 8 + 68)
  int prio ;			// event priority (see LOG_* definitions)
  char sender [MAX_SENDER] ;	// daemon which emits this event
  char event [MAX_EVENT] ;	// message emitted by the daemon
} ;
