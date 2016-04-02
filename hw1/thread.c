#include "interrupt.h"
#include "thread.h"
#include "util.h"
#include "allocator.h"
#include "lock.h"
#include "serial_port.h"

typedef enum {READY, RUNNING, FINISHED, WAITING} thread_state;

struct thread {
    void* stack_pointer;
    void* stack_start;
    int cnt_log_page;
    void* ret_value;
    thread_state state;
};

static volatile pid_t current_thread = 1;
static volatile pid_t previous_thread = 1;

struct thread_pool {
    volatile pid_t first_ready;
    volatile struct thread threads[MAX_THREAD];
    volatile pid_t next[MAX_THREAD];
    volatile pid_t prev[MAX_THREAD];
};

static struct thread_pool thread_pool;

void thread_pool_init() {
    thread_pool.first_ready = 2;
    for (int i = 2; i < MAX_THREAD - 1; ++i) {
        thread_pool.next[i] = i + 1;
    }
    thread_pool.next[1] = thread_pool.prev[1] = 1;

    thread_pool.threads[1].state = RUNNING;
    thread_pool.threads[0].state = READY;

    serial_port_write_line("Initialise thread pool: successful.\n");
}

pid_t create_thread(void* (*fptr)(void *), void *arg) {
    start_critical_section();

    pid_t first = thread_pool.first_ready;
    thread_pool.first_ready = thread_pool.next[first];

    thread_pool.next[first] = thread_pool.next[1];
    thread_pool.prev[first] = 1;
    thread_pool.prev[thread_pool.next[1]] = first;
    thread_pool.next[1] = first;

    volatile struct thread *new_thread = &thread_pool.threads[first];
    new_thread->cnt_log_page = 10;
    new_thread->stack_start = get_page(new_thread->cnt_log_page);

    new_thread->stack_pointer = (uint8_t *)new_thread->stack_start + PAGE_SIZE * (1 << (new_thread->cnt_log_page));

    struct init_thread_data  {
        uint64_t r15, r14, r13, r12, rbx, rbp;
        void* start_thread_address;
        void* function_address;
        void* arg;
    };

    new_thread->stack_pointer = (uint8_t *)new_thread->stack_pointer - sizeof(struct init_thread_data);

    struct init_thread_data* init_value = new_thread->stack_pointer;

    init_value->r12 = 0;
    init_value->r13 = 0;
    init_value->r14 = 0;
    init_value->r15 = 0;
    init_value->rbx = 0;
    init_value->rbp = 0;

    extern void *start_thread;
    init_value->start_thread_address = &start_thread;

    init_value->function_address = fptr;
    init_value->arg = arg;

    new_thread->state = RUNNING;
    end_critical_section();
    return (pid_t)(new_thread - thread_pool.threads);
}

pid_t get_thread() {
    return current_thread;
}

// used in thread_handlers.S
void switch_threads(void **old_sp, void *new_sp);

// used in thread_handlers.S
void check_thread_finished() {
    volatile struct thread* thread = thread_pool.threads + previous_thread;

    if (thread->state == FINISHED && previous_thread != current_thread) {
        free_page(thread->stack_start, thread->cnt_log_page);
        thread->state = WAITING;
    }
}

void yield() {
    start_critical_section();

    for (pid_t i = thread_pool.next[current_thread]; ; i = thread_pool.next[current_thread]) {
        
        if (i == 0 || thread_pool.threads[i].state != RUNNING)
            continue;
        
        if (current_thread == i)
            return;

        struct thread *thread = (struct thread*)thread_pool.threads + i;

        int old_thread_id = current_thread;
        current_thread = i;
        previous_thread = old_thread_id;

        struct thread *old_thread = (struct thread*)thread_pool.threads + old_thread_id;

        switch_threads(&old_thread->stack_pointer, thread->stack_pointer);

        check_thread_finished();
        
        break;
    }

    end_critical_section();
}


void join(pid_t thread, void** retvalue) {
    while (thread_pool.threads[thread].state != WAITING) {
        yield();
        barrier();
    }
    if (retvalue) {
        *retvalue = thread_pool.threads[thread].ret_value;
    }
    thread_pool.threads[thread].state = READY;

    start_critical_section();
    thread_pool.next[thread] = thread_pool.first_ready;
    thread_pool.first_ready = thread;
    end_critical_section();
}

// used in thread_handlers.S
void finish_current_thread(void* value) {
    start_critical_section();
    int ct = get_thread();

    volatile struct thread* current_t = thread_pool.threads + ct;
    current_t->state = FINISHED;
    current_t->ret_value = value;

    thread_pool.prev[thread_pool.next[ct]] = thread_pool.prev[ct];
    thread_pool.next[thread_pool.prev[ct]] = thread_pool.next[ct];

    end_critical_section();
    yield();
}