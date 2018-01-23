#include <system.h>
// #include <task.h>

#define TQ_SIZE 10

// task_queue error codes
#define ETQ_NONE  0
#define ETQ_FULL  1
#define ETQ_EMPTY 2

typedef struct TaskDescriptor {
  int tid;
  struct TaskDescriptor *next;
} TaskDescriptor;


typedef struct task_queue {
  TaskDescriptor *head;
  TaskDescriptor *tail;
  int size;
} task_queue;


void tq_init(task_queue *tq);

int tq_push(task_queue *tq, TaskDescriptor *t);

int tq_pop(task_queue *tq, TaskDescriptor **t);

int tq_peek(task_queue *tq, TaskDescriptor **t);

// void tq_print(task_queue *tq);

