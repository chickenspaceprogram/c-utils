#pragma once

#include <cu/minheap.h>
#include <cu/sync.h>

#define CU_BLOCK_PRI_QUEUE_TYPE(TYPENAME)\
struct {\
	CU_MINHEAP_TYPE(TYPENAME) deque;\
	cu_sem sem;\
	int tmp_storage;\
}
