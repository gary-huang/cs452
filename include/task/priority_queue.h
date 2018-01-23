#define NUM_PRIORITIES 5

typedef struct priority_queue {
  task_queue pqs[NUM_PRIORITIES];
} priority_queue;


void pq_init(priority_queue *pq);

int pq_push(priority_queue *pq, int priority, TaskDescriptor *t);

int pq_pop(priority_queue *pq, TaskDescriptor **t);
