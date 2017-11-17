/*
 * Offsched Scheduling Class (mapped to the SCHED_OFFSCHED policy)
 */

#define DEBUG

#include <linux/smp.h>
#include <linux/printk.h>
#include <linux/offsched_log.h>

#include "sched.h"

#define offsched_condition true
#define __offsched_log(entry) \
	if (offsched_condition) \
		offsched_log(entry)

void __init init_offsched_rq(struct offsched_rq *offsched_rq)
{
	INIT_LIST_HEAD(&offsched_rq->head);
	offsched_rq->nr_running = 0;
	offsched_rq->active = false;
}

void offsched_begin(int cpu)
{
	struct rq *rq = cpu_rq(cpu);
	struct offsched_rq *offsched_rq = &rq->offsched;

	offsched_rq->active = true;

	add_nr_running(rq, offsched_rq->nr_running);
}
EXPORT_SYMBOL_GPL(offsched_begin);

void offsched_end(int cpu)
{
        struct rq *rq = cpu_rq(cpu);
	struct offsched_rq *offsched_rq = &rq->offsched;

	offsched_rq->active = false;

	sub_nr_running(rq, offsched_rq->nr_running);
}
EXPORT_SYMBOL_GPL(offsched_end);

static inline struct task_struct *task_of_offsched(struct offsched_entity *offsched)
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

static void enqueue_task_offsched(struct rq *rq, struct task_struct *p,
	int flags)
{
	struct offsched_entity *offsched = &p->offsched;
	struct offsched_rq *offsched_rq = &rq->offsched;

	list_add(&offsched->list, &offsched_rq->head);
	offsched_rq->nr_running++;

	if (offsched_rq->active)
		add_nr_running(rq, 1);

	__offsched_log("OFFSCHED_C: enqueue_task()");
}

static void dequeue_task_offsched(struct rq *rq, struct task_struct *p,
	int flags)
{
	struct offsched_entity *offsched = &p->offsched;
	struct offsched_rq *offsched_rq = &rq->offsched;

	list_del(&offsched->list);
	offsched_rq->nr_running--;

	if (offsched_rq->active)
		sub_nr_running(rq, 1);

	__offsched_log("OFFSCHED_C: dequeue_task()");
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
	struct task_struct *next;
	struct offsched_entity *prev_offsched, *next_offsched;
	int cpu = smp_processor_id();

	if (!rq->offsched.active || rq->offsched.nr_running == 0)
		return NULL;

        __offsched_log("OFFSCHED_C: pick_next_task(): begin");

	if (prev->sched_class != &offsched_sched_class) {
		next = offsched_task(cpu);
	} else {
		prev_offsched = &prev->offsched;
		if (list_is_last(&prev_offsched->list, &rq->offsched.head)) {
			next = offsched_task(cpu);
		} else {
			next_offsched = list_next_entry(prev_offsched, list);
			next = task_of_offsched(next_offsched);
		}
	}

        __offsched_log("OFFSCHED_C: pick_next_task(): end");

	put_prev_task(rq, prev);

	return next;
}

static void put_prev_task_offsched(struct rq *rq, struct task_struct *p)
{
}

static int select_task_rq_offsched(struct task_struct *p, int task_cpu,
	int sd_flag, int flags)
{
	int cpu = smp_processor_id();

	return cpu;
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
        __offsched_log("OFFSCHED_C: task_tick()");
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
	.put_prev_task		= &put_prev_task_offsched,		/* Empty */

	.select_task_rq		= &select_task_rq_offsched,

	.set_cpus_allowed	= &set_cpus_allowed_offsched,		/* Empty */

	.rq_online		= &rq_online_offsched,			/* Empty */
	.rq_offline		= &rq_offline_offsched,			/* Empty */

	.set_curr_task		= &set_curr_task_offsched,		/* Empty */
	.task_tick		= &task_tick_offsched,			/* BUG */

	.switched_to		= &switched_to_offsched,		/* Empty */
	.prio_changed		= &prio_changed_offsched,		/* Empty */

	.update_curr		= &update_curr_offsched			/* Empty */
};
