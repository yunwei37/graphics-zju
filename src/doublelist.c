#include "doublelist.h"

List createEmptyList() {
	List a;
	a = (List)malloc(sizeof(struct LNode));
	a->Next = (List)malloc(sizeof(struct LNode));
	a->Next->Before = a;
	a->Before = NULL;
	a->Next->Next = NULL;
	return a;
}

List Insert_list_node(PtrToLNode L, void* X) {
	if (L == NULL)
		acl_error("insert list at wrong pos!");
	List a;
	a = (List)malloc(sizeof(struct LNode));
	L->Next->Before = a;
	a->Next = L->Next;
	L->Next = a;
	a->Before = L;
	a->Data = X;
	return a;
}

List Delete_list_node(PtrToLNode L) {
	if (L == NULL || L->Before == NULL || L->Next == NULL) {
		acl_error("delete list at wrong pos!");
		return NULL;
	}
	List pre;
	pre = L->Before;
	pre->Next = L->Next;
	L->Next->Before = pre;
	free(L);
	return pre;//do the deetion;
}

List Find_node(List L,void* X) {
	List p;
	for (p = L->Next; p->Next != NULL; p = p->Next) {
		if (p->Data == X)
			return p;
	}
	return NULL;
}


void delete_list(List L) {
	List p,t;
	for (p = L->Next; p->Next != NULL;) {
		t = p;
		p = p->Next;
		Delete_list_node(t);
	}
	free(L->Next);
	free(L);
}