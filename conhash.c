
/* Copyright (C) 2010. sparkling.liang@hotmail.com. All rights reserved. */

#include "conhash.h"
#include "conhash_inter.h"

struct conhash_s* conhash_init(conhash_cb_hashfunc pfhash)
{
    /* alloc memory and set to zero */
    struct conhash_s *conhash = (struct conhash_s*)calloc(1, sizeof(struct conhash_s));
    if(conhash == NULL)
    {
        return NULL;
    }
    do
	{
        /* setup callback functions */
        if(pfhash != NULL)
        {
            conhash->cb_hashfunc = pfhash;
        }
        else
        {
            conhash->cb_hashfunc = __conhash_hash_def;
        }
        //初始化红黑树
	util_rbtree_init(&conhash->vnode_tree);
        return conhash;

	}while(0);

    free(conhash);
    return NULL;
}

void conhash_fini(struct conhash_s *conhash)
{
	if(conhash != NULL)
	{
		/* free rb tree */
        while(!util_rbtree_isempty(&(conhash->vnode_tree)))
        {
            util_rbtree_node_t *rbnode = conhash->vnode_tree.root;
            util_rbtree_delete(&(conhash->vnode_tree), rbnode);
            __conhash_del_rbnode(rbnode);
        }
		free(conhash);
	}
}
/*
 * 函数:conhash_set_node
 * 参数:虚拟节点vnode数量由conhash_set_node的第三个参数replicas控制
 */
void conhash_set_node(struct node_s *node, const char *iden, u_int replica)
{
    strncpy(node->iden, iden, sizeof(node->iden)-1);
    node->replicas = replica;
    node->flag = NODE_FLAG_INIT;
}

/*
 * 函数名:conhash_add_node
 * 功能：把结点添加到一致性hash中
 */
int conhash_add_node(struct conhash_s *conhash, struct node_s *node)
{
    if((conhash==NULL) || (node==NULL)) 
    {
        return -1;
    }
    /* check node fisrt */
    //这种写法不错
    if(!(node->flag&NODE_FLAG_INIT) || (node->flag&NODE_FLAG_IN))
    {
        return -1;
    }
    //设置添加结点的状态为已添加状态　
    node->flag |= NODE_FLAG_IN;
    /* add replicas of server */
    __conhash_add_replicas(conhash, node);
 
    return 0;
}

int conhash_del_node(struct conhash_s *conhash, struct node_s *node)
{
   if((conhash==NULL) || (node==NULL)) 
    {
        return -1;
    }
    /* check node first */
    if(!(node->flag&NODE_FLAG_INIT) || !(node->flag&NODE_FLAG_IN))
    {
        return -1;
    }
    node->flag &= (~NODE_FLAG_IN);
    /* add replicas of server */
    __conhash_del_replicas(conhash, node);

    return 0;
}
/*
 * 函数名:conhash_lookup
 * 功能：查看object 应该分配在哪台结点上?
 * 参数：object 是要查询的对象
 */
const struct node_s* conhash_lookup(const struct conhash_s *conhash, const char *object)
{
    long hash;
    const util_rbtree_node_t *rbnode;
    if((conhash==NULL) || (conhash->ivnodes==0) || (object==NULL)) 
    {
        return NULL;
    }
    /* calc hash value */
    //计算出来被查询对象的hash值
    hash = conhash->cb_hashfunc(object);
    
    rbnode = util_rbtree_lookup(&(conhash->vnode_tree), hash);
    if(rbnode != NULL)
    {
        struct virtual_node_s *vnode = rbnode->data;
        return vnode->node;
    }
    return NULL;
}
