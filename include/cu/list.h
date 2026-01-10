// This Source Code Form is subject to the terms of the Mozilla Public
// License, v. 2.0. If a copy of the MPL was not distributed with this
// file, You can obtain one at https://mozilla.org/MPL/2.0/.

// SPDX-License-Identifier: MPL-2.0

#ifndef CU_LIST_H
#define CU_LIST_H


#ifdef _MSC_VER
#	define CU_TYPEOF(T) __typeof__(T)
#else
#	define CU_TYPEOF(T) typeof(T)
#endif

#include <stdint.h>
#include <stddef.h>
#include <stdbool.h>
#include <assert.h>


// A generic linked list library, similar to the one used in the Linux kernel.
//
// The code's of course different, it is my own implementation.
// The interface also differs slightly.
// But, the most important features should be here.
//
// To use this, you add a `cu_slist` or `cu_dlist` field to some struct.
// You can then use the functions in this library to add it to a list, and
// you can then later cast the `cu_slist` or `cu_dlist` back to its actual type
// with `cu_container_of`.
//
// In this way, type erasure is achieved, and the amount of macro-hell is
// reduced.
//
// The `cu_slist` (singly-linked circular list) and `cu_dlist` (doubly-linked
// circular list) serve simultaneously as nodes and as handles for the entire
// list, depending on context.
//
// The first node of any list is a "head node" that carries no associated data.
// It is simply a handle to the rest of the list. You must initialize these
// head nodes with cu_slist_init_head or cu_dlist_init_head. Subsequent nodes
// can carry data and do not need to be initialized in this way; they are
// initialized when they are added to a list.
//
// The API is set up in this way to make it convenient to create an array of
// linked lists, or something similar, while remaining fully generic. You can
// simply make an array of `cu_slist` or `cu_dlist` as needed, instantiate them
// all as heads, and straightforwardly add nodes of arbitrary type to them.
//


// Casts a list back to the type of its container.
// TYPE must not contain a comma, be sensible
#define cu_container_of(PTR, TYPE, MEMBER)\
	((TYPE *)((uint8_t *)(PTR) - offsetof(TYPE, MEMBER)))


typedef struct cu_slist cu_slist;
struct cu_slist {
	cu_slist *next;
};

typedef struct cu_dlist cu_dlist;
struct cu_dlist {
	cu_dlist *next;
	cu_dlist *prev;
};

static inline void cu_slist_init_head(cu_slist *slist_head)
{
	assert(slist_head != NULL && "Cannot instantiate a null pointer");
	slist_head->next = slist_head;
}
static inline void cu_dlist_init_head(cu_dlist *dlist_head)
{
	assert(dlist_head != NULL && "Cannot instantiate a null pointer");
	dlist_head->next = dlist_head;
	dlist_head->prev = dlist_head;
}

#define cu_list_init_head(HEAD)\
	_Generic((HEAD),\
		cu_slist *: cu_slist_init_head,\
		cu_dlist *: cu_dlist_init_head\
	)((HEAD))

// The cu_list_for_each* macros iterate through cu_lists.
//
// The list head is ignored and not iterated over; only the actual elements
// of the list are iterated over.
//
// --- Genericity ---
//
// Macros whose names do not contain "rev" are generic; the cu_list can be
// either a `cu_slist` or a `cu_dlist`.
// 
// Macros whose names do contain "rev" can only iterate through a
// `cu_dlist`.
//
// --- Naming scheme ---
//
// The part of the macro follwing the `cu_slist_for_each` portion is a sequence
// of tags, delimited with `_`. These tags are listed in alphabetical order and
// determine the behavior of the macro.
//
// "continue": Assumes `CURSOR_NAME` is an element within the list whose head
// is `HEAD`. Iteration starts with the element following `CURSOR_NAME`,
// instead of at the beginning of the list.
//
// "safe": Ensures that `CURSOR_NAME` can be invalidated during the body of the
// for loop without causing issues. If this tag is not present, it is invalid
// to invalidate `CURSOR_NAME` inside the body of the for loop.
//
// "cast": Upcasts the `cu_list *` to the type of its container struct before
// storing it to `CURSOR_NAME`. If this tag is present, the container struct
// must contain a `cu_list` element. If it is not present, `CURSOR_NAME` must
// be of type `cu_slist` or `cu_dlist`.
//
// "rev": Iterates through the list in reverse order. Only valid for
// `cu_dlist`.
//
// --- Parameters ---
//
// `CURSOR_NAME` must be of type `cu_slist *` or `cu_dlist *` in macros whose
// names do not contain "cast". In macros whose names contain "cast", it must
// be a pointer to a struct which contains a `cu_slist` or `cu_dlist`. Further,
// it must be a valid identifier name in the current scope.
// As the list is iterated through, `CURSOR_NAME` will be set to the current
// list element.
//
// `HEAD` is a pointer to the head of the list you wish to iterate through.
//
// `TMP_NAME` must be a valid identifier in the current scope of identical
// type to `CURSOR_NAME`.
//
// `MEMBER_NAME` must be the name of the cu_list element in the struct pointer
// you pass as `CURSOR_NAME`.
#define cu_list_for_each(CURSOR_NAME, HEAD)\
	for ((CURSOR_NAME) = (HEAD)->next; (CURSOR_NAME) != (HEAD);\
		(CURSOR_NAME) = (CURSOR_NAME)->next)
#define cu_list_for_each_continue(CURSOR_NAME, HEAD)\
	for ((CURSOR_NAME) = (CURSOR_NAME)->next; (CURSOR_NAME) != (HEAD);\
		(CURSOR_NAME) = (CURSOR_NAME)->next)

// "Safe" variants of the above functions; freeing CURSOR_NAME within the body
// of the loop will not cause issues.
#define cu_list_for_each_safe(CURSOR_NAME, TMP_NAME, HEAD)\
	for ((CURSOR_NAME) = (HEAD)->next, (TMP_NAME) = (CURSOR_NAME)->next;\
		(CURSOR_NAME) != (HEAD);\
		(CURSOR_NAME) = (TMP_NAME), (TMP_NAME) = (TMP_NAME)->next)
#define cu_list_for_each_continue_safe(CURSOR_NAME, TMP_NAME, HEAD)\
	for ((CURSOR_NAME) = (CURSOR_NAME)->next, (TMP_NAME) = (CURSOR_NAME)->next;\
		(CURSOR_NAME) != (HEAD);\
		(CURSOR_NAME) = (TMP_NAME), (TMP_NAME) = (TMP_NAME)->next)

// assumes CURSOR_NAME is a pointer to a type that contains a `cu_slist` or
// `cu_dlist`; this function will iterate through the container type instead
// of the list type
//
// this saves you having to constantly put cu_container_of everywhere
#define cu_list_for_each_cast(CURSOR_NAME, HEAD, MEMBER_NAME)\
	for ((CURSOR_NAME) = cu_container_of(\
			(HEAD)->next, CU_TYPEOF(CURSOR_NAME), MEMBER_NAME\
		); (CURSOR_NAME) != (HEAD);\
		(CURSOR_NAME) = cu_container_of(\
			(CURSOR_NAME)->MEMBER_NAME->next, CU_TYPEOF(CURSOR_NAME), MEMBER_NAME\
		))
#define cu_list_for_each_cast_continue(CURSOR_NAME, HEAD, MEMBER_NAME)\
	for ((CURSOR_NAME) = cu_container_of(\
			(CURSOR_NAME)->next, CU_TYPEOF(CURSOR_NAME), MEMBER_NAME\
		); (CURSOR_NAME) != (HEAD);\
		(CURSOR_NAME) = cu_container_of(\
			(CURSOR_NAME)->MEMBER_NAME->next, CU_TYPEOF(CURSOR_NAME), MEMBER_NAME\
		))

#define cu_list_for_each_cast_safe(CURSOR_NAME, TMP_NAME, HEAD, MEMBER_NAME)\
	for ((CURSOR_NAME) = cu_container_of(\
			(HEAD)->next, CU_TYPEOF(CURSOR_NAME), MEMBER_NAME\
		), (TMP_NAME) = cu_container_of(\
			(CURSOR_NAME)->next, CU_TYPEOF(CURSOR_NAME), MEMBER_NAME\
		);\
		(CURSOR_NAME) != (HEAD);\
		(CURSOR_NAME) = (TMP_NAME), (TMP_NAME) = cu_container_of(\
			(TMP_NAME)->next, CU_TYPEOF(CURSOR_NAME), MEMBER_NAME\
		))

#define cu_list_for_each_cast_continue_safe(CURSOR_NAME, TMP_NAME, HEAD, MEMBER_NAME)\
	for ((CURSOR_NAME) = cu_container_of(\
			(CURSOR_NAME)->next, CU_TYPEOF(CURSOR_NAME), MEMBER_NAME\
		), (TMP_NAME) = cu_container_of(\
			(CURSOR_NAME)->next, CU_TYPEOF(CURSOR_NAME), MEMBER_NAME\
		);\
		(CURSOR_NAME) != (HEAD);\
		(CURSOR_NAME) = (TMP_NAME), (TMP_NAME) = cu_container_of(\
			(TMP_NAME)->next, CU_TYPEOF(CURSOR_NAME), MEMBER_NAME\
		))

// Reversed iterators; these only work for cu_dlist

#define cu_list_for_each_rev(CURSOR_NAME, HEAD)\
	for ((CURSOR_NAME) = (HEAD)->prev; (CURSOR_NAME) != (HEAD);\
		(CURSOR_NAME) = (CURSOR_NAME)->prev)
#define cu_list_for_each_continue_rev(CURSOR_NAME, HEAD)\
	for ((CURSOR_NAME) = (CURSOR_NAME)->prev; (CURSOR_NAME) != (HEAD);\
		(CURSOR_NAME) = (CURSOR_NAME)->prev)

#define cu_list_for_each_rev_safe(CURSOR_NAME, TMP_NAME, HEAD)\
	for ((CURSOR_NAME) = (HEAD)->prev, (TMP_NAME) = (CURSOR_NAME)->prev;\
		(CURSOR_NAME) != (HEAD);\
		(CURSOR_NAME) = (TMP_NAME), (TMP_NAME) = (TMP_NAME)->prev)
#define cu_list_for_each_continue_rev_safe(CURSOR_NAME, TMP_NAME, HEAD)\
	for ((CURSOR_NAME) = (CURSOR_NAME)->prev,\
			(TMP_NAME) = (CURSOR_NAME)->prev;\
		(CURSOR_NAME) != (HEAD);\
		(CURSOR_NAME) = (TMP_NAME), (TMP_NAME) = (TMP_NAME)->prev)

#define cu_list_for_each_cast_rev(CURSOR_NAME, HEAD, MEMBER_NAME)\
	for ((CURSOR_NAME) = cu_container_of(\
			(HEAD)->prev, CU_TYPEOF(CURSOR_NAME), MEMBER_NAME\
		); (CURSOR_NAME) != (HEAD);\
		(CURSOR_NAME) = cu_container_of(\
			(CURSOR_NAME)->MEMBER_NAME->prev, CU_TYPEOF(CURSOR_NAME), MEMBER_NAME\
		))
#define cu_list_for_each_cast_continue_rev(CURSOR_NAME, HEAD, MEMBER_NAME)\
	for ((CURSOR_NAME) = cu_container_of(\
			(CURSOR_NAME)->prev, CU_TYPEOF(CURSOR_NAME), MEMBER_NAME\
		); (CURSOR_NAME) != (HEAD);\
		(CURSOR_NAME) = cu_container_of(\
			(CURSOR_NAME)->MEMBER_NAME->prev, CU_TYPEOF(CURSOR_NAME), MEMBER_NAME\
		))

#define cu_list_for_each_cast_rev_safe(CURSOR_NAME, TMP_NAME, HEAD, MEMBER_NAME)\
	for ((CURSOR_NAME) = cu_container_of(\
			(HEAD)->prev, CU_TYPEOF(CURSOR_NAME), MEMBER_NAME\
		), (TMP_NAME) = cu_container_of(\
			(CURSOR_NAME)->prev, CU_TYPEOF(CURSOR_NAME), MEMBER_NAME\
		);\
		(CURSOR_NAME) != (HEAD);\
		(CURSOR_NAME) = (TMP_NAME), (TMP_NAME) = cu_container_of(\
			(TMP_NAME)->prev, CU_TYPEOF(CURSOR_NAME), MEMBER_NAME\
		))
#define cu_list_for_each_cast_continue_rev_safe(CURSOR_NAME, TMP_NAME, HEAD, MEMBER_NAME)\
	for ((CURSOR_NAME) = cu_container_of(\
			(CURSOR_NAME)->prev, CU_TYPEOF(CURSOR_NAME), MEMBER_NAME\
		), (TMP_NAME) = cu_container_of(\
			(CURSOR_NAME)->prev, CU_TYPEOF(CURSOR_NAME), MEMBER_NAME\
		);\
		(CURSOR_NAME) != (HEAD);\
		(CURSOR_NAME) = (TMP_NAME), (TMP_NAME) = cu_container_of(\
			(TMP_NAME)->prev, CU_TYPEOF(CURSOR_NAME), MEMBER_NAME\
		))

// Checks if there are elements in the list described by `HEAD`
// `HEAD` is a pointer to the list's head.
#define cu_list_empty(HEAD) ((HEAD) == (HEAD)->next)

static inline void cu_slist_poison(cu_slist *elem)
{
	assert(elem != NULL && "Cannot poison a null pointer");
	elem->next = NULL;
}

static inline void cu_dlist_poison(cu_dlist *elem)
{
	assert(elem != NULL && "Cannot poison a null pointer");
	elem->next = NULL;
	elem->prev = NULL;
}

// Poisons `ELEM`. Operations on a poisoned element will usually either
// segfault or fail assertions.
#define cu_list_poison(ELEM)\
	_Generic((ELEM),\
		cu_slist *: cu_slist_poison,\
		cu_dlist *: cu_dlist_poison\
	)((ELEM))

static inline bool cu_slist_is_poisoned(const cu_slist *elem)
{
	return elem->next == NULL;
}

static inline bool cu_dlist_is_poisoned(const cu_dlist *elem)
{
	return elem->next == NULL || elem->prev == NULL;
}

#define cu_list_is_poisoned(ELEM)\
	_Generic((ELEM),\
		cu_slist *: cu_slist_is_poisoned,\
		cu_dlist *: cu_dlist_is_poisoned\
	)((ELEM))


// Adds `new_el` to the list, immediately after the `slist` node.
//
// If `slist` is a list head, this will effectively add an element to the
// front of the list.
//
// Runs in O(1) time.
static inline void cu_slist_add(cu_slist *new_el, cu_slist *slist)
{
	assert(slist != NULL && "Cannot add to a null pointer");
	assert(new_el != NULL && "Cannot add a null pointer to a list");
	assert(slist->next != NULL
		&& "Used poisoned list element without reinitializing it");
	new_el->next = slist->next;
	slist->next = new_el;
}

// Same as `cu_slist_add`, but for a `cu_dlist`.
static inline void cu_dlist_add(cu_dlist *new_el, cu_dlist *dlist)
{
	assert(dlist != NULL && "Cannot add to a null pointer");
	assert(new_el != NULL && "Cannot add a null pointer to a list");
	assert(dlist->next != NULL && dlist->prev != NULL
		&& "Tried to add to a poisoned list");
	new_el->next = dlist->next;
	new_el->prev = dlist;

	new_el->next->prev = new_el;
	dlist->next = new_el;
}

// Generic macro, abstracting over the two list types
#define cu_list_add(NEW_EL, LIST)\
	_Generic((LIST),\
		cu_slist *: cu_slist_add,\
		cu_dlist *: cu_dlist_add\
	)((NEW_EL), (LIST))

// Adds `new_el` to the list, immediately after the `dlist` node.
// This only works for `cu_dlist`.
//
// If `dlist` is a list head, this will effectively add an element to the
// end of the list.
//
// Runs in O(1) time.
static inline void cu_list_add_prev(cu_dlist *dlist, cu_dlist *new_el)
{
	assert(dlist != NULL && "Cannot add to a null pointer");
	assert(new_el != NULL && "Cannot add a null pointer to a list");
	assert(dlist->next != NULL && dlist->prev != NULL
		&& "Tried to add to a poisoned list");
	
	new_el->next = dlist;
	new_el->prev = dlist->prev;

	new_el->prev->next = new_el;
	dlist->prev = new_el;
}

// Deletes the next element of the list.
//
// This is a slightly-inconvenient API but is necessary for singly-linked
// lists.
//
// If applied to a list head, this will effectively remove the first element
// of the list.
//
// The removed entry is "poisoned"; it is not itself a valid list.
// You can of course use `cu_list_init_head` to make it the head of a new list,
// or you can `cu_list_add` it to another list.
static inline void cu_slist_del_next(cu_slist *slist)
{
	assert(slist != NULL && "Cannot delete elements from a null pointer");
	assert(slist->next != NULL
		&& "Tried to delete from a poisoned list");
	assert(!cu_list_empty(slist)
		&& "Cannot delete from a list with no elements");
	cu_slist *el = slist->next;
	slist->next = el->next;

	cu_list_poison(el);
}

static inline void cu_dlist_del_next(cu_dlist *dlist)
{
	assert(dlist != NULL && "Cannot delete elements from a null pointer");
	assert(dlist->next != NULL && dlist->prev != NULL
		&& "Tried to delete from a poisoned list");
	assert(!cu_list_empty(dlist)
		&& "Cannot delete from a list with no elements");
	cu_dlist *el = dlist->next;
	dlist->next = el->next;
	el->next->prev = dlist;

	cu_list_poison(el);
}

#define cu_list_del_next(LIST)\
	_Generic((LIST),\
		cu_slist *: cu_slist_del_next,\
		cu_dlist *: cu_dlist_del_next\
	)((LIST))

// Removes `elem` from from its containing list.
// If `elem` is a list head, you are free to choose either the element before
// or after it to be the new list head.
//
// This function is only valid for `cu_dlist` elements.
//
// If a list head is deleted, it will simply become poisoned.
//
// `elem` itself is poisoned.
static inline void cu_list_del(cu_dlist *elem)
{
	assert(elem != NULL && "Cannot delete a null pointer");
	assert(elem->next != NULL && elem->prev != NULL
		&& "Tried to delete from a poisoned list");
	cu_dlist *prev = elem->prev;
	cu_dlist *next = elem->next;
	prev->next = next;
	next->prev = prev;

	cu_list_poison(elem);
}

// Deletes the element immediately before `list`.
//
// If `list` is a list head, this effectively removes the last element of the
// list.
//
// The removed element is poisoned.
static inline void cu_list_del_prev(cu_dlist *list)
{
	assert(list != NULL && "Cannot delete elements from a null pointer");
	assert(list->next != NULL && list->prev != NULL
		&& "Tried to delete from a poisoned list");
	assert(!cu_list_empty(list)
		&& "Cannot delete from a list with no elements");
	cu_dlist *el = list->prev;
	list->prev = el->prev;
	el->prev->next = list;

	cu_list_poison(el);
}

// Swaps the next elements in `l1` and `l2`.
static inline void cu_slist_swap_next(cu_slist *l1, cu_slist *l2)
{
	assert(l1 != NULL && "Cannot swap null pointer");
	assert(l2 != NULL && "Cannot swap null pointer");

	assert(!cu_list_is_poisoned(l1) && "Tried to swap poisoned list");
	assert(!cu_list_is_poisoned(l2) && "Tried to swap poisoned list");

	assert(!cu_list_empty(l1) && "Cannot swap next element of empty list");
	assert(!cu_list_empty(l2) && "Cannot swap next element of empty list");

	cu_slist *l1_elem = l1->next;
	cu_slist *l2_elem = l2->next;

	cu_list_del_next(l1);
	cu_list_del_next(l2);

	cu_list_add(l2_elem, l1);
	cu_list_add(l1_elem, l2);
}

static inline void cu_dlist_swap_next(cu_dlist *l1, cu_dlist *l2)
{
	assert(l1 != NULL && "Cannot swap null pointer");
	assert(l2 != NULL && "Cannot swap null pointer");

	assert(!cu_list_is_poisoned(l1) && "Tried to swap poisoned list");
	assert(!cu_list_is_poisoned(l2) && "Tried to swap poisoned list");

	assert(!cu_list_empty(l1) && "Cannot swap next element of empty list");
	assert(!cu_list_empty(l2) && "Cannot swap next element of empty list");

	cu_dlist *l1_elem = l1->next;
	cu_dlist *l2_elem = l2->next;

	cu_list_del_next(l1);
	cu_list_del_next(l2);

	cu_list_add(l1, l2_elem);
	cu_list_add(l2, l1_elem);
}

#define cu_list_swap_next(L1, L2)\
	_Generic((L1),\
		cu_slist *: cu_slist_swap_next,\
		cu_dlist *: cu_dlist_swap_next\
	)((L1), (L2))

static inline void cu_list_swap(cu_dlist *l1, cu_dlist *l2)
{
	assert(l1 != NULL && "Cannot swap null pointer");
	assert(l2 != NULL && "Cannot swap null pointer");

	assert(!cu_list_is_poisoned(l1) && "Tried to swap poisoned list");
	assert(!cu_list_is_poisoned(l2) && "Tried to swap poisoned list");

	if (cu_list_empty(l1) && cu_list_empty(l2))
		return;
	if (cu_list_empty(l1)) {
		cu_list_add(l2, l1);
		cu_list_del(l2);
		cu_list_init_head(l2);
	}
	else if (cu_list_empty(l2)) {
		cu_list_add(l1, l2);
		cu_list_del(l1);
		cu_list_init_head(l1);
	}
	else {
		cu_list_swap_next(l1->prev, l2->prev);
	}
}

static inline void cu_list_swap_prev(cu_dlist *l1, cu_dlist *l2)
{
	assert(l1 != NULL && "Cannot swap null pointer");
	assert(l2 != NULL && "Cannot swap null pointer");

	assert(!cu_list_is_poisoned(l1) && "Tried to swap poisoned list");
	assert(!cu_list_is_poisoned(l2) && "Tried to swap poisoned list");

	assert(!cu_list_empty(l1)
		&& "Cannot swap previous element of empty list");
	assert(!cu_list_empty(l2)
		&& "Cannot swap previous element of empty list");

	cu_dlist *l1_elem = l1->prev;
	cu_dlist *l2_elem = l2->prev;

	cu_list_del_prev(l1);
	cu_list_del_prev(l2);

	cu_list_add_prev(l1, l2_elem);
	cu_list_add_prev(l2, l1_elem);
}


#endif // CU_LIST_H
