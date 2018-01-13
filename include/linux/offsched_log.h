#ifndef _LINUX_OFFSCHED_LOG_H
#define _LINUX_OFFSCHED_LOG_H

extern void offsched_log_init(void);

extern int offsched_log_str(const char *str);
extern int offsched_log_raw(void *ptr, int bytes);
extern void offsched_log_nl(void);

#endif
