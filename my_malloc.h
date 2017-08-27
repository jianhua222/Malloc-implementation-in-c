#ifndef __MY_MALLOC_H__
#define __MY_MALLOC_H__

/* we need this *ONLY* for the size_t typedef */
#include <stdio.h>

/* Our metadata structure for use in the freelist.
 * you *MUST NOT* change this definition unless specified
 * in an official assignment update by the TAs.
 */
typedef struct metadata
{
    struct metadata* next;
    unsigned long size; // size in number of words
    unsigned long canary;
} metadata_t;

/* This is your error enum. The three
 * different types of errors for this homework are explained below.
 * If ANY function has a case where one of the errors described could
 * occur, it must set ERRNO to the appropriate enum.
 * In the case where a single request is too large and
 * the request results in an out of memory error as well, the
 * SINGLE_REQUEST_TOO_LARGE should take precedence. If any of the three
 * functions complete successfully, the error code should be set to
 * NO_ERROR. */
enum my_malloc_err {
    NO_ERROR,
    OUT_OF_MEMORY,
    SINGLE_REQUEST_TOO_LARGE,
    CANARY_CORRUPTED
};
enum my_malloc_err ERRNO;

/* MALLOC
 *
 * this function should allocate a block that is big enough to hold the
 * specified size (bytes), and that is all. if there is not a block that is able
 * to satisfy the request, then you should attempt to grab more heap
 * space with a call to my_sbrk. if this succeeds, then you should continue
 * as normal. If it fails (by returning NULL), then you should return NULL.
 */
void* my_malloc(size_t);

/* REALLOC
 * 
 * This function should reallocate the provided block to the new size requested (in bytes).
 * If the pointer passed in is NULL, this call is the same as my_malloc(new_size).
 * If the size passed in is 0 and the pointer is non-NULL, this call is the same as
 * my_free(ptr). The contents of the given block should be copied over to the new block
 * up to the size of the new block. If the new block is larger, the additional space
 * should not be initialized. A useful function for copying over the data is memcpy.
 * Run "man memcpy" in a terminal to read more about it.
 */
void* my_realloc(void*, size_t);

/* CALLOC
 *
 * This function should allocate a block of size nmemb * size (in bytes). It should also zero
 * out the entire block before returning to the user. A useful function for this is
 * memset. Run "man memset" in a terminal to read more about it.
 */
void* my_calloc(size_t nmemb, size_t size);

/* FREE
 *
 * this function should free the block of memory, merging it with the block
 * in memory to its right and left if possible.
 */
void my_free(void*);

#endif /* __MY_MALLOC_H__ */
