#include "hw_malloc.h"
#include <stdio.h>
#include <stdlib.h>

/*Global Variable*/
bool has_init = false;
void *start_brk = NULL;
void *heap_brk = NULL;
bin_t s_bin[7] = {};
bin_t *bin[7];
/*Static function*/
static chunk_header *create_chunk(const chunk_size_t size);
static chunk_header *split(chunk_header **ori, const chunk_size_t need);
static int search_bin(const chunk_size_t need);
static void en_bin(const int bin_num, chunk_header *c_h);
static chunk_header *de_bin(const int index, const chunk_size_t need);

void *hw_malloc(size_t bytes)
{
	long long need = bytes + 40LL;
	if (!has_init) {
		has_init = true;
		for (int i = 0; i < 7; i++) {
			bin[i] = &s_bin[i];
			bin[i]->prev = bin[i];
			bin[i]->next = bin[i];
			bin[i]->size = 0;
		}
		start_brk = sbrk(64 * 1024);
		heap_brk = start_brk;
		chunk_header *s = create_chunk(64 * 1024);
		heap_brk = start_brk; // reset heap top pointer
		printf("sbrk: %p, size: %lli\n", start_brk, s->chunk_size);
		// if (64 * 1024 - need > 8) {
		chunk_header *c = split(&s, need);
		return (void *)((intptr_t)(void*)c +
		                sizeof(chunk_header) -
		                (intptr_t)(void*)start_brk);
		// } else {
		// return (void *)((intptr_t)(void*)s +
		// sizeof(chunk_header) -
		// (intptr_t)(void*)start_brk);
		// }
	} else {
		chunk_header *r = NULL;
		int bin_num = search_bin(need);
		if (bin_num == -1) {
		} else if (bin_num <= 5) {
			r = de_bin(bin_num, need);
			return (void *)((intptr_t)(void*)r +
			                sizeof(chunk_header) -
			                (intptr_t)(void*)start_brk);
		} else { // bin_num = 6
			chunk_header *s = de_bin(6, need);
			chunk_header *c = split(&s, need);
			return (void *)((intptr_t)(void*)c +
			                sizeof(chunk_header) -
			                (intptr_t)(void*)start_brk);
		}
		//     if (r == NULL) {
		//         printf("not found in bin\n");
		//     } else {
		//         return (void *)((intptr_t)(void*)r +
		//                         sizeof(chunk_header) -
		//                         (intptr_t)(void*)start_brk);
		//     }
	}
	return NULL;
}

int hw_free(void *mem)
{
	return 0;
}

void *get_start_sbrk(void)
{
	return (void *)start_brk;
}

static chunk_header *create_chunk(const chunk_size_t need)
{
	if (heap_brk - start_brk + need > 64 * 1024) {
		printf("heap not enough\n");
		return NULL;
	}
	chunk_header *ret = heap_brk;
	heap_brk += need;
	ret->chunk_size = need;
	ret->prev = NULL;
	ret->next = NULL;
	ret->pre_chunk_size = 0;
	ret->prev_free_flag = 0;
	return ret;
}

static chunk_header *split(chunk_header **ori, const chunk_size_t need)
{
	printf("ori %p size: %lld, need: %lld\n", (*ori), (*ori)->chunk_size, need);
	if ((*ori)->chunk_size - need >= 8) {
		// printf("%p, %p\n", *ori, heap_brk);
		chunk_header *new = (void *)((intptr_t)(void*)*ori + need);
		new->chunk_size = (*ori)->chunk_size - need;
		new->pre_chunk_size = need;
		new->prev_free_flag = 0;
		*ori = new;
		chunk_header *ret = create_chunk(need);
		en_bin(6, (*ori));
		printf("en %p size: %lld\n", (*ori), (*ori)->chunk_size);
		return ret;
	} else {
		return (*ori);
	}
}

static int search_bin(const chunk_size_t need)
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
		printf("not any free chunk\n");
		return -1;
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
				while (cur != (void *)bin[6]) {
					if (c_h->chunk_size < cur->chunk_size) {
						tmp = cur->next;
						cur->next = c_h;
						c_h->prev = cur;
						tmp->prev = c_h;
						c_h->next = tmp;
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
			return ret;
		case 6:
			if (bin[6]->size > 0 &&
			    need > ((chunk_header *)bin[6]->next)->chunk_size) {
				printf("not enough bin: %d, %lld, need: %lld\n", bin[6]->size,
				       ((chunk_header *)bin[6]->next)->chunk_size, need);
				return NULL;
			} else {
				cur = bin[6]->prev;
				while (cur != (void *)bin[6]) {
					if (need < cur->chunk_size) {
						ret = cur;
						//TODO check if cur->prev or next is bin
						((chunk_header *)cur->prev)->next = cur->next;
						((chunk_header *)cur->next)->prev = cur->prev;
						ret->prev = NULL;
						ret->next = NULL;
						return ret;
					}
					cur = cur->prev;
				}
			}
		}
		return NULL; //TODO should not reach here
	}
}
