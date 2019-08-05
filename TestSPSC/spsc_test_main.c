// 版本状态：新范式内部Demo代码，未经测试前严禁用于生产环境！

/* ISO/IEC 9899:2011 (ISO国际标准2011版C语言头文件) */
#include <errno.h>
#include <stdio.h>
#include <string.h> // 用到 memset()、strerror() 等函数

/* POSIX.1 IEEE 标准头文件 */
#include <pthread.h> // POSIX 线程库: 用到 pthread_create() 等函数
#include <unistd.h> // Unix 标准库: 用到 sleep() 等函数

/* 第三方 SDK 提供的头文件 */
#include <urcu.h>
#include <urcu/rculfqueue.h> /* RCU Lock-free queue */

/* 新范式自定义头文件 */
#include "container.h"

/* 定义若干“生产者”/“消费者”线程 */

static
void *producer_thrd(void *arg){
    // producer 生产者线程:
    // 监听 UDP 8000 端口，每收到一个UDP数据包，执行一次enqueue操作。
    // 约定: 循环收包，直到收到某个特殊的命令包后跳出无限循环，退出线程。
    //
    // (为了简化调试，以下代码使用写死的临时数据代替真实的UDP套接字读写)

    struct cds_lfq_queue_rcu *queue = (struct cds_lfq_queue_rcu *) arg;

    rcu_register_thread();//RCU注册当前线程

    int values[] = { 1, 2, 3, 4, 5, 6, 7, INT32_MAX};
    unsigned TOTAL_VALUES = sizeof(values)/sizeof(values[0]);
    for (unsigned i = 0; i < TOTAL_VALUES; i++) {
        sleep(2); // TODO: (使用随机数模拟真实的UDP收包间隔时间)

        CONTAINER *container_ptr = CONTAINER_new(values[i]);
        if (!container_ptr) {
            goto TAG_ERROR_REPORT;
        }

        // 无锁队列 enqueue 和 dequeue 两种操作必须被约束在 RCU 临界区段内(RCU read-side critical section)
        rcu_read_lock();
        cds_lfq_enqueue_rcu(queue, &container_ptr->member2); // 无锁队列的 enqueue 操作只关注取成员变量 member2 的指针
        rcu_read_unlock();
    }

    rcu_unregister_thread();//RCU注销当前线程
    return NULL;

    TAG_ERROR_REPORT:{
        fprintf(stderr, "Error: no memory to allocate new queue item!\n");
        fprintf(stderr, "Error: errno=%d, message=%s\n", errno, strerror(errno));
    }
    rcu_unregister_thread();//RCU注销当前线程
    return NULL;
}

static
void *consumer_thrd(void *arg){
    // consumer 消费者线程:

    struct cds_lfq_queue_rcu *queue = (struct cds_lfq_queue_rcu *) arg;

    rcu_register_thread();//RCU注册当前线程

    // cosumer(消费者线程): 对队列进行dequeue操作
    // Dequeue each node from the queue. Those will be dequeued from
    // the oldest (first enqueued) to the newest (last enqueued).
    printf("消费者线程执行dequeue操作，输出如下:\n");
    fflush(stdout);
    for (;;) {
        CONTAINER *container_ptr = NULL;
        struct cds_lfq_node_rcu *member2_ptr = NULL;

        // Both enqueue and dequeue need to be called within RCU
        // read-side critical section.
        rcu_read_lock();
        member2_ptr = cds_lfq_dequeue_rcu(queue);
        rcu_read_unlock();
        if (!member2_ptr) {
            /* 当前队列已经无结点了，此处原地休眠若干秒，等待生产者生产下一个结点。 */
            sleep(1);
            continue;
        }
        /* Getting the container structure from its member2 (from the "member2" inside "struct container") */
        container_ptr = get_container_ptr_by_member2(member2_ptr);
        int value = container_ptr->value;
        CONTAINER_free(container_ptr);
        if (INT32_MAX == value) {
            /* 约定: value 等于 INT32_MAX 表示没有后续数据了 */
            printf(" INT32_MAX");
            break;
        }
        printf(" %d", value);
        fflush(stdout);
    }
    printf("\n");

    rcu_unregister_thread();//RCU注销当前线程
    return NULL;
}


#define NULL_PTHREAD_ATTR NULL
static int UNDEFINED = -1;
static void * const UNDEFINED_RETCODE = (void *)&UNDEFINED;

int main()
{
    pthread_t producer;
    pthread_t consumer;
    void *producer_retcode_ptr = UNDEFINED_RETCODE;
    void *consumer_retcode_ptr = UNDEFINED_RETCODE;

    struct cds_lfq_queue_rcu myqueue;

    rcu_register_thread();//RCU注册当前线程

    /* Init the queue */
    cds_lfq_init_rcu(&myqueue, call_rcu);

    /* 启动 producer 生产者子线程: */
    // 子线程中模拟以下动作：监听某个IPv4-UDP套接字端口，每收到一个UDP数据包，执行一次enqueue操作。约定收到某个结束符后跳出无限循环，退出线程。
    pthread_create(&producer, NULL_PTHREAD_ATTR, producer_thrd, (void *)&myqueue);

    /* 启动 consumer 消费者子线程: */
    pthread_create(&consumer, NULL_PTHREAD_ATTR, consumer_thrd, (void *)&myqueue);

    /* 主进程等待各个子线程结束: */
    producer_retcode_ptr = UNDEFINED_RETCODE;
    pthread_join(producer, &producer_retcode_ptr);
    consumer_retcode_ptr = UNDEFINED_RETCODE;
    pthread_join(consumer, &consumer_retcode_ptr);

    /* Release memory used by the queue. */
    {
        int ec=0;
        ec = cds_lfq_destroy_rcu(&myqueue);
        if (ec) {
            fprintf(stderr, "Error destroying queue (non-empty), Error code: ec = %d\n", ec);
        }
    }

    rcu_unregister_thread();//RCU注销当前线程
    return 0;
}
