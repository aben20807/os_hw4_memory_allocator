#include "hw_malloc.h"
#include <stdio.h>
#include <stdlib.h>

/*Global Variable*/
bool has_init = false;
void *start_brk = NULL;
bin_t s_bin[7] = {};
bin_t *bin[7];
int slice_num = 1; // count the number of chunk
chunk_header *top[2];
pthread_mutex_t mutex = PTHREAD_MUTEX_INITIALIZER;

/*Static function*/
static chunk_header *create_chunk(chunk_header *base, const chunk_size_t need);
static chunk_header *split(chunk_header **ori, const chunk_size_t need);
static chunk_header *merge(chunk_header *h);
static int search_debin(const chunk_size_t need);
static int search_enbin(const chunk_size_t need);
static void en_bin(const int index, chunk_header *c_h);
static chunk_header *de_bin(const int index, const chunk_size_t need);
static void rm_chunk_from_bin(chunk_header *c);
static int check_valid_free(const void *mem);

void *hw_malloc(size_t bytes)
{
	pthread_mutex_lock(&mutex);
	/*Make need = bytes + 40 and be multiple of 8 bytes*/
	chunk_size_t need = bytes + 40LL + (bytes % 8 != 0 ? (8 - (bytes % 8)) : 0);
	if (!has_init) {
		has_init = true;
		if (need > 64 * 1024) {
			PRINTERR("not enough space\n");
			has_init = false;
			pthread_mutex_unlock(&mutex);
			return NULL;
		}
		for (int i = 0; i < 7; i++) {
			bin[i] = &s_bin[i];
			bin[i]->prev = bin[i];
			bin[i]->next = bin[i];
			bin[i]->size = 0;
		}
		start_brk = sbrk(64 * 1024);
		/*Create two chunk_header on start_brk + 64 * 1024*/
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
		chunk_header *s = create_chunk(get_start_brk(), 64 * 1024);
		chunk_header *c = split(&s, need);
		pthread_mutex_unlock(&mutex);
		return (void *)((intptr_t)(void*)c +
		                sizeof(chunk_header) -
		                (intptr_t)(void*)get_start_brk());
	} else {
		chunk_header *r = NULL;
		int bin_num = search_debin(need);
		if (bin_num == -1) {
			PRINTERR("search debin error\n");
		} else if (bin_num <= 5) {
			r = de_bin(bin_num, need);
			pthread_mutex_unlock(&mutex);
			return (void *)((intptr_t)(void*)r +
			                sizeof(chunk_header) -
			                (intptr_t)(void*)get_start_brk());
		} else { // bin_num = 6
			chunk_header *s = de_bin(6, need);
			if (s == NULL) {
				PRINTERR("bin[6] NULL\n");
				pthread_mutex_unlock(&mutex);
				return NULL;
			}
			chunk_header *c = split(&s, need);
			if (c == NULL) {
				PRINTERR("NULL after split\n");
				pthread_mutex_unlock(&mutex);
				return NULL;
			}
			pthread_mutex_unlock(&mutex);
			return (void *)((intptr_t)(void*)c +
			                sizeof(chunk_header) -
			                (intptr_t)(void*)get_start_brk());
		}
	}
	pthread_mutex_unlock(&mutex);
	return NULL;
}

int hw_free(void *mem)
{
	pthread_mutex_lock(&mutex);
	void *a_mem = (void *)((intptr_t)(void*)mem +
	                       (intptr_t)(void*)get_start_brk());
	if (!has_init || !check_valid_free(a_mem)) {
		pthread_mutex_unlock(&mutex);
		return 0;
	} else {
		chunk_header *h = (chunk_header *)((intptr_t)(void*)a_mem -
		                                   (intptr_t)(void*)sizeof(chunk_header));
		chunk_header *nxt = (chunk_header *)((intptr_t)(void*)h +
		                                     (intptr_t)(void*)((chunk_header *)h)->chunk_size);
		nxt->prev_free_flag = 1;
		chunk_header *m = merge(h);
		en_bin(search_enbin(m->chunk_size), m);
		pthread_mutex_unlock(&mutex);
		return 1;
	}
}

void *get_start_brk(void)
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
	while ((void *)cur != (void *)bin[i]) {
		void *r_cur = (void *)((intptr_t)(void*)cur -
		                       (intptr_t)(void*)get_start_brk());
		printf("0x%08" PRIxPTR "--------%lld\n", (uintptr_t)r_cur, cur->chunk_size);
		cur = cur->next;
	}
}

static chunk_header *create_chunk(chunk_header *base, const chunk_size_t need)
{
	if ((void *)base - get_start_brk() + need > 64 * 1024) {
		PRINTERR("heap not enough\n");
		return NULL;
	}
	chunk_header *ret = base;
	ret->chunk_size = need;
	ret->prev = NULL;
	ret->next = NULL;
	return ret;
}

static chunk_header *split(chunk_header **ori, const chunk_size_t need)
{
	if ((*ori)->chunk_size - need >= 48) {
		chunk_header *base = *ori;
		/*Change next chunk's prev_chunk_size*/
		chunk_header *nxt = (chunk_header *)((intptr_t)(void*)base +
		                                     (intptr_t)(void*)((chunk_header *)base)->chunk_size);
		nxt->prev_chunk_size -= need;
		/*Create upper chunk by shifting need*/
		chunk_header *new = (void *)((intptr_t)(void*)base + need);
		new->chunk_size = (base)->chunk_size - need;
		new->prev_chunk_size = need;
		new->prev_free_flag = 0;
		*ori = new;
		chunk_header *ret = create_chunk(base, need);
		/*Insert upper chunk into bin*/
		en_bin(search_enbin((*ori)->chunk_size), (*ori));
		slice_num++;
		return ret;
	} else {
		/*If chunk size is not enough to split, return whole chunk*/
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
		/*If next chunk is free, being able to merge*/
		nnxt->prev_chunk_size += h->chunk_size;
		rm_chunk_from_bin(nxt);
		bin[search_enbin(nxt->chunk_size)]->size--;
		h->chunk_size += nxt->chunk_size;
		nxt->chunk_size = 0;
	}
	if (h->prev_free_flag == 1) {
		chunk_header *nxt = (chunk_header *)((intptr_t)(void*)h +
		                                     (intptr_t)(void*)((chunk_header *)h)->chunk_size);
		chunk_header *pre = (chunk_header *)((intptr_t)(void*)h -
		                                     (intptr_t)(void*)((chunk_header *)h)->prev_chunk_size);
		nxt->prev_chunk_size += pre->chunk_size;
		rm_chunk_from_bin(pre);
		bin[search_enbin(pre->chunk_size)]->size--;
		pre->chunk_size += h->chunk_size;
		h->chunk_size = 0;
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
	case 56:
	case 64:
	case 72:
	case 80:
	case 88:
		return ((size - 40) / 8) - 1;
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
			rm_chunk_from_bin(ret);
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
						rm_chunk_from_bin(cur);
						bin[index]->size--;
						return ret;
					}
					cur = cur->prev;
				}
			}
		}
		PRINTERR("de bin error\n");
		return NULL;
	}
}

static void rm_chunk_from_bin(chunk_header *c)
{
	/*Used to reconnect linked list when removing a chunk*/
	if (c->prev == bin[0] || c->prev == bin[1] ||
	    c->prev == bin[2] || c->prev == bin[3] ||
	    c->prev == bin[4] || c->prev == bin[5] ||
	    c->prev == bin[6]
	   ) {
		((bin_t *)c->prev)->next = c->next;
	} else {
		((chunk_header *)c->prev)->next = c->next;
	}
	if (c->next == bin[0] || c->next == bin[1] ||
	    c->next == bin[2] || c->next == bin[3] ||
	    c->next == bin[4] || c->next == bin[5] ||
	    c->next == bin[6]
	   ) {
		((bin_t *)c->next)->prev = c->prev;
	} else {
		((chunk_header *)c->next)->prev = c->prev;
	}
	c->prev = NULL;
	c->next = NULL;
}

void watch_heap()
{
	chunk_header *cur = get_start_brk();
	int count = 0;
	printf("slice: %d\n", slice_num);
	while (count++ < slice_num + 1) {
		printf("----------\n");
		printf("0x%08" PRIxPTR "(",
		       (uintptr_t)(void *)((intptr_t)(void*)cur - (intptr_t)(void*)get_start_brk()));
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
	chunk_header *cur = get_start_brk();
	int count = 0;
	while (count++ < slice_num + 1) {
		if ((intptr_t)(void*)cur > (intptr_t)(void*)a_mem - 40) {
			return 0;
		}
		if (cur == a_mem - 40) {
			void *nxt;
			nxt = (void *)((intptr_t)(void*)cur +
			               (intptr_t)(void*)cur->chunk_size);
			if ((intptr_t)(void*)nxt - (intptr_t)(void*)get_start_brk() <= 65536 &&
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
