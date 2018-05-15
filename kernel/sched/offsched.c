/*
 * Offsched Scheduling Class (mapped to the SCHED_OFFSCHED policy)
 */

#include <linux/smp.h>
#include <linux/printk.h>
#include <linux/offsched_log.h>
#include <linux/jiffies.h>
#include <linux/kernel_stat.h>
#include <linux/rcupdate.h>

#include "sched.h"

#define __offsched_raw(str, raw) \
	do { \
		offsched_log_str(str); \
		offsched_log_raw(&raw, sizeof(raw)); \
		offsched_log_nl(); \
	} while (0)
#define __offsched_log(str) \
	do { \
		offsched_log_str(str); \
		offsched_log_nl(); \
	} while (0)

void __init init_offsched_rq(struct offsched_rq *offsched_rq)
{
	INIT_LIST_HEAD(&offsched_rq->head);
	offsched_rq->nr_running = 0;
	offsched_rq->nr_total = 0;
	offsched_rq->active = false;
	offsched_rq->next = NULL;
}

static inline
struct task_struct *task_of_offsched(struct offsched_entity *offsched)
{
	return container_of(offsched, struct task_struct, offsched);
}

struct task_struct *offsched_task(int cpu)
{
        struct rq *rq = cpu_rq(cpu);
	struct offsched_entity *offsched = list_first_entry_or_null(
		&rq->offsched.head, struct offsched_entity, list);

	return offsched ? task_of_offsched(offsched) : NULL;
}
EXPORT_SYMBOL_GPL(offsched_task);

static inline
struct offsched_entity *pick_next_offsched(struct offsched_rq *offsched_rq,
	struct offsched_entity *offsched)
{
	if (!list_is_last(&offsched->list, &offsched_rq->head))
		return list_next_entry(offsched, list);

	return list_first_entry(&offsched_rq->head,
		struct offsched_entity, list);
}

static void enqueue_task_offsched(struct rq *rq, struct task_struct *p,
	int flags)
{
	struct offsched_entity *offsched = &p->offsched;
	struct offsched_rq *offsched_rq = &rq->offsched;

	list_add_tail(&offsched->list, &offsched_rq->head);
	offsched_rq->nr_running++;

	if (unlikely(offsched->cpu != rq->cpu)) {
		offsched->cpu = rq->cpu;
		offsched->jiffies = jiffies;

		offsched_rq->nr_total++;
	}

	if (offsched_rq->active)
		add_nr_running(rq, 1);

	if (offsched_rq->nr_running == 1)
		offsched_rq->next = p;

	__offsched_raw("OFFSCHED_C: enqueue_task(): ", p);
}

static void dequeue_task_offsched(struct rq *rq, struct task_struct *p,
	int flags)
{
	struct offsched_entity *offsched = &p->offsched;
	struct offsched_rq *offsched_rq = &rq->offsched;
	struct offsched_entity *next_offsched;

	if (offsched_rq->next == p) {
		next_offsched = pick_next_offsched(offsched_rq, offsched);
		offsched_rq->next = task_of_offsched(next_offsched);
	}

	list_del(&offsched->list);
	offsched_rq->nr_running--;

	if (offsched_rq->active)
		sub_nr_running(rq, 1);

	if (offsched_rq->nr_running == 0)
		offsched_rq->next = NULL;

	__offsched_raw("OFFSCHED_C: dequeue_task(): ", p);
}

static void yield_task_offsched(struct rq *rq)
{
}

static void check_preempt_curr_offsched (struct rq *rq, struct task_struct *p,
	int flags)
{
}

static struct task_struct *pick_next_task_offsched(struct rq *rq,
	struct task_struct *prev, struct rq_flags *rf)
{
	struct offsched_rq *offsched_rq = &rq->offsched;
	struct task_struct *next = offsched_rq->next;
	struct offsched_entity *next_next_offsched;

	if (!offsched_rq->active)
		return NULL;

	/* check if there are sleeping tasks */
	if (offsched_rq->nr_running != offsched_rq->nr_total)
		sched_ttwu_pending();

	if (next) {
		next_next_offsched = pick_next_offsched(offsched_rq,
			&next->offsched);
		offsched_rq->next = task_of_offsched(next_next_offsched);

		/* account_offsched_time() if prev == next */
		put_prev_task(rq, prev);

		next->offsched.jiffies = jiffies;
	}

	return next;
}

static inline void account_offsched_time(struct task_struct *p)
{
	struct offsched_entity *offsched = &p->offsched;
	u64 cputime = (jiffies - offsched->jiffies) * TICK_NSEC;

	account_user_time(p, cputime);
}

static void put_prev_task_offsched(struct rq *rq, struct task_struct *p)
{
	account_offsched_time(p);
}

static int select_task_rq_offsched(struct task_struct *p, int task_cpu,
	int sd_flag, int flags)
{
	struct offsched_entity *offsched = &p->offsched;

	if (offsched->cpu >= 0)
		return offsched->cpu;

	return task_cpu;
}

static void set_cpus_allowed_offsched(struct task_struct *p,
	const struct cpumask *newmask)
{
}

static void rq_online_offsched(struct rq *rq)
{
}

static void rq_offline_offsched(struct rq *rq)
{
}

static void set_curr_task_offsched(struct rq *rq)
{
}

static void task_tick_offsched(struct rq *rq, struct task_struct *p,
	int queued)
{
	BUG_ON(cpu_offsched(rq->cpu));
}

static void task_dead_offsched(struct task_struct *p)
{
	struct offsched_entity *offsched = &p->offsched;
	struct rq *rq = cpu_rq(offsched->cpu);
	struct offsched_rq *offsched_rq = &rq->offsched;

	offsched_rq->nr_total--;

	__offsched_raw("OFFSCHED_C: task_dead(): ", p);
}

static void switched_to_offsched(struct rq *this_rq, struct task_struct *task)
{
}

static void prio_changed_offsched(struct rq *this_rq, struct task_struct *task,
	int oldprio)
{
}

static void update_curr_offsched(struct rq *rq)
{
}

const struct sched_class offsched_sched_class = {
	.next			= &dl_sched_class,

	.enqueue_task		= &enqueue_task_offsched,
	.dequeue_task		= &dequeue_task_offsched,
	.yield_task		= &yield_task_offsched,			/* Empty */

	.check_preempt_curr	= &check_preempt_curr_offsched,		/* Empty */

	.pick_next_task		= &pick_next_task_offsched,
	.put_prev_task		= &put_prev_task_offsched,

	.select_task_rq		= &select_task_rq_offsched,

	.set_cpus_allowed	= &set_cpus_allowed_offsched,		/* Empty */

	.rq_online		= &rq_online_offsched,			/* Empty */
	.rq_offline		= &rq_offline_offsched,			/* Empty */

	.set_curr_task		= &set_curr_task_offsched,		/* Empty */
	.task_tick		= &task_tick_offsched,			/* BUG */
	.task_dead		= &task_dead_offsched,

	.switched_to		= &switched_to_offsched,		/* Empty */
	.prio_changed		= &prio_changed_offsched,		/* Empty */

	.update_curr		= &update_curr_offsched			/* Empty */
};

void offsched_begin(void)
{
	struct rq *rq = cpu_rq(smp_processor_id());
	struct offsched_rq *offsched_rq = &rq->offsched;

	offsched_rq->active = true;

	add_nr_running(rq, offsched_rq->nr_running);

	__offsched_log("OFFSCHED_C: begin");
}
EXPORT_SYMBOL(offsched_begin);

void offsched_end(void)
{
	struct rq *rq = cpu_rq(smp_processor_id());
	struct offsched_rq *offsched_rq = &rq->offsched;

	offsched_rq->active = false;

	sub_nr_running(rq, offsched_rq->nr_running);

	__offsched_log("OFFSCHED_C: end");
}
EXPORT_SYMBOL(offsched_end);

void offsched_idle(void)
{
	struct rq *rq = cpu_rq(smp_processor_id());
	struct offsched_rq *offsched_rq = &rq->offsched;
	int i;

	while (offsched_rq->nr_total > 0) {
		sched_ttwu_pending();
		schedule();

		for (i = 0; i < 1000000; i++) {
		}
	}
}
EXPORT_SYMBOL(offsched_idle);
