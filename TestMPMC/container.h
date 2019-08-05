#ifndef CONTAINER_H
#define CONTAINER_H

#include <urcu.h>
#include <urcu/rculfqueue.h>

typedef struct container_st CONTAINER;

/*
 * 用户数据容器
 */
struct container_st {
	/* 此示例程序中无锁队列中的用户数据是 int value */
	int value;
	/* 结构体成员变量 member2 存储无锁队列结点信息 */
	struct cds_lfq_node_rcu member2;
	/* 结构体成员变量 member3 仅用于辅助执行RCU回调函数 */
	struct rcu_head member3;
};

extern void *get_container_ptr_by_member2(struct cds_lfq_node_rcu *member2);
extern void *get_container_ptr_by_member3(struct rcu_head *member3);

extern CONTAINER *CONTAINER_new(int init_value);
extern void CONTAINER_free(CONTAINER *container);

#endif /* CONTAINER_H */