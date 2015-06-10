
/* Copyright (C) 2010. sparkling.liang@hotmail.com. All rights reserved. */

#ifndef __CONHASH_INTER_H_
#define __CONHASH_INTER_H_

#include "configure.h"
#include "md5.h"
#include "util_rbtree.h"


#include <stdio.h>
#include <stdlib.h>
#include <memory.h>
#include <string.h>

/* virtual node structure */
struct virtual_node_s
{
	long hash;             //虚拟结点的hash值   
	struct node_s *node; /* pointer to node */  //虚拟结点指向的真实结点
};

/* consistent hashing */
struct conhash_s
{
	util_rbtree_t vnode_tree; /* rbtree of virtual nodes */ //虚拟结点的红黑树
    u_int ivnodes; /* virtual node number *///虚拟结点的个数
	long (*cb_hashfunc)(const char *);  //hash 函数
};

struct __get_vnodes_s
{
    long *values;
    long size, cur;
};


int __conhash_vnode_cmp(const void *v1, const void *v2);

/*
 * 函数:_conhash_node2string
 * 功能:计算虚拟结点的hash值
 * 参数:i代表当前虚拟结点是物理结点node的第i个虚拟结点
 * 
 */
void __conhash_node2string(const struct node_s *node, u_int replica_idx, char buf[128], u_int *len);
long __conhash_hash_def(const char *instr);
/*
 * 函数:_conhash_add_replicas
 * 功能:把结点添加到一致性hash中，结点可能有多个虚拟结点
 */
void __conhash_add_replicas(struct conhash_s *conhash, struct node_s *node);
void __conhash_del_replicas(struct conhash_s *conhash, struct node_s *node);

util_rbtree_node_t *__conhash_get_rbnode(struct node_s *node, long hash);
void __conhash_del_rbnode(util_rbtree_node_t *rbnode);

#endif /* end __CONHASH_INTER_H_ */
