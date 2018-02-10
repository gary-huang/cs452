#ifndef TEST_TASKS_H
#define TEST_TASKS_H

#include <types.h>
#include <io/io.h>
#include <user/syscalls.h>
#include <user/test/test_defines.h>
#include <user/test/taskid_test.h>
#include <user/test/clockserver_test.h>
#include <user/test/context_switch_test.h>
#include <user/test/nameserver_test.h>
#include <user/test/messaging_test.h>
#include <user/test/schedule_test.h>

void TestTask();

#endif /*TEST_TASKS_H*/
