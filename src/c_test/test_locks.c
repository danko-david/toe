

#include "test.h"

struct lock_test_helper
{
	void* LOCK;
	int (*lock)(void*);
	int (*unlock)(void*);
	int (*trylock)(void*);
	struct switch_holder signal_to_thread;
	struct switch_holder feedback_from_thread;
};

static void thread_lock_then_set_true(struct lock_test_helper* help)
{
	TEST_ASSERT_EQUAL(false, help->signal_to_thread.value);
	help->signal_to_thread.value = true;

	help->lock(help->LOCK);

	TEST_ASSERT_EQUAL(false, help->feedback_from_thread.value);
	help->feedback_from_thread.value = true;

	help->unlock(help->LOCK);
}

void assert_not_locked(void* LOCK, int (*trylock)(void*), int (*unlock)(void*))
{
	TEST_ASSERT_EQUAL(0, trylock(LOCK));
	TEST_ASSERT_EQUAL(0, unlock(LOCK));
}

static void generic_test_lock
(
	void* LOCK,
	int (*init)(void*),
	int (*lock)(void*),
	int (*unlock)(void*),
	int (*trylock)(void*),
	int (*destroy)(void*)
)
{
	TEST_ASSERT_EQUAL(0, init(LOCK));

	//newly created locks bust be unlocked by default
	//if lock locked by default , this call fails faster
	//because simple "lock()" blocks the execution, so in that case the
	//test case will be failed because of timeout.
	TEST_ASSERT_EQUAL(0, trylock(LOCK));

	//previous call locked, now release.
	TEST_ASSERT_EQUAL(0, unlock(LOCK));

	TEST_ASSERT_EQUAL(0, lock(LOCK));

	//must "fail" with EBUSY
	TEST_ASSERT_EQUAL(EBUSY, trylock(LOCK));

	TEST_ASSERT_EQUAL(0, unlock(LOCK));

	//involving threads, testing real concurrency locking
	{
		struct lock_test_helper help;
		help.LOCK = LOCK;
		help.lock = lock;
		help.unlock = unlock;
		help.trylock = trylock;
		help.signal_to_thread.value = false;
		help.feedback_from_thread.value = false;

		/**
		 * Test case:
		 * 	we lock the lock
		 * 	we start a new thread, and we waiting for the thread become started
		 *	we releases the lock
		 *	now the thread can lock the lock, and set the
		 *		feedback_from_thread.value to true
		 *
		 * */
		struct rerunnable_thread* thread = toetest_test_create_idle_thread();

		lock(LOCK);

		toetest_test_thread_execute_with_ensure
		(
			thread,
			(void (*)(void*)) thread_lock_then_set_true,
			(void*) &help
		);

		//if thread doesn't start, we fail the test case with timeout
		thread_wait_until_true(&help.signal_to_thread);

		//test for not falling through the locked code part
		TEST_ASSERT_EQUAL(false, help.feedback_from_thread.value);
		unlock(LOCK);

		//it can take "a while"
		thread_wait_until_true(&help.feedback_from_thread);

		TEST_ASSERT_EQUAL(true, help.feedback_from_thread.value);

		toetest_test_destroy_thread(thread);
	}
}

void test_short_lock(void)
{
	short_lock lock;
	generic_test_lock
	(
		(void*) &lock,
		(int (*)(void*)) short_lock_init,
		(int (*)(void*)) short_lock_lock,
		(int (*)(void*)) short_lock_unlock,
		(int (*)(void*)) short_lock_trylock,
		(int (*)(void*)) short_lock_destroy
	);
}


void test_long_lock(void)
{
	long_lock lock;
	generic_test_lock
	(
		(void*) &lock,
		(int (*)(void*)) long_lock_init,
		(int (*)(void*)) long_lock_lock,
		(int (*)(void*)) long_lock_unlock,
		(int (*)(void*)) long_lock_trylock,
		(int (*)(void*)) long_lock_destroy
	);
}
