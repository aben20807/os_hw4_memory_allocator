# os_hw4_memory_allocator
+ Implement a memory allocator library
+ 3 functions
    + `void *hw_malloc(size_t bytes);`
    + `int hw_free(void *mem);`
    + `void *hw_get_start_brk();`

## Chunk
+ header: 40 bytes
    + `prev`: point to next free chunk in same list
    + `next`: point to prev free chunk in same list
    + `chunk_size`: the size of this chunk
    + `prev_chunk_size`: the size of prev adjacent chunk
    + `prev_free_flag`: prev adjacent chunk is free (`1`) or not (`0`)
```
        +-------------------------------+
        .          next chunk           .
        .                               .
        +-------------------------------+
        |          chunk data           |
   mem->+-------------------------------+
 header:| chunk_ptr_t prev;             |
        | chunk_ptr_t next;             |
        | chunk_size_t chunk_size;      |
        | chunk_size_t prev_chunk_size; |
        | chunk_flag_t prev_free_flag;  |
        +-------------------------------+
        .          prev chunk           .
        .                               .
        +-------------------------------+
```

## Bins
+ doubly-linked list
+ 7 bins
    + `bin[0]`: 48 bytes (include header)
    + `bin[1]`: 56 bytes
    + `bin[2]`: 64 bytes
    + `bin[3]`: 72 bytes
    + `bin[4]`: 80 bytes
    + `bin[5]`: 88 bytes
    + `bin[6]`: > 88 bytes

```
    +--------+
    | bin[0] |
    +--------+
       ↕    ↑
    +----+  |
    | 48 |  |
    +----+  |
       ↕    |
    +----+  |
    | 48 | ←┘
    +----+
```