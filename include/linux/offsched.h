#ifndef _LINUX_OFFSCHED_H
#define _LINUX_OFFSCHED_H

extern int register_offsched_callback(void (*offsched_callback)(void),
	int cpuid);
extern void unregister_offsched_callback(int cpuid);
extern int is_offsched_callback(int cpuid);
extern void run_offsched_callback(void);

extern void offsched_begin(void);
extern void offsched_end(void);
extern void offsched_idle(void);

#endif /* _LINUX_OFFSCHED_H */
