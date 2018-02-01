#ifndef TASK_H
#define TASK_H
#include <defines.h>
#include <types.h>
#include <asm.h>
#include <circularbuffer.h>

// TODO: these are copies of the ones in kernel.h, we should figure out where
//       to put them centrally.

#define MAX_TASK 16

typedef enum TaskStatus { // a task is...
  ACTIVE  = 0,            //  active, currently running
  READY   = 1,            //  ready to be run
  BLOCKED = 2,            //  blocked, waiting for something
  UNINIT  = 3,            //  not yet initialized
  ZOMBIE  = 4,            //  dead... but still alive?
  RCV_BL  = 5,
  SND_BL  = 6,
  RPL_BL  = 7,
} TaskStatus;

//Kernel Handles Task Request
typedef enum TaskRequest {
  PASS           = 0,
  BLOCK          = 1,
  CREATE         = 2,
  MY_TID         = 3,
  MY_PARENT_TID  = 4,
  EXIT           = 5,
  ASSERT         = 6,
  SEND           = 7,
  RECEIVE        = 8,
  REPLY          = 9,
  NS_REG         = 10,
  NS_GET         = 11
} TaskRequest;

typedef struct TaskDescriptor {
  int  tid; //Task id

  uint32_t  sp;   //stack pointer
  uint32_t  stack_base; //stack base

  uint32_t  psr;    //Status Register

  void* task; //Function pointer
  TaskStatus status;  //Task status

  struct TaskDescriptor *parent; //Parent task
  struct TaskDescriptor *next;
  int priority;

  CircularBuffer send_q;

  uint32_t ret; //Return value
} TaskDescriptor;

typedef struct TidTracker {
  CircularBuffer cb;
} TidTracker;

void tt_init(TidTracker *tt);
int tt_get(TidTracker *tt);
void tt_return(int tid, TidTracker *tt);
int tt_size(TidTracker *tt);

/**
 * Initialize a task descriptor to be uninitialized.
 */
void td_init(TaskDescriptor *td);

#endif /* TASK_H */
