#include <assert.h>
#include <stdio.h>
#include <test/circularbuffer.h>
#include <test/task_queue.h>
#include <test/priority_queue.h>
#include <test/nameserver.h>

int main(void) {
  printf("\n=== RUNNING UNIT TESTS ===\n");
  circular_buffer_tests();
  task_queue_tests();
  priority_queue_tests();
  nameserver_tests();
  printf("✓✓✓ ALL TESTS PASSED :D ✓✓✓\n\n");
  return 0;
}

