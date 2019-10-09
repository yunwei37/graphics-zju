/* list.h: the impliment of a list without headnode.
 * version 1.0 by ZhengYusheng 25/3/2018
 */

#ifndef _DOUBLELIST_H
#define _DOUBLELIST_H

#include <stdio.h>
#include <stdlib.h>

/* the defination of the list structure*/
//变量保存链表的结构声明
typedef struct LNode *PtrToLNode;
struct LNode {
    void* Data;
	PtrToLNode Before;
    PtrToLNode Next;
};
typedef PtrToLNode List;

/* create a empty list with a head node and a tail node;
 * return the head node of the list;
 */
List createEmptyList();

/* Insert X After the node pointed to by location P, 
 * and return the insert node.
 * If parameter P points to an illegal location, 
 * 
 */ 
List Insert_list_node(PtrToLNode L, void* X );//用来插入新的变量节点；

/* Delete the element of position P and return the 
 * list node befor L. If parameter P points to an illegal location, 
 * print "Wrong Position for Deletion" and return NULL.
 * List Delete( List L, Position P );
 */
List Delete_list_node(PtrToLNode L);//用来删除被更新的变量节点；

/*
 * find a list node in the list
 * the function travel the list and find the list node with the same element
 * if more than one element exist, it return the first one 
 */
List Find_node(List L,void* X);

/*
 * delete the whole list and the node in it
 */
void delete_list(List L);

#endif
