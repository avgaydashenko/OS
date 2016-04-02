#include "test.h"

int use[(1 << 16)];

void* func1(void *arg) {

	serial_port_write_line("functon 1 arg: ");
	serial_port_write_num(*(pid_t*)arg);
	serial_port_write_line("\n");

	serial_port_write_line("func1: ");
    if (*(pid_t*)arg == 3)
    	serial_port_write_line("everything is ok\n");
    else
    	serial_port_write_line("something went wrong\n");

    use[get_thread()] = 1;
    yield();
    return 0;
}

void* func2(void *arg) {

	serial_port_write_line("functon 1 arg: ");
	serial_port_write_num(*(pid_t*)arg);
	serial_port_write_line("\n");

	serial_port_write_line("func1: ");
    if (*(pid_t*)arg == 2)
    	serial_port_write_line("everything is ok\n");
    else
    	serial_port_write_line("something went wrong\n");

    use[get_thread()] = 1;
    yield();
    return 0;
}

void test_switch_and_arg() {

	serial_port_write_line("\ntest_switch_and_arg:\n");

    pid_t t2 = 3;
    pid_t t1 = create_thread(func1, &t2);
    create_thread(func2, &t1);
    yield();

	serial_port_write_line("test_switch_and_arg: ");
    if (use[t1] == 1 && use[t2] == 1)
    	serial_port_write_line("everything is ok\n");
    else
    	serial_port_write_line("something went wrong\n");

    serial_port_write_line("return to main thread\n\n");
}


void* func_finish(void *arg __attribute__((unused))) {
    serial_port_write_line("func_finish: everything is ok\n");
    use[get_thread()] = 1;
    return 0;
}

void test_finish() {

	serial_port_write_line("test_finish:\n");

    pid_t t1 = create_thread(func_finish, 0);

    use[t1] = 0;

    yield();

    serial_port_write_line("test_finish: ");
    if (use[t1] == 1)
    	serial_port_write_line("everything is ok\n");
    else
    	serial_port_write_line("something went wrong\n");

    serial_port_write_line("return to main thread\n\n");
}

struct spinlock spinlock1;
struct spinlock spinlock2;

int value = 0;

void* func_lock1(void *arg __attribute__((unused))) {

    use[get_thread()] = 1;
    lock(&spinlock1);

    serial_port_write_line("functon 1: in lock 1; value = ");
	serial_port_write_num(value);
	serial_port_write_line("\n");

    yield();
    lock(&spinlock2);

	serial_port_write_line("functon 1: in lock 2; value = ");
	serial_port_write_num(value);
	serial_port_write_line("\n");

    use[get_thread()] = 2;
    value = 1;

    unlock(&spinlock2);

    serial_port_write_line("functon 1: out lock 2; value = ");
	serial_port_write_num(value);
	serial_port_write_line("\n");

    unlock(&spinlock1);

    serial_port_write_line("functon 1: out lock 1; value = ");
	serial_port_write_num(value);
	serial_port_write_line("\n");

    return 0;
}

void* func_lock2(void *arg __attribute__((unused))) {
    use[get_thread()] = 1;
    lock(&spinlock1);
    use[get_thread()] = 2;

    serial_port_write_line("functon 2: in lock 1; value = ");
	serial_port_write_num(value);
	serial_port_write_line("\n");

    lock(&spinlock2);

    serial_port_write_line("functon 2: in lock 2; value = ");
	serial_port_write_num(value);
	serial_port_write_line("\n");

    value = 2;
    unlock(&spinlock2);

    serial_port_write_line("functon 2: out lock 2; value = ");
	serial_port_write_num(value);
	serial_port_write_line("\n");
    
    unlock(&spinlock1);

    serial_port_write_line("functon 2: out lock 1; value = ");
	serial_port_write_num(value);
	serial_port_write_line("\n");

    return 0;
}

void test_lock() {

	serial_port_write_line("test_lock:\n");

    pid_t t1 = create_thread(func_lock1, 0);
    pid_t t2 = create_thread(func_lock2, 0);

    use[t1] = 0;
    use[t2] = 0;

    yield();
	yield();

    serial_port_write_line("test_lock: ");
    if (use[t1] == 2 && use[t2] == 2)
    	serial_port_write_line("everything is ok\n");
    else
    	serial_port_write_line("something went wrong\n");

    serial_port_write_line("return to main thread\n\n");
}

void* func_join(void *arg __attribute__((unused))) {

	serial_port_write_line("func_join:\n");

    for (int i = 0; i < 9; ++i) {
        yield();

		serial_port_write_line("yield: ");
		serial_port_write_num(i + 1);
		serial_port_write_line("\n");
    }

    return func_join;
}

void test_join() {

	serial_port_write_line("test_join:\n");

    pid_t t1 = create_thread(func_join, 0);

    void* ret;
    join(t1, &ret);

    serial_port_write_line("test_join: ");
    if (ret == func_join)
    	serial_port_write_line("everything is ok\n");
    else
    	serial_port_write_line("something went wrong\n");

    serial_port_write_line("return to main thread\n\n");
}

void* func_create_slab(void *arg __attribute__((unused))) {

    struct slabctl** slab = slab_init(3, 1);

	serial_port_write_line("func_create_slab: ");
	serial_port_write_num((uint64_t) slab_allocate(slab));
	serial_port_write_line("\n");

    return 0;
}

void test_slab() {
	serial_port_write_line("test_slab:\n");

    create_thread(func_create_slab, 0);
    create_thread(func_create_slab, 0);
    while(1);
}

void* func_loop(void *arg __attribute__((unused))) {

    while(1) {
        serial_port_write_line("func_loop: ");
		serial_port_write_num(get_thread());
		serial_port_write_line("\n");
    }
    return 0;
}

void test_timer_interrupt() {

	serial_port_write_line("test_timer_interrupt:\n");

    create_thread(func_loop, 0);
    create_thread(func_loop, 0);
    create_thread(func_loop, 0);

    while (1);
}