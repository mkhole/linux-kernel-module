#undef TRACE_SYSTEM
#define TRACE_SYSTEM trace

#if !defined(_TRACE_TRACE_H) || defined(TRACE_HEADER_MULTI_READ)
#define _TRACE_TRACE_H

#include <linux/sched.h>
#include <linux/tracepoint.h>

TRACE_EVENT(trace,

	TP_PROTO(int count, const char *str),

	TP_ARGS(count, str),

	TP_STRUCT__entry(
		__field(int, count)
		__string(str, str)
	),

	TP_fast_assign(
		__entry->count = count;
		__assign_str(str, str);
	),

	TP_printk("count: %d, str: %s", __entry->count, __get_str(str))
);

int trace_reg(void);
void trace_unreg(void);

TRACE_EVENT_FN(trace_fn,

	TP_PROTO(const char *str),

	TP_ARGS(str),

	TP_STRUCT__entry(
		__field(pid_t, pid)
		__dynamic_array(char, comm, TASK_COMM_LEN)
		__string(str, str)
	),

	TP_fast_assign(
		__entry->pid = current->pid;
		memcpy(__get_str(comm), current->comm, TASK_COMM_LEN);
		__assign_str(str, str);
	),

	TP_printk("pid: %d, comm: %s, str: %s", __entry->pid, __get_str(comm),  __get_str(str)),

	trace_reg, trace_unreg
);

DECLARE_EVENT_CLASS(trace_class,

	TP_PROTO(const char *str),

	TP_ARGS(str),

	TP_STRUCT__entry(
		__field(	pid_t,	pid)
		__dynamic_array(char, comm, TASK_COMM_LEN)
		__string(	str,	str)
    ),

	TP_fast_assign(
		__entry->pid	= current->pid;
		memcpy(__get_str(comm), current->comm, TASK_COMM_LEN);
		__assign_str(str, str);
	),

	TP_printk("pid: %d, comm: %s, str: %s", __entry->pid, __get_str(comm),  __get_str(str))
);

DEFINE_EVENT(trace_class, trace_class,
	TP_PROTO(const char *str),
	TP_ARGS(str));

DEFINE_EVENT_FN(trace_class, trace_class_fn,
	TP_PROTO(const char *str),
	TP_ARGS(str),
	trace_reg, trace_unreg);

#endif /* _TRACE_trace_H */

#undef TRACE_INCLUDE_PATH
#define TRACE_INCLUDE_PATH .

#undef TRACE_INCLUDE_FILE
#define TRACE_INCLUDE_FILE trace

#include <trace/define_trace.h>
