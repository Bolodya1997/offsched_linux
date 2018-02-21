#ifndef _LINUX_OFFSCHED_H
#define _LINUX_OFFSCHED_H

extern void set_offsched_dead(int cpu);
extern void clear_offsched_dead(int cpu);
extern int is_offsched_dead(int cpu);

extern int register_offsched_callback(void (*offsched_callback)(void),
	int cpu);
extern void unregister_offsched_callback(int cpu);
extern int is_offsched_callback(int cpu);
extern void run_offsched_callback(void);

#define cpu_offsched(cpu)       (is_offsched_dead(cpu) && is_offsched_callback(cpu))

extern void offsched_begin(int cpu);
extern void offsched_end(int cpu);
extern unsigned int offsched_total(int cpu);

#endif /* _LINUX_OFFSCHED_H */
