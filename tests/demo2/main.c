#include <stdio.h>
#include <urcu.h>
#include <urcu/rculfqueue.h> /* RCU Lock-free queue */

#include "container.h"

int main()
{
    struct cds_lfq_queue_rcu myqueue;
    
    rcu_register_thread();//RCU注册当前线程
    
    /* Init the queue */
    cds_lfq_init_rcu(&myqueue, call_rcu);
    
    // TODO: 对队列进行读写操作
    
    /* Release memory used by the queue. */
    int err=0;
    err = cds_lfq_destroy_rcu(&myqueue);
    if (err) {
        fprintf(stderr, "Error destroying queue (non-empty), err = %d\n", err);
    }
    
    rcu_unregister_thread();//RCU注销当前线程
    return 0;
}
