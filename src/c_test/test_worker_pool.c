
#include "test.h"

struct worker_pool* toetest_test_create_worker_pool()
{
	struct worker_pool* pool = malloc_zero(sizeof(struct worker_pool));
	if(0 == wp_init(pool))
	{
		return pool;
	}
	else
	{
		free(pool);
		return NULL;
	}
}

void toetest_test_destroy_worker_pool(struct worker_pool* wp)
{
	TEST_ASSERT_EQUAL(0, wp_shutdown(wp));
	TEST_ASSERT_EQUAL(wp_shutting_down, wp_get_status(wp));
	TEST_ASSERT_EQUAL(0, wp_wait_exit(wp));
	TEST_ASSERT_EQUAL(wp_exited, wp_get_status(wp));
	TEST_ASSERT_EQUAL(0, wp_destroy(wp));
}


void test_worker_pool(void)
{
	struct worker_pool* wp = toetest_test_create_worker_pool();
	TEST_ASSERT_NOT_NULL(wp);

	struct switch_holder sw;
	sw.value = false;

	assert_not_locked
	(
		(void*) &wp->pool_lock,
		(int (*)(void*)) long_lock_trylock,
		(int (*)(void*)) long_lock_unlock
	);

	TEST_ASSERT_EQUAL(0, wp_submit_task(wp, thread_set_true, &sw));

	assert_switch_reach_state
	(
		"pool set value true",
		&sw,
		true
	);


	toetest_test_destroy_worker_pool(wp);
	free(wp);
	//can


	//TODO if task running: 1 idle, 0 busy

	//TODO test for leakage

	//TODO test ping-pong task submitting

	//TODO wait shutdown
	//TODO destroy
}

static void random_sleepers(void* asd)
{
	usleep(1000*(rand()%1000));
}

#ifdef TOETEST_INCLUDE_STRESS_TESTS

void test_worker_pool__high_thread_count(void)
{
	int i=-1;
	while(++i <10)
	{
		struct worker_pool* wp = toetest_test_create_worker_pool();
		TEST_ASSERT_NOT_NULL(wp);

		int n = -1;
		while(++n < 150)
		{
			wp_submit_task(wp, random_sleepers, NULL);
		}

		//half sec
		usleep(1000* 500);

		toetest_test_destroy_worker_pool(wp);
		free(wp);
	}
}
#endif

//TODO shrink/spawn
