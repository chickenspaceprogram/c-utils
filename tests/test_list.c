// This Source Code Form is subject to the terms of the Mozilla Public
// License, v. 2.0. If a copy of the MPL was not distributed with this
// file, You can obtain one at https://mozilla.org/MPL/2.0/.

// SPDX-License-Identifier: MPL-2.0

#include <cu/list.h>
#include <cu/dbgassert.h>

typedef struct {
	int flag_val;
	cu_slist list;
} slist_container;

typedef struct {
	int flag_val;
	cu_dlist list;
} dlist_container;

#define BREAK_VAL 10
#define LIST_NEL 30


cu_slist SLIST_HEAD;
cu_dlist DLIST_HEAD;
static slist_container SLIST_STORAGE[LIST_NEL];
static dlist_container DLIST_STORAGE[LIST_NEL];


static void test_slist_init(void)
{
	cu_list_poison(&SLIST_HEAD);
	dbgassert(SLIST_HEAD.next == NULL && "cu_slist_poison failed");

	cu_list_init_head(&SLIST_HEAD);
	dbgassert(SLIST_HEAD.next == &SLIST_HEAD && "cu_slist_init_head failed");

	
	cu_slist *cur_sl = &SLIST_HEAD;
	for (int i = 0; i < LIST_NEL; ++i) {
		cu_list_add(&(SLIST_STORAGE[i].list), cur_sl);
		dbgassert(cur_sl != cur_sl->next
			&& cur_sl->next != &SLIST_HEAD
			&& "cu_slist_add failed");
		cur_sl = cur_sl->next;
	}
}
static void test_dlist_init(void)
{
	cu_list_poison(&DLIST_HEAD);
	dbgassert(DLIST_HEAD.next == NULL && "cu_dlist_poison failed");

	cu_list_init_head(&DLIST_HEAD);
	dbgassert(DLIST_HEAD.next == &DLIST_HEAD && "cu_dlist_init_head failed");

	
	cu_dlist *cur_dl = &DLIST_HEAD;
	for (int i = 0; i < LIST_NEL; ++i) {
		cu_list_add(&(DLIST_STORAGE[i].list), cur_dl);
		dbgassert(cur_dl != cur_dl->next
			&& cur_dl->next != &DLIST_HEAD
			&& "cu_dlist_add failed");
		cur_dl = cur_dl->next;
	}
}
static void test_slist_iter(void)
{
	for (int i = 0; i < LIST_NEL; ++i) {
		SLIST_STORAGE[i].flag_val = i;
	}
	test_slist_init();

	int i = 0;
	cu_slist *cur_sl = NULL;
	cu_list_for_each(cur_sl, &SLIST_HEAD) {
		slist_container *cur = cu_container_of(cur_sl,
			slist_container, list);
		dbgassert(cur->flag_val == i
			&& "bad flag from cu_list_for_each");
		if (i++ == BREAK_VAL)
			break;
	}
	cu_list_for_each_continue(cur_sl, &SLIST_HEAD) {
		slist_container *cur = cu_container_of(cur_sl,
			slist_container, list);
		dbgassert(cur->flag_val == i++
			&& "bad flag from cu_list_for_each_continue");
	}
	i = 0;
	cur_sl = NULL;
	cu_slist *tmp_sl = NULL;
	cu_list_for_each_safe(cur_sl, tmp_sl, &SLIST_HEAD) {
		slist_container *cur = cu_container_of(cur_sl,
			slist_container, list);
		dbgassert(cur->flag_val == i
			&& "bad flag from cu_list_for_each_safe");
		if (i++ == BREAK_VAL)
			break;
		cu_list_poison(cur_sl);
		cur_sl = NULL;
	}
	cu_list_for_each_continue_safe(cur_sl, tmp_sl, &SLIST_HEAD) {
		slist_container *cur = cu_container_of(cur_sl,
			slist_container, list);
		dbgassert(cur->flag_val == i++
			&& "bad flag from cu_list_for_each_continue_safe");
		cu_list_poison(cur_sl);
		cur_sl = NULL;
	}
	test_slist_init();
	slist_container *cur_elem = NULL;
	slist_container *tmp_elem = NULL;
	i = 0;
	cu_list_for_each_cast_safe(cur_elem, tmp_elem, &SLIST_HEAD, list) {
		dbgassert(cur_elem->flag_val == i
			&& "bad flag from cu_list_for_each");
		if (i++ == BREAK_VAL)
			break;
		cu_list_poison(&(cur_elem->list));
		cur_elem = NULL;
	}
	cu_list_for_each_cast_continue_safe(cur_elem, tmp_elem, &SLIST_HEAD, list) {
		dbgassert(cur_elem->flag_val == i++
			&& "bad flag from cu_list_for_each");
		cu_list_poison(&(cur_elem->list));
		cur_elem = NULL;
	}

}

static void test_dlist_iter(void)
{
	for (int i = 0; i < LIST_NEL; ++i) {
		DLIST_STORAGE[i].flag_val = i;
	}
	test_dlist_init();

	int i = 0;
	cu_dlist *cur_dl = NULL;
	cu_list_for_each(cur_dl, &DLIST_HEAD) {
		dlist_container *cur = cu_container_of(cur_dl,
			dlist_container, list);
		dbgassert(cur->flag_val == i
			&& "bad flag from cu_list_for_each");
		if (i++ == BREAK_VAL)
			break;
	}
	cu_list_for_each_continue(cur_dl, &DLIST_HEAD) {
		dlist_container *cur = cu_container_of(cur_dl,
			dlist_container, list);
		dbgassert(cur->flag_val == i++
			&& "bad flag from cu_list_for_each_continue");
	}
	i = 0;
	cur_dl = NULL;
	cu_dlist *tmp_dl = NULL;
	cu_list_for_each_safe(cur_dl, tmp_dl, &DLIST_HEAD) {
		dlist_container *cur = cu_container_of(cur_dl,
			dlist_container, list);
		dbgassert(cur->flag_val == i
			&& "bad flag from cu_list_for_each_safe");
		if (i++ == BREAK_VAL)
			break;
		cu_list_poison(cur_dl);
		cur_dl = NULL;
	}
	cu_list_for_each_continue_safe(cur_dl, tmp_dl, &DLIST_HEAD) {
		dlist_container *cur = cu_container_of(cur_dl,
			dlist_container, list);
		dbgassert(cur->flag_val == i++
			&& "bad flag from cu_list_for_each_continue_safe");
		cu_list_poison(cur_dl);
		cur_dl = NULL;
	}
	test_dlist_init();
	dlist_container *cur_elem = NULL;
	dlist_container *tmp_elem = NULL;
	i = 0;
	cu_list_for_each_cast_safe(cur_elem, tmp_elem, &DLIST_HEAD, list) {
		dbgassert(cur_elem->flag_val == i
			&& "bad flag from cu_list_for_each");
		if (i++ == BREAK_VAL)
			break;
		cu_list_poison(&(cur_elem->list));
		cur_elem = NULL;
	}
	cu_list_for_each_cast_continue_safe(cur_elem, tmp_elem, &DLIST_HEAD, list) {
		dbgassert(cur_elem->flag_val == i++
			&& "bad flag from cu_list_for_each");
		cu_list_poison(&(cur_elem->list));
		cur_elem = NULL;
	}

}

static void test_dlist_iter_rev(void)
{
	for (int i = 0; i < LIST_NEL; ++i) {
		DLIST_STORAGE[i].flag_val = i;
	}
	test_dlist_init();

	int i = 29;
	cu_dlist *cur_dl = NULL;
	cu_list_for_each_rev(cur_dl, &DLIST_HEAD) {
		dlist_container *cur = cu_container_of(cur_dl,
			dlist_container, list);
		dbgassert(cur->flag_val == i
			&& "bad flag from cu_list_for_each");
		if (i-- == BREAK_VAL)
			break;
	}
	cu_list_for_each_continue_rev(cur_dl, &DLIST_HEAD) {
		dlist_container *cur = cu_container_of(cur_dl,
			dlist_container, list);
		dbgassert(cur->flag_val == i--
			&& "bad flag from cu_list_for_each_continue");
	}
	i = 29;
	cur_dl = NULL;
	cu_dlist *tmp_dl = NULL;
	cu_list_for_each_rev_safe(cur_dl, tmp_dl, &DLIST_HEAD) {
		dlist_container *cur = cu_container_of(cur_dl,
			dlist_container, list);
		dbgassert(cur->flag_val == i
			&& "bad flag from cu_list_for_each_safe");
		if (i-- == BREAK_VAL)
			break;
		cu_list_poison(cur_dl);
		cur_dl = NULL;
	}
	cu_list_for_each_continue_rev_safe(cur_dl, tmp_dl, &DLIST_HEAD) {
		dlist_container *cur = cu_container_of(cur_dl,
			dlist_container, list);
		dbgassert(cur->flag_val == i--
			&& "bad flag from cu_list_for_each_continue_safe");
		cu_list_poison(cur_dl);
		cur_dl = NULL;
	}
	test_dlist_init();
	dlist_container *cur_elem = NULL;
	dlist_container *tmp_elem = NULL;
	i = 29;
	cu_list_for_each_cast_rev_safe(cur_elem, tmp_elem, &DLIST_HEAD, list) {
		dbgassert(cur_elem->flag_val == i
			&& "bad flag from cu_list_for_each");
		if (i-- == BREAK_VAL)
			break;
		cu_list_poison(&(cur_elem->list));
		cur_elem = NULL;
	}
	cu_list_for_each_cast_continue_rev_safe(cur_elem, tmp_elem, &DLIST_HEAD, list) {
		dbgassert(cur_elem->flag_val == i--
			&& "bad flag from cu_list_for_each");
		cu_list_poison(&(cur_elem->list));
		cur_elem = NULL;
	}


}


int main(void)
{
	test_slist_iter();
	test_dlist_iter();
	test_dlist_iter_rev();
}
