#ifndef KERNEL_H
#define KERNEL_H

#include <defines.h>
#include <system.h>
#include <ts7200.h>
#include <asm/asm.h>
#include <kernel/kernel_task.h>

#ifdef KTEST
#include <user/test/test_task.h>
#endif
#include <user/init/bootstrap.h>

//Data Structures
#include <priority_queue.h>
#include <interrupt_matrix.h>

//Debug - set DEBUG through the gcc option (-D DEBUG)
#include <debug.h>

#include <kernel/handlers/create.h>
#include <kernel/handlers/msg.h>
#include <kernel/handlers/nameserver.h>
#include <kernel/handlers/interrupt.h>

#include <lib/terminal_escape_codes.h>

//Bridges from C to ASM
extern unsigned int kernel_stack_base;
extern unsigned int user_stack_base;

void send_handler(TaskDescriptor *std);
void receive_handler(TaskDescriptor *rtd);

TaskDescriptor tasks[MAX_TASK];
TaskDescriptor *running_task;
TidTracker tid_tracker;
priority_queue pq_tasks;
interrupt_matrix im_tasks;


#endif /* KERNEL_H */
