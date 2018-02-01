#include <kernel.h>

/**
 * Saves the old SP to the new kernel stack and sets SP to the kernel stack.
 */
#define KERNEL_INIT() \
  asm( \
       ".extern kernel_stack_base;"  \
       "ldr r1, =kernel_stack_base;" \
       "ldr r1, [r1];"               \
       "sub fp, r1, #4;"             \
       "sub r1, r1, #64;"            \
       "mov r4, sp;"                 \
       "mov sp, r1;"                 \
       "stmfd sp!, {r4, lr};"        \
     );

/**
 * Restores the old SP in order to return to RedBoot.
 */
#define KERNEL_EXIT() \
  asm( \
       ".extern kernel_stack_base;"  \
       "ldr r1, =kernel_stack_base;" \
       "ldr r1, [r1];"               \
       "sub r1, r1, #72;"            \
       "ldmfd r1, {sp, pc};"         \
     );


unsigned int kernel_stack_base = KERNEL_STACK_BASE;
unsigned int user_stack_base = USER_STACK_BASE;

void initialize() {
  SANITY();
  DBLOG_INIT("Initializing", "");
  /*
  asm(
    "ldr r3, =KERNEL_ENTRY;"
    "mov r4, #"STR(KERNEL_ENTRY)";"
    "str r3, [r4];"
  );
  */

  int i;
  for (i = 0; i < MAX_TASK; i++) {
    td_init(&tasks[i]);
  }

  DBLOG_START("init task queue", "");
  tt_init(&tid_tracker);
  pq_init(&pq_tasks);
  DBLOG_S();

  int priority;
  void *task;

#ifdef KTEST
  priority = 0;
  task = &TestTask;
#else
  priority = 3;
#ifdef METRIC_64
  task = &K2InitMetricTask;
#elif METRIC_4
  task = &K2InitMetricTask;
#else
  task=&InitTask;
#endif
#endif

  int tid = tt_get(&tid_tracker);
  TaskDescriptor* volatile td = &tasks[(tid & 0xffff)];

  DBLOG_START("creating task %x", tid);
  ktd_create(td, tid, task, priority, READY, NULL);
  DBLOG_S();
  DBLOG_START("pushing task %x to queue", tid);
  pq_push(&pq_tasks, priority, td);
  DBLOG_S();
}

// Much TODO here
TaskDescriptor* schedule() {
  if (pq_tasks.size == 0)
    return NULL;

  int ret;
  TaskDescriptor *td = NULL;

  ret = pq_dumb_pop(&pq_tasks, &td);
  KASSERT(ret == 0 && td != NULL);

  return td;
}

TaskRequest activate(TaskDescriptor* td) {
  //Store Kernel State
  PUSH_STACK("r0-r12, lr");
  //Push ret val to stack as temp
  asm("mov r8, %0"::"r"(td->ret));
  PUSH_STACK("r8");
  //Install SPSR
  WRITE_SPSR(td->psr);
  //Change to system mode
  SET_CPSR(SYSTEM_MODE);
  //Change the stack pointer to the task's stack (uses fp so no worries)
  WRITE_SP(td->sp);
  //Load instruction after swi (r4) from user stack
  POP_STACK("r4");
  //Change to kernel mode
  SET_CPSR(KERNEL_MODE);
  //Save into kernel lr for loading
  asm("mov lr, r4;");
  //Change to system mode
  SET_CPSR(SYSTEM_MODE);
  //Load the User Trap Frame
  POP_STACK("r0-r12, lr");
  //Switch back to kernel mode
  SET_CPSR(KERNEL_MODE);
  //Set r0 with the new return value from stack
  POP_STACK("r0");
  //Move to the user task
  REVERSE_SWI();

  // TODO: to avoid corruption of r12, could we save user state onto kernel
  //       stack?
  //AFTER USER TASK CALLS SWI (CANT USE FP)
  asm("KERNEL_ENTRY:");
  //Save the r12 to Kernel Stack - Implicitly changed by SET_CPSR
  PUSH_STACK("r12");
  //Change to System mode
  SET_CPSR(SYSTEM_MODE);
  //Save the user state
  PUSH_STACK("r0-r12, lr");
  //Change to Kernel mode
  SET_CPSR(KERNEL_MODE);
  //Save lr to stratch r3
  asm("mov r3, lr");
  //Change to System mode
  SET_CPSR(SYSTEM_MODE);
  //Save the lr(r3)
  PUSH_STACK("r3")
  //Change back to kernel mode
  SET_CPSR(KERNEL_MODE);
  //load the kernel stack (fp is now resuable again!)
  asm("add sp, sp, #4");
  POP_STACK("r0-r12");
  //Change back to system mode
  SET_CPSR(SYSTEM_MODE); //Note we can still use fp!
  //Save the user sp to TaskDescriptor's sp
  READ_SP(td->sp);
  //Change back to kernel mode
  SET_CPSR(KERNEL_MODE);
  //Save the spsr to the TaskDescriptor's psr
  READ_SPSR(td->psr);
  //Load the user's r12 and put it on user stack
  asm("ldr r12, [sp, #-56]");
  asm("str r12, [%0, #48]"::"r"(td->sp));
  // manually put swi arg in r0, avoid overhead of return
  SWI_ARG_FETCH("r0");
  POP_STACK("lr");
}

void create(TaskDescriptor *td) {
  //Get the arguments r0 (priority) r1 (function pointer)
  int tid = tt_get(&tid_tracker);
  int priority;
  void *task;

  asm("ldr %0, [%1, #4];":"=r"(priority):"r"(td->sp));
  asm("ldr %0, [%1, #8];":"=r"(task):"r"(td->sp));

  //TODO: FIX THIS ONCE SCHEDULING IS DONE
  if (tid < 0 /*|| (tid & 0xffff) >= 10*/) {
    td->ret = -2;
    KASSERT(false && "Out of Tids");
  }
  //else if(bad priority)
  else {
    TaskDescriptor *newTask = &tasks[(tid & 0xffff)];
    ktd_create(newTask, tid, task, priority, READY, td);
    pq_push(&pq_tasks, priority, newTask);
    td->ret = tid;
  }
}

void get_tid(TaskDescriptor *td) {
  td->ret = td->tid;
}

void get_parentTid( TaskDescriptor *td) {
  //Get the parent tid into user stack
  // asm(
  //   "str %0, [%1, #4]"::"r"(td->parent ? td->parent->tid : -1), "r"(td->sp)
  // );
  td->ret = td->parent ? td->parent->tid : -1;
}

void handle(TaskDescriptor *td, TaskRequest req) {
  switch (req) {
  case ASSERT:
    KABORT();
    break;
  case PASS:
    pq_push(&pq_tasks, td->priority, td);
    break;
  case BLOCK:
    td->status = BLOCKED;
    pq_push(&pq_tasks, td->priority, td);
    break;
  case CREATE:
    create(td);
    pq_push(&pq_tasks, td->priority, td);
    break;
  case MY_TID:
    get_tid(td);
    pq_push(&pq_tasks, td->priority, td);
    break;
  case MY_PARENT_TID:
    get_parentTid(td);
    pq_push(&pq_tasks, td->priority, td);
    break;
  case SEND:
    send_handler(td);
    break;
  case RECEIVE:
    receive_handler(td);
    break;
  case REPLY:
    reply_handler(td);
    break;
  case NS_REG:
    ns_register(td);
    pq_push(&pq_tasks, td->priority, td);
    break;
  case NS_GET:
    ns_get(td);
    pq_push(&pq_tasks, td->priority, td);
    break;
  case EXIT:
    // TODO: uninitialize the task descriptor
    td->status = ZOMBIE;
    tt_return(td->tid, &tid_tracker);
    break;
  default:
    KASSERT(false && "UNDEFINED SWI PARAM");
    pq_push(&pq_tasks, td->priority, td);
    break;
  }
};

int no_tasks() {
  return tid_tracker.cb.size == MAX_TASK;
}


// TODO: fix this
// NOTE: sl register not loaded
//       YOU CANNOT USE GLOBALS IN MAIN
__attribute__((naked)) void main(void) {
  KERNEL_INIT();
#ifdef CACHE
  ENABLE_ALL_CACHE();
#endif

  asm(
    "ldr r3, =KERNEL_ENTRY;"
    "mov r4, #"STR(KERNEL_ENTRY)";"
    "str r3, [r4];"
  );

  initialize();

  while (true) {
    //get a task from scheduler
    TaskDescriptor* td = schedule();

    if (!td && no_tasks()) break;

    if (!td) continue;

    //activate task
    TaskRequest req = activate(td);

    //Handle the swi
    handle(td, req);
  }

#ifdef CACHE
  DISABLE_ALL_CACHE();
#endif

  KERNEL_EXIT();
}


