#include "hw_malloc.h"
#include <stdio.h>
#include <stdlib.h>

/*Global Variable*/
bool has_init = false;
void *start_brk = NULL;
void *heap_brk = NULL;
bin_t s_bin[7] = {};
bin_t *bin[7];
int slice_num = 1;
chunk_header *top[2];
/*Static function*/
static chunk_header *create_chunk(chunk_header *ori, const chunk_size_t need);
static chunk_header *split(chunk_header **ori, const chunk_size_t need);
static chunk_header *merge(chunk_header *h);
static int search_debin(const chunk_size_t need);
static int search_enbin(const chunk_size_t need);
static void en_bin(const int index, chunk_header *c_h);
static chunk_header *de_bin(const int index, const chunk_size_t need);
static int check_valid_free(const void *mem);

void *hw_malloc(size_t bytes)
{
	long long need = bytes + 40LL + (bytes % 8 != 0 ? (8 - (bytes % 8)) : 0);
	if (!has_init) {
		has_init = true;
		for (int i = 0; i < 7; i++) {
			bin[i] = &s_bin[i];
			bin[i]->prev = bin[i];
			bin[i]->next = bin[i];
			bin[i]->size = 0;
		}
		start_brk = sbrk(64 * 1024);
		top[0] = sbrk(40);
		top[1] = sbrk(40);
		top[0]->prev = NULL;
		top[0]->next = NULL;
		top[0]->chunk_size = 40;
		top[0]->prev_chunk_size = 64 * 1024;
		top[0]->prev_free_flag = 0;
		top[1]->prev = NULL;
		top[1]->next = NULL;
		top[1]->chunk_size = 40;
		top[1]->prev_chunk_size = 40;
		top[1]->prev_free_flag = 0;
		heap_brk = start_brk;
		chunk_header *s = create_chunk(start_brk, 64 * 1024);
		heap_brk = start_brk; // reset heap top pointer
		chunk_header *c = split(&s, need);
		return (void *)((intptr_t)(void*)c +
		                sizeof(chunk_header) -
		                (intptr_t)(void*)start_brk);
	} else {
		chunk_header *r = NULL;
		int bin_num = search_debin(need);
		if (bin_num == -1) {
		} else if (bin_num <= 5) {
			r = de_bin(bin_num, need);
			return (void *)((intptr_t)(void*)r +
			                sizeof(chunk_header) -
			                (intptr_t)(void*)start_brk);
		} else { // bin_num = 6
			chunk_header *s = de_bin(6, need);
			if (s == NULL) {
				PRINTERR("bin[6] NULL\n");
				return NULL; // XXX
			}
			chunk_header *c = split(&s, need);
			if (c == NULL) {
				PRINTERR("NULL after split\n");
				return NULL;
			}
			return (void *)((intptr_t)(void*)c +
			                sizeof(chunk_header) -
			                (intptr_t)(void*)start_brk);
		}
	}
	return NULL;
}

int hw_free(void *mem)
{
	void *a_mem = (void *)((intptr_t)(void*)mem +
	                       (intptr_t)(void*)start_brk);
	if (!has_init || !check_valid_free(a_mem)) {
		return 0;
	} else {
		// TODO if free the top one
		chunk_header *h = (chunk_header *)((intptr_t)(void*)a_mem -
		                                   (intptr_t)(void*)sizeof(chunk_header));
		chunk_header *nxt = (chunk_header *)((intptr_t)(void*)h +
		                                     (intptr_t)(void*)((chunk_header *)h)->chunk_size);
		nxt->prev_free_flag = 1;
		chunk_header *m = merge(h);
		nxt = (chunk_header *)((intptr_t)(void*)m +
		                       (intptr_t)(void*)((chunk_header *)m)->chunk_size);
		if (nxt == top[0]) {
			heap_brk = start_brk + 1024 * 64 - m->chunk_size;
		}
		en_bin(search_enbin(m->chunk_size), m);
		return 1;
	}
}

void *get_start_sbrk(void)
{
	return (void *)start_brk;
}

void show_bin(const int i)
{
	if (!has_init) {
		return;
	}
	// printf("bin size: %d\n", bin[i]->size);
	chunk_header *cur = bin[i]->next;
	if (bin[i]->size == 0 || cur == NULL) {
		return;
	}
	while ((void *)cur != (void *)bin[i]) {
		void *r_cur = (void *)((intptr_t)(void*)cur -
		                       (intptr_t)(void*)start_brk);
		printf("0x%08" PRIxPTR "--------%lld\n", (uintptr_t)r_cur, cur->chunk_size);
		cur = cur->next;
	}
}

static chunk_header *create_chunk(chunk_header *ori, const chunk_size_t need)
{
	// if (heap_brk - start_brk + need > 64 * 1024 + 40) {
	if ((void *)ori - start_brk + need > 64 * 1024 + 40) {
		// printf("-+-%lld\n", need);
		PRINTERR("heap not enough\n");
		return NULL;
	}
	// chunk_header *ret = heap_brk;
	chunk_header *ret = ori;
	if (ori == heap_brk) {
		// printf("y\n");
		heap_brk += need;
	}
	ret->chunk_size = need;
	ret->prev = NULL;
	ret->next = NULL;
	return ret;
}

static chunk_header *split(chunk_header **ori, const chunk_size_t need)
{
	if ((*ori)->chunk_size - need >= 48) {
		void *base = *ori;
		if ((*ori) == top[0] - top[0]->prev_chunk_size) {
			top[0]->prev_chunk_size -= need;
		} else {
			chunk_header *nxt = (chunk_header *)((intptr_t)(void*)base +
			                                     (intptr_t)(void*)((chunk_header *)base)->chunk_size);
			nxt->prev_chunk_size -= need;
		}
		// void *base = *ori;
		chunk_header *new = (void *)((intptr_t)(void*)*ori + need);
		new->chunk_size = (*ori)->chunk_size - need;
		new->prev_chunk_size = need;
		new->prev_free_flag = 0;
		*ori = new;
		// printf("%p, %p\n", base, heap_brk);
		chunk_header *ret = create_chunk((base), need);
		en_bin(search_enbin((*ori)->chunk_size), (*ori));
		slice_num++;
		return ret;
	} else {
		heap_brk += (*ori)->chunk_size;
		chunk_header *nxt = (chunk_header *)((intptr_t)(void*)(*ori) +
		                                     (intptr_t)(void*)((chunk_header *)(*ori))->chunk_size);
		nxt->prev_free_flag = 0;
		return (*ori);
	}
}

static chunk_header *merge(chunk_header *h)
{
	chunk_header *nxt = (chunk_header *)((intptr_t)(void*)h +
	                                     (intptr_t)(void*)((chunk_header *)h)->chunk_size);
	chunk_header *nnxt = (chunk_header *)((intptr_t)(void*)nxt +
	                                      (intptr_t)(void*)((chunk_header *)nxt)->chunk_size);
	if (nnxt->prev_free_flag == 1) {
		nnxt->prev_chunk_size += h->chunk_size;
		if ((chunk_header *)nxt->prev != NULL && (chunk_header *)nxt->next != NULL) {
			((chunk_header *)nxt->prev)->next = nxt->next;
			((chunk_header *)nxt->next)->prev = nxt->prev;
		}
		bin[search_enbin(nxt->chunk_size)]->size--;
		h->chunk_size += nxt->chunk_size;
		nxt->chunk_size = 0;
		nxt->prev = NULL;
		nxt->next = NULL;
	}
	if (h->prev_free_flag == 1) {
		chunk_header *nxt = (chunk_header *)((intptr_t)(void*)h +
		                                     (intptr_t)(void*)((chunk_header *)h)->chunk_size);
		chunk_header *pre = (chunk_header *)((intptr_t)(void*)h -
		                                     (intptr_t)(void*)((chunk_header *)h)->prev_chunk_size);
		nxt->prev_chunk_size += pre->chunk_size;
		if ((chunk_header *)pre->prev != NULL && (chunk_header *)pre->next != NULL) {
			((chunk_header *)pre->prev)->next = pre->next;
			((chunk_header *)pre->next)->prev = pre->prev;
		}
		bin[search_enbin(pre->chunk_size)]->size--;
		pre->chunk_size += h->chunk_size;
		h->chunk_size = 0;
		pre->prev = NULL;
		pre->next = NULL;
		h->prev = NULL;
		h->next = NULL;
		return pre;
	} else {
		h->prev = NULL;
		h->next = NULL;
		return h;
	}
}

static int search_debin(const chunk_size_t need)
{
	for (int i = 0; i <= 5; i++) {
		if (bin[i]->size == 0) {
			continue;
		}
		if (need <= 40 + (i + 1) * 8) {
			return i;
		}
	}
	if (bin[6]->size > 0) {
		return 6;
	} else {
		PRINTERR("not any free chunk\n");
		return -1;
	}
}

static int search_enbin(const chunk_size_t size)
{
	switch (size) {
	case 48:
		return 0;
	case 56:
		return 1;
	case 64:
		return 2;
	case 72:
		return 3;
	case 80:
		return 4;
	case 88:
		return 5;
	default:
		return 6;
	}
}

static void en_bin(const int index, chunk_header *c_h)
{
	if (bin[index]->size == 0) {
		bin[index]->next = c_h;
		c_h->prev = bin[index];
		bin[index]->prev = c_h;
		c_h->next = bin[index];
	} else {
		chunk_header *tmp;
		chunk_header *cur;
		switch (index) {
		case 0:
		case 1:
		case 2:
		case 3:
		case 4:
		case 5:
			tmp = bin[index]->prev;
			bin[index]->prev = c_h;
			c_h->next = bin[index];
			tmp->next = c_h;
			c_h->prev = tmp;
			break;
		case 6:
			if (bin[6]->size > 0 &&
			    c_h->chunk_size > ((chunk_header *)bin[6]->next)->chunk_size) {
				tmp = bin[index]->next;
				bin[index]->next = c_h;
				c_h->prev = bin[index];
				tmp->prev = c_h;
				c_h->next = tmp;
			} else {
				cur = bin[6]->prev;
				while ((void *)cur != (void *)bin[6]) {
					if (c_h->chunk_size <= cur->chunk_size) {
						tmp = cur->next;
						cur->next = c_h;
						c_h->prev = cur;
						tmp->prev = c_h;
						c_h->next = tmp;
						break;
					}
					cur = cur->prev;
				}
			}
			break;
		}
	}
	bin[index]->size++;
}

static chunk_header *de_bin(const int index, const chunk_size_t need)
{
	if (bin[index]->size == 0) {
		PRINTERR("size = 0\n");
		return NULL;
	} else {
		chunk_header *ret;
		chunk_header *cur;
		switch (index) {
		case 0:
		case 1:
		case 2:
		case 3:
		case 4:
		case 5:
			ret = bin[index]->next;
			if (bin[index]->size == 1) {
				bin[index]->next = bin[index];
				bin[index]->prev = bin[index];
			} else {
				bin[index]->next = ret->next;
				((chunk_header *)ret->next)->prev = bin[index];
			}
			ret->prev = NULL;
			ret->next = NULL;
			bin[index]->size--;
			return ret;
		case 6:
			if (bin[6]->size > 0 &&
			    need > ((chunk_header *)bin[6]->next)->chunk_size) {
				PRINTERR("not enough bin\n");
				return NULL;
			} else {
				cur = bin[6]->prev;
				while (cur != (void *)bin[6]) {
					if (need <= cur->chunk_size) {
						if (((chunk_header *)cur->prev)->chunk_size == cur->chunk_size) {
							cur = cur->prev;
							continue;
						}
						ret = cur;
						if (cur->prev == bin[6]) {
							((bin_t *)cur->prev)->next = cur->next;
						} else {
							((chunk_header *)cur->prev)->next = cur->next;
						}
						if (cur->next == bin[6]) {
							((bin_t *)cur->next)->prev = cur->prev;
						} else {
							((chunk_header *)cur->next)->prev = cur->prev;
						}
						ret->prev = NULL;
						ret->next = NULL;
						bin[index]->size--;
						return ret;
					}
					cur = cur->prev;
				}
			}
		}
		PRINTERR("de bin error\n");
		return NULL; //TODO should not reach here
	}
}

void watch_heap()
{
	chunk_header *cur = start_brk;
	int count = 0;
	printf("slice: %d\n", slice_num);
	while (count++ < slice_num + 1) {
		printf("----------\n");
		printf("0x%08" PRIxPTR "(",
		       (uintptr_t)(void *)((intptr_t)(void*)cur - (intptr_t)(void*)start_brk));
		printf("0x%08" PRIxPTR ")\n",
		       (uintptr_t)(void *)cur);
		printf("chun_size:%lld\n", cur->chunk_size);
		printf("prev_size:%lld\n", cur->prev_chunk_size);
		printf("prev_free:%lld\n", cur->prev_free_flag);
		cur = (void *)((intptr_t)(void*)cur + (intptr_t)(void*)cur->chunk_size);
	}
}

static int check_valid_free(const void *a_mem)
{
	chunk_header *cur = start_brk;
	int count = 0;
	while (count++ < slice_num + 1) {
		if ((intptr_t)(void*)cur > (intptr_t)(void*)a_mem - 40) {
			return 0;
		}
		if (cur == a_mem - 40) {
			// TODO check if free the top one
			void *nxt;
			nxt = (void *)((intptr_t)(void*)cur +
			               (intptr_t)(void*)cur->chunk_size);
			if ((intptr_t)(void*)nxt - (intptr_t)(void*)start_brk <= 65536 &&
			    ((chunk_header *)nxt)->prev_free_flag == 0) {
				return 1;
			} else {
				return 0;
			}
		}
		cur = (void *)((intptr_t)(void*)cur + (intptr_t)(void*)cur->chunk_size);
	}
	return 0;
}
