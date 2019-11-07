#ifndef TEST_H_
#define TEST_H_

#include "include.h"
#include "toe.h"

struct switch_holder
{
	volatile bool value;
};

void thread_set_true(struct switch_holder*);
void thread_set_false(struct switch_holder*);

void thread_wait_until_true(struct switch_holder*);

int wait_thread_for_state
(
	struct rerunnable_thread* thread,
	enum rerunnable_thread_state state,
	int wait_unit_time_us,
	int try_max_count
);

int wait_switch_for_state
(
	struct switch_holder* sw,
	bool state,
	int wait_unit_time_us,
	int try_max_count
);

void assert_thread_reach_state
(
	const char* info,
	struct rerunnable_thread* rrt,
	enum rerunnable_thread_state state
);

void assert_switch_reach_state
(
	const char* info,
	struct switch_holder* sw,
	bool state
);

struct rerunnable_thread* toetest_test_create_idle_thread();

void toetest_test_destroy_thread(struct rerunnable_thread* thread);

/**
 * submit a new task and ensures, it's really gona be executed
 * (checks not the result, but the thread is idle and therefore
 * accepts the given task)
 * Otherwise this call breaks the test case.
 * */
void toetest_test_thread_execute_with_ensure
(
	struct rerunnable_thread* thread,
	void (*func)(void*),
	void* param
);

struct worker_pool* toetest_test_create_worker_pool();

void toetest_test_destroy_worker_pool(struct worker_pool* wp);

void assert_not_locked(void* LOCK, int (*trylock)(void*), int (*unlock)(void*));



#endif /* TEST_CORE_H_ */
