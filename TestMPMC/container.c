#ifndef WRITE_ERROR_LOG
#include <stdio.h>
#include <stdarg.h>
#define WRITE_ERROR_LOG(format, ...) fprintf(stderr, format, __VA_ARGS__)
#endif

#include <errno.h>
#include <stddef.h> // offsetof(type, member)
#include <stdint.h> // typedef unsigned char uint8_t;
#include <stdlib.h>
#include <string.h>

#include "container.h"

/**
 * 计算成员变量的指针偏移量, 推导出外层 container 结构体的首字节地址
 */
void *get_container_ptr_by_member2(struct cds_lfq_node_rcu *member2)
{
	uint8_t *container_ptr = (uint8_t *) member2;

	container_ptr -= offsetof(struct container_st, member2);
	return container_ptr;
}

/**
 * 计算成员变量的指针偏移量, 推导出外层 container 结构体的首字节地址
 */
void *get_container_ptr_by_member3(struct rcu_head *member3)
{
	uint8_t *container_ptr = (uint8_t *) member3;

	container_ptr -= offsetof(struct container_st, member3);
	return container_ptr;
}

/**
 * CONTAINER 类的构造函数
 */
CONTAINER *CONTAINER_new(int init_value)
{
	struct container_st *ptr = NULL;
	ptr = malloc(sizeof(struct container_st));
	if (!ptr) {
		WRITE_ERROR_LOG("Error: System is out of memory: %s\n", strerror(errno));
		WRITE_ERROR_LOG("[SOURCE=%s:LINE=%d:errno=%d]\n", __FILE__, __LINE__, errno);
		abort();
	}
	ptr->value = init_value;
	cds_lfq_node_init_rcu(&(ptr->member2));
	memset(&ptr->member3, 0x00, sizeof(ptr->member3));
	return (ptr);
}

/* 内部辅助函数 */
static
void CONTAINER_free_callback(struct rcu_head *member3)
{
	/* Getting the container structure from field member3 */
	void *container_ptr = get_container_ptr_by_member3(member3);
	free(container_ptr);
}

/**
 * CONTAINER 类的析构函数
 */
void CONTAINER_free(struct container_st *ptr)
{
	call_rcu(&ptr->member3, CONTAINER_free_callback);
}