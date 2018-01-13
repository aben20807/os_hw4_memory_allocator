#include "hw_malloc.h"
#include <stdio.h>
#include <stdlib.h>

/*Global Variable*/
bool has_init = false;
void *start_brk = NULL;
void *heap_brk = NULL;
bin_t bin[7] = {};
/*Static function*/
static chunk_header *create_chunk(const chunk_size_t size);
static chunk_header *split(chunk_header *ori, const chunk_size_t need);
static void en_bin(const int bin_num, chunk_header *c_h);
static chunk_header *de_bin(const int index);

void *hw_malloc(size_t bytes)
{
	long long chunk_size = bytes + 40LL;
	if (!has_init) {
		has_init = true;
		for (int i = 0; i < 7; i++) {
			bin[i].prev = &bin[i];
			bin[i].next = &bin[i];
			bin[i].size = 0;
		}
		start_brk = sbrk(64 * 1024);
		heap_brk = start_brk;
		chunk_header *s = create_chunk(64 * 1024);
		printf("sbrk: %p, size: %lli\n", start_brk, s->chunk_size);
		if (64 * 1024 - chunk_size > 8) {
			chunk_header *c = split(s, chunk_size);
			printf("bin size: %d\n", bin[0].size);
			en_bin(0, c);
			en_bin(6, s);
			printf("bin0 size: %d\n", bin[0].size);
			printf("bin6 size: %d\n", bin[6].size);
			// printf("%lld\n", c->chunk_size);
			// printf("%p, size: %lli\n", start_brk, s->chunk_size);
		} else {
		}
	} else {
	}
	// void *ptr = sbrk(0);
	// printf("%p\n", ptr);
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

static chunk_header *create_chunk(const chunk_size_t size)
{
	chunk_header *ret = heap_brk;
	heap_brk += sizeof(chunk_header);
	ret->chunk_size = size;
	ret->prev = NULL;
	ret->next = NULL;
	ret->pre_chunk_size = 0;
	ret->prev_free_flag = 0;
	return ret;
}

static chunk_header *split(chunk_header *ori, const chunk_size_t need)
{
	ori->chunk_size -= need;
	ori->pre_chunk_size = need;
	ori->prev_free_flag = 0;
	chunk_header *ret = create_chunk(need);
	return ret;
}

static void en_bin(const int index, chunk_header *c_h)
{
	if (bin[index].size == 0) {
		bin[index].next = c_h;
		c_h->prev = &bin[index];
		bin[index].prev = c_h;
		c_h->next = &bin[index];
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
			tmp = bin[index].prev;
			bin[index].prev = c_h;
			c_h->next = &bin[index];
			tmp->next = c_h;
			c_h->prev = tmp;
			break;
		case 6:
			if (bin[6].size > 0 &&
			    c_h->chunk_size > ((chunk_header *)bin[6].next)->chunk_size) {
				tmp = bin[index].next;
				bin[index].next = c_h;
				c_h->prev = &bin[index];
				tmp->prev = c_h;
				c_h->next = tmp;
			} else {
				cur = bin[6].prev;
				while (cur != (void *)&bin[6]) {
					if (c_h->chunk_size < cur->chunk_size) {
						tmp = cur->next;
						cur->next = c_h;
						c_h->prev = cur;
						tmp->prev = c_h;
						c_h->next = tmp;
					}
				}
			}
			break;
		}
	}
	bin[index].size++;
}

static chunk_header *de_bin(const int index)
{
	return NULL;
}
