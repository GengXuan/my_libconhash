
/* Copyright (C) 2010. sparkling.liang@hotmail.com. All rights reserved. */

#include "conhash_inter.h"
#include "conhash.h"

/* 
 * the default hash function, using md5 algorithm
 * @instr: input string
 */
/*
 * 返回hash值
 */
long __conhash_hash_def(const char *instr)
{
    int i;
    long hash = 0;
    unsigned char digest[16];
    conhash_md5_digest((const u_char*)instr, digest);

    /* use successive 4-bytes from hash as numbers */
    //数组中的每一位都进行移位操作
    for(i = 0; i < 4; i++)
    {
        hash += ((long)(digest[i*4 + 3]&0xFF) << 24)
            | ((long)(digest[i*4 + 2]&0xFF) << 16)
            | ((long)(digest[i*4 + 1]&0xFF) <<  8)
            | ((long)(digest[i*4 + 0]&0xFF));
    }
	return hash;
}

/*
 * 函数:_conhash_node2string
 * 功能:计算虚拟结点的hash值
 * 参数:i代表当前虚拟结点是物理结点node的第i个虚拟结点
 * 
 */
void __conhash_node2string(const struct node_s *node, u_int replica_idx, char buf[128], u_int *len)
{
#if (defined (WIN32) || defined (__WIN32))
    _snprintf_s(buf, 127, _TRUNCATE, "%s-%03d", node->iden, replica_idx);
#else
    snprintf(buf, 127, "%s-%03d", node->iden, replica_idx);
#endif
}
/*
 * 函数:_conhash_add_replicas
 * 功能:把结点添加到一致性hash中，结点可能有多个虚拟结点
 */
void __conhash_add_replicas(struct conhash_s *conhash, struct node_s *node)
{
    u_int i, len;
    long hash;
    char buff[128];
    util_rbtree_node_t *rbnode;
    for(i = 0; i < node->replicas; i++)
    {
        /* calc hash value of all virtual nodes */
        __conhash_node2string(node, i, buff, &len);
        //buf 中存放的是虚拟结点的名称(虚拟结点的名称由真实结点名称和虚拟结点的编号组成)
        //hash为虚拟结点的hash值
        hash = conhash->cb_hashfunc(buff);
        /* add virtual node, check duplication */
        //在一致性hash的虚拟结点树中查找是否已经有该hash值
        if(util_rbtree_search(&(conhash->vnode_tree), hash) == NULL)
        {
            rbnode = __conhash_get_rbnode(node, hash);
            if(rbnode != NULL)
            {
                //把结点插入到一致性hash的虚拟结点树中
                util_rbtree_insert(&(conhash->vnode_tree), rbnode);
                //一致性hash中虚拟结点的个数加１
                conhash->ivnodes++;
            }
        }
    }
}
/*
 * 函数名:_conhash_del_replicas
 * 功能:从虚拟树中删除结点node的所有虚拟结点
 */
void __conhash_del_replicas(struct conhash_s *conhash, struct node_s *node)
{
    u_int i, len;
    long hash;
    char buff[128];
    struct virtual_node_s *vnode;
    util_rbtree_node_t *rbnode;
    for(i = 0; i < node->replicas; i++)
    {
        /* calc hash value of all virtual nodes */
        __conhash_node2string(node, i, buff, &len);
        hash = conhash->cb_hashfunc(buff);
        rbnode = util_rbtree_search(&(conhash->vnode_tree), hash);
        if(rbnode != NULL)
        {
            vnode = rbnode->data;
            if((vnode->hash == hash) && (vnode->node == node))
            {
                conhash->ivnodes--;
                util_rbtree_delete(&(conhash->vnode_tree), rbnode);
                __conhash_del_rbnode(rbnode);
            }
        }
    }
}

/*
 * 函数:_conhash_get_rbnode
 * 参数:node 真实的结点,hash是虚拟结点的hash值
 */
util_rbtree_node_t *__conhash_get_rbnode(struct node_s *node, long hash)
{
    util_rbtree_node_t *rbnode;
    //创建一个结点
    rbnode = (util_rbtree_node_t *)malloc(sizeof(util_rbtree_node_t));
    if(rbnode != NULL)
    {
        rbnode->key = hash;
        rbnode->data = malloc(sizeof(struct virtual_node_s));
        if(rbnode->data != NULL)
        {
            struct virtual_node_s *vnode = rbnode->data;
            vnode->hash = hash;
            vnode->node = node;
        }
        else
        {
            free(rbnode);
            rbnode = NULL;
        }
    }
    return rbnode;
}

void __conhash_del_rbnode(util_rbtree_node_t *rbnode)
{
    struct virtual_node_s *node;
    node = rbnode->data;
    free(node);
    free(rbnode);
}
