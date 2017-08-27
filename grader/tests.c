#include "tests.h"

size_t total_freelist_block_size() {
    size_t total = 0;
    for (metadata_t *t = freelist; t; t = t->next)
        total += t->size;
    return total;
}

size_t total_freelist_count() {
    size_t total = 0;
    for (metadata_t *t = freelist; t; t = t->next)
        total++;
    return total;
}

int is_freelist_in_order() {
    if (freelist == NULL)
        return 1;

    for (metadata_t* t = freelist; t->next; t = t->next) {
        if ((uintptr_t)t->next < (uintptr_t)t) {
            return 0;
        }
    }

    return 1;
}

unsigned long compute_canary(metadata_t* block) {
    return (uintptr_t)block ^ block->size;
}

#define SBRK_SIZE 2048
#define WORD_SIZE 8
#define TOTAL_METADATA_SIZE ((sizeof(metadata_t) + sizeof(long)) / WORD_SIZE)
#define MIN_BLOCK_SIZE (TOTAL_METADATA_SIZE + 1)

#define to_metadata(i) ((metadata_t*)(i) - 1)
#define round_to_word(i) ((((i)%WORD_SIZE == 0) ? (i) : (i) + (WORD_SIZE - (i)%WORD_SIZE)) / WORD_SIZE)
#define round_to_block(i) (round_to_word(i) + TOTAL_METADATA_SIZE)

/* Thoroughly checks their my_malloc function */
void check_malloc_1() {
    char* tag = "check_malloc_1 - basic tests";

    int size1 = 10, size2 = 100, size3 = 500, size4 = 1200;

    metadata_t *arr1 = my_malloc(size1);

    if (NULL == arr1 || to_metadata(arr1)->size != round_to_block(size1)) {
        deduct_points(M_SIZE_POINTS, tag, "the block of data was not malloced to the proper size;");
    }  else {
        add_points(M_SIZE_POINTS, tag, "the block of data was malloced to the proper size;");
    }

    /* Check the total free list size */
    {
        if (total_freelist_block_size() != SBRK_SIZE/WORD_SIZE - round_to_block(size1) || !is_freelist_in_order()) {
            deduct_points(M_FREELIST_POINTS, tag, "the freelist is not what it should be, which will throw off everything else in your library;");
        } else {
            add_points(M_FREELIST_POINTS, tag, "the freelist is what it should be");
        }
    }

    /* test 2 malloc (serious test) */
    metadata_t *arr2 = my_malloc(size2);
    my_malloc(size3);
    metadata_t *arr4 = my_malloc(size4);
    my_free(arr2);
    my_free(arr4);
    printList();
    {
        if (total_freelist_block_size() != SBRK_SIZE/WORD_SIZE - (round_to_block(size1)+round_to_block(size3)) || !is_freelist_in_order()) {
            deduct_points(M_FREELIST2_POINTS, tag, "the freelist is not what it should be after a couple of calls to malloc;");
        } else {
            add_points(M_FREELIST2_POINTS, tag, "the freelist is what it should be after a couple of calls to malloc;");
        }
    }

    /* malloc enough to make it allocate another block */
    arr2 = my_malloc(size3+size4);
    printList();
    {
        if (total_freelist_block_size() != 2*SBRK_SIZE/WORD_SIZE - (round_to_block(size1)+round_to_block(size3)+round_to_block(size3+size4)) || !is_freelist_in_order()) {
            deduct_points(M_FREELIST3_POINTS, tag, "the freelist is not what it should be after a couple more calls to malloc;");
        } else {
            add_points(M_FREELIST3_POINTS, tag, "the freelist is what it should be after a couple more calls to malloc;");
        }
    }
}

void check_malloc_2a() {
    char* tag = "check_malloc_2a - exact fit";

    int size1 = 4 * WORD_SIZE, size2 = 300, size3 = 3 * WORD_SIZE, size4 = 300;
    void* t1 = my_malloc(size1);
    my_malloc(size2);
    void* t3 = my_malloc(size3);
    my_malloc(size4);

    my_free(t1);
    my_free(t3);

    my_malloc(size3);

    if (total_freelist_block_size() != SBRK_SIZE/WORD_SIZE - round_to_block(size2) - round_to_block(size3) - round_to_block(size4) || total_freelist_count() != 2 || !is_freelist_in_order()) {
        deduct_points(M_EXACT_FIT_POINTS, tag, "Did not select exact fit from the freelist;");
    } else {
        add_points(M_EXACT_FIT_POINTS, tag, "Selected exact fit from freelist;");
    }
}

void check_malloc_2b() {
    char* tag = "check_malloc_2b - smallest split";

    int size1 = 50*WORD_SIZE;
    void* v1 = my_malloc(size1);

    my_malloc(50*WORD_SIZE);

    int size2 = 30*WORD_SIZE;
    void* v2 = my_malloc(size2);

    my_malloc(50*WORD_SIZE);

    my_free(v1);
    my_free(v2);

    // freelist -> {size=54} -> {size=34} -> {size=60} on 64-bit
    // freelist -> (size=52} -> {size=32} -> {size=68} on 32-bit

    my_malloc(WORD_SIZE);

    // The second block in the freelist (regardless of splitting from the back or front) should be the chosen block.
    if (!freelist || !freelist->next || freelist->next->size != round_to_block(size2) - round_to_block(WORD_SIZE) || total_freelist_count() != 3 || !is_freelist_in_order()) {
        deduct_points(M_SPLIT_POINTS, tag, "Did not select smallest splittable from the freelist or freelist not correct;");
    } else {
        add_points(M_SPLIT_POINTS, tag, "Selected smallest splittable block and freelist is correct;");
    }
}

void check_malloc_2c() {
    char* tag = "check_malloc_2c - smallest fit";

    int size1 = 3*WORD_SIZE;
    void* v1 = my_malloc(size1);

    int size2 = 10*WORD_SIZE;
    my_malloc(size2);

    int size3 = 2*WORD_SIZE;
    void* v3 = my_malloc(size3);

    int size4 = SBRK_SIZE - TOTAL_METADATA_SIZE*WORD_SIZE*5 - 18*WORD_SIZE;
    my_malloc(size4);

    my_free(v1);
    my_free(v3);

    printList();

    // freelist -> {size=7} -> {size=6} -> {size=7} on 64-bit
    // freelist -> {size=5} -> {size=4} -> {size=5} on 32-bit

    my_malloc(WORD_SIZE);

    if (total_freelist_block_size() != TOTAL_METADATA_SIZE*2+6 || total_freelist_count() != 2 || !is_freelist_in_order()) {
        deduct_points(M_NEXT_FIT_POINTS, tag, "Did not select smallest fit from the freelist;");
    } else {
        add_points(M_NEXT_FIT_POINTS, tag, "Selected exact next smallest freelist;");
    }
}

void check_malloc_3() {
    /* Malloc's edge cases. */
    char* tag = "check_malloc_3 - edge cases";

    ERRNO = 99;

    metadata_t* it = my_malloc(0);
    if (it != NULL || ERRNO != NO_ERROR) {
        deduct_points(MALLOC_ZERO, tag, "Allocated block for size 0 or did not set NO_ERROR;");
    } else {
        add_points(MALLOC_ZERO, tag, "Returned NULL for size 0;");
    }

    it = my_malloc(SBRK_SIZE - TOTAL_METADATA_SIZE * WORD_SIZE);
    if (it == NULL) {
        deduct_points(MALLOC_EDGE_MAX, tag, "Did not allocate max size;");
    } else {
        add_points(MALLOC_EDGE_MAX, tag, "Did allocate max size;");
    }
    my_free(it);

    ERRNO = 99;

    it = my_malloc(SBRK_SIZE - TOTAL_METADATA_SIZE * WORD_SIZE + 1);
    if (it != NULL || ERRNO!=SINGLE_REQUEST_TOO_LARGE) {
        deduct_points(MALLOC_EDGE_MAX1, tag, "Allocated max size + 1 or did not set SINGLE_REQUEST_TOO_LARGE;");
    } else {
        add_points(MALLOC_EDGE_MAX1, tag, "Did not allocate max size + 1;");
    }

    ERRNO = 99;

    it = my_malloc(1000000);
    if (it != NULL || ERRNO!=SINGLE_REQUEST_TOO_LARGE) {
        deduct_points(MALLOC_EDGE_HUGE, tag, "Allocated 1000000 bytes or did not set SINGLE_REQUEST_TOO_LARGE;");
    } else {
        add_points(MALLOC_EDGE_HUGE, tag, "Did not allocate 1000000 bytes;");
    }

    ERRNO = 99;

    int i;
    void *mem[5];
    for (i = 0; i < 5; i++) {
        mem[i] = my_malloc(SBRK_SIZE - TOTAL_METADATA_SIZE * WORD_SIZE);
    }

    if (mem[4] != NULL || ERRNO!=OUT_OF_MEMORY) {
        deduct_points(MALLOC_OOM, tag, "Did not return NULL for OOM or did not set OUT_OF_MEMORY;");
    } else {
        add_points(MALLOC_OOM, tag, "Did return NULL for OOM;");
    }
}

void check_malloc_4() {
    char* tag = "check_malloc_4 - stress test";
    char* last_data = NULL;
    for (size_t i = 0; i < 1000; i++) {
        size_t size = (i*i)%1000 + 1;
        char* data = my_malloc(size);
        for (size_t j = 0; j < size; j++)
            data[j] = i*j;
        if (last_data) {
            size_t old_size = ((i-1)*(i-1))%1000 + 1;
            for (size_t j = 0; j < old_size; j++) {
                if (last_data[j] != (char)((i-1)*j)) {
                    deduct_points(MALLOC_STRESS, tag, "Did not pass stress test;");
                    return;
                }
            }
            my_free(last_data);

            if (total_freelist_block_size()%(SBRK_SIZE/WORD_SIZE) != SBRK_SIZE/WORD_SIZE - round_to_block(size) || !is_freelist_in_order()) {
                deduct_points(MALLOC_STRESS, tag, "Did not pass stress test check;");
                return;
            }
        }
        last_data = data;
    }
    my_free(last_data);

    if (total_freelist_block_size()%(SBRK_SIZE/WORD_SIZE) != 0 || total_freelist_count() != 1) {
        deduct_points(MALLOC_STRESS, tag, "Did not pass stress test final check;");
    } else {
        add_points(MALLOC_STRESS, tag, "Passed stress test;");
    }
}

void check_realloc_1() {
    char* tag = "check_realloc_1 - realloc smaller block";

    void* c1 = my_malloc(100);
    memset(c1, 42, 100);

    int newsize = 50;
    void* c2 = my_realloc(c1, newsize);

    int freelist_size1 = total_freelist_block_size();

    int mem_copied = 1;
    for (int i = 0; i < 50; i++) {
        if (((char*)c2)[i] != 42) {
            mem_copied = 0;
            break;
        }
    }

    my_free(c2);

    int freelist_size2 = total_freelist_block_size();

    // check that doing the realloc did not corrupt the CANARY
    if (!mem_copied || ERRNO == CANARY_CORRUPTED) {
        deduct_points(REALLOC_SMALLER, tag, "reallocated block not properly copied over;");
    } else if (freelist_size1 != SBRK_SIZE/WORD_SIZE - round_to_block(newsize) || freelist_size2 != SBRK_SIZE/WORD_SIZE) {
        deduct_points(REALLOC_SMALLER, tag, "freelist not correct after realloc and free;");
    } else {
        add_points(REALLOC_SMALLER, tag, "freelist is correct and realloc block properly copied over;");
    }
}

void check_realloc_2() {
    char* tag = "check_realloc_2 - realloc larger block";

    void* c1 = my_malloc(100);
    memset(c1, 42, 100);

    int newsize = 500;
    void* c2 = my_realloc(c1, newsize);

    int freelist_size1 = total_freelist_block_size();

    int mem_copied = 1;
    for (int i = 0; i < 100; i++) {
        if (((char*)c2)[i] != 42) {
            mem_copied = 0;
            break;
        }
    }

    my_free(c2);

    int freelist_size2 = total_freelist_block_size();

    if (!mem_copied) {
        deduct_points(REALLOC_LARGER, tag, "reallocated block not properly copied over;");
    } else if (freelist_size1 != SBRK_SIZE/WORD_SIZE - round_to_block(newsize) || freelist_size2 != SBRK_SIZE/WORD_SIZE) {
        deduct_points(REALLOC_LARGER, tag, "freelist not correct after realloc and free;");
    } else {
        add_points(REALLOC_LARGER, tag, "freelist is correct and realloc block properly copied over;");
    }
}

void check_realloc_3() {
    char* tag = "check_realloc_3 - realloc with NULL and 0";

    int size = 100;
    void* c1 = my_realloc(NULL, size);
    metadata_t* t = to_metadata(c1);

    if (t->size != round_to_block(size) || total_freelist_block_size() != SBRK_SIZE/WORD_SIZE - t->size) {
        deduct_points(REALLOC_NULL, tag, "did not malloc when realloc passed NULL ptr;");
    } else {
        add_points(REALLOC_NULL, tag, "properly malloc-ed when realloc passed NULL ptr;");

        void* c2 = my_realloc(c1, 0);

        if (c2 != NULL || total_freelist_block_size() != SBRK_SIZE/WORD_SIZE) {
            deduct_points(REALLOC_FREE, tag, "did not free when realloc passed size 0;");
        } else {
            add_points(REALLOC_FREE, tag, "properly freed when realloc passed size 0;");
        }
    }
}

void check_calloc() {
    char* tag = "check_calloc";

    int size = 200;
    void* c1 = my_malloc(size);
    memset(c1, 42, 200);
    my_free(c1);

    if (c1 != my_calloc(50, 4)) {
        deduct_points(CALLOC, tag, "returned a different block of memory");
        return;
    }

    metadata_t* t = to_metadata(c1);

    int mem_zero = 1;
    for (int i = 0; i < 200; i++) {
        if (((char*)c1)[i] != 0) {
            mem_zero = 0;
            break;
        }
    }

    if (!mem_zero) {
        deduct_points(CALLOC, tag, "did not zero out memory;");
    } else if (t->size != round_to_block(size) || total_freelist_block_size() != SBRK_SIZE/WORD_SIZE - t->size) {
        deduct_points(CALLOC, tag, "did not allocate correct number of bytes or freelist incorrect after allocation;");
    } else {
        add_points(CALLOC, tag, "properly allocated correct number of bytes and zeroed memory;");
    }
}

/*[> Thoroughly checks their my_free function <]*/
void check_free_1() {
    char* tag = "check_free_1 - basic";
    metadata_t *arr1, *arr2, *arr3, *arr4;
    arr1 = my_malloc(1);
    arr2 = my_malloc(1);
    arr3 = my_malloc(1);
    arr4 = my_malloc(1);

    my_free(arr2);

    /*[> Now, make sure that they didn't combine the nodes <]*/
    if (to_metadata(arr2)->size != round_to_block(1)) {
        deduct_points(F_NO_COMBINE, tag, "Combined the size improperly;");
    } else {
        if (total_freelist_block_size() != SBRK_SIZE/WORD_SIZE - 3*round_to_block(1) || !is_freelist_in_order()) {
            deduct_points(F_NO_COMBINE, tag, "freelist is not correct after one free;");
        } else {
            add_points(F_NO_COMBINE, tag, "freelist is correct after one free;");
        }
    }

    /*[> Combine two nodes into one <]*/
    my_free(arr1);
    if (to_metadata(arr1)->size == round_to_block(1) && to_metadata(arr2)->size == round_to_block(1)) {
        deduct_points(F_COMBINE, tag, "Did not combine two nodes into one;");
    } else {
        if (total_freelist_block_size() != SBRK_SIZE/WORD_SIZE - 2*round_to_block(1) || !is_freelist_in_order()) {
            deduct_points(F_COMBINE, tag, "the freelist is not what it should be after combining one node;");
        } else {
            add_points(F_COMBINE, tag, "the freelist is what it should be after combining one node;");
        }
    }

    /*[> Combine all of the way to 2048 <]*/
    my_free(arr3);
    my_free(arr4);
    {
        if (total_freelist_block_size() != SBRK_SIZE/WORD_SIZE || total_freelist_count() != 1 || !is_freelist_in_order()) {
            deduct_points(F_COMBINE_RECURSE, tag, "the freelist is not correct after combining all to 2048;");
        } else {
            add_points(F_COMBINE_RECURSE, tag, "the freelist is correct after combining all to 2048;");
        }
    }
}

void check_free_2() {
    char* tag = "check_free_2 - more combining";
    /*[> The next check is to make sure we do combine with a different size. <]*/
    metadata_t *arr1 = my_malloc(64);
    metadata_t *arr3 = my_malloc(32);
    my_malloc(32);
    my_free(arr3);
    my_free(arr1);
    if (total_freelist_block_size() != SBRK_SIZE/WORD_SIZE - round_to_block(32) || total_freelist_count() != 2 || !is_freelist_in_order()) {
        deduct_points(F_DIFF_SIZE, tag, "the freelist is not correct after freeing a buddy of different size;");
    } else {
        add_points(F_DIFF_SIZE, tag, "the freelist is correct after freeing a buddy of different size;");
    }
}

void check_free_3() {

    char* tag = "check_free_3 - test freelist";
    /* The next set of test cases is essentially test code for the linked list
     * free list implementations. So far, in all of the above, the frees only
     * removed the only element from the list. The next three tests will have
     * three free list nodes, one of which is the buddy being freed.
     */
    metadata_t *arr1 = my_malloc(1);
    metadata_t *arr2 = my_malloc(1);
    my_malloc(1); //[> Prevent recombination <]
    metadata_t *arr3 = my_malloc(1);
    my_malloc(1); //[> Prevent recombination <]
    metadata_t *arr4 = my_malloc(1);
    my_free(arr2);
    my_free(arr3);
    my_free(arr4);
    my_free(arr1);
    if (total_freelist_block_size() != SBRK_SIZE/WORD_SIZE - 2*round_to_block(1) || total_freelist_count() != 3 || !is_freelist_in_order()) {
        deduct_points(FREE_MULTI_HEAD, tag, "freelist not correct if buddy is first in list;");
    } else {
        add_points(FREE_MULTI_HEAD, tag, "freelist correct if buddy is first in list;");
    }
}
void check_free_4() {
    char* tag = "check_free_4 - more recombination";
    metadata_t *arr1 = my_malloc(1);
    metadata_t *arr2 = my_malloc(1);
    my_malloc(1); //[> Prevent recombination <]
    metadata_t *arr3 = my_malloc(1);
    my_malloc(1); //[> Prevent recombination <]
    metadata_t *arr4 = my_malloc(1);
    my_free(arr3);
    my_free(arr2);
    my_free(arr4);

    my_free(arr1);
    {
        if (total_freelist_block_size() != SBRK_SIZE/WORD_SIZE - 2*round_to_block(1) || total_freelist_count() != 3 || !is_freelist_in_order()) {
            deduct_points(FREE_MULTI_MID, tag, "freelist not correct if buddy is second in list;");
        } else {
            add_points(FREE_MULTI_MID, tag, "freelist correct if buddy is second in list;");
        }
    }
}

void check_free_5() {
    char* tag = "check_free_5 - more recombination";
    metadata_t *arr1 = my_malloc(1);
    metadata_t *arr2 = my_malloc(1);
    my_malloc(1); //[> Prevent recombination <]
    metadata_t *arr3 = my_malloc(1);
    my_malloc(1); //[> Prevent recombination <]
    metadata_t *arr4 = my_malloc(1);
    my_free(arr3);
    my_free(arr4);
    my_free(arr2);
    my_free(arr1);
    {
        if (total_freelist_block_size() != SBRK_SIZE/WORD_SIZE - 2*round_to_block(1) || total_freelist_count() != 3 || !is_freelist_in_order()) {
            deduct_points(FREE_MULTI_TAIL, tag, "freelist not correct if buddy is last in list;");
        } else {
            add_points(FREE_MULTI_TAIL, tag, "freelist correct if buddy is last in list;");
        }
    }

    /*my_free(NULL);*/
}
void *c1,*c2,*c3,*c4,*c5,*c6,*c7,*c8;
enum my_malloc_err e1, e2, e3,e4,e5,e6,e7,e8;
void check_EC_1() {
    char* tag = "check_EC_1 - test REQUEST_TOO_LARGE";

    printf("Testing 16\n");
    ERRNO = 99;
    c1 = my_malloc(SBRK_SIZE);
    e1 = ERRNO;

    printf("Testing 17\n");
    ERRNO = 99;
    c2 = my_malloc(SBRK_SIZE-TOTAL_METADATA_SIZE*WORD_SIZE+1);
    e2 = ERRNO;

    printf("Testing 18\n");
    ERRNO = 99;
    c3 = my_malloc(SBRK_SIZE-TOTAL_METADATA_SIZE*WORD_SIZE);
    e3 = ERRNO;

    printf("Testing 19\n");
    ERRNO = 99;
    c4 = my_malloc(512);
    e4 = ERRNO;

    printf("Finished EC part 1\n");
    if (((e1!=SINGLE_REQUEST_TOO_LARGE)||
                (e2!=SINGLE_REQUEST_TOO_LARGE)||
                (e3!=NO_ERROR)||
                (e4!=NO_ERROR)))
        deduct_points(ERR_SINGLE_REQUEST_TOO_LARGE_POINTS, tag, "Error code SINGLE_REQUEST_TOO_LARGE not used correctly;");
    else
        add_points(ERR_SINGLE_REQUEST_TOO_LARGE_POINTS, tag, "Error code SINGLE_REQUEST_TOO_LARGE used correctly;");
}
void check_EC_2() {
    char* tag = "check_EC_2 - test OUT_OF_MEMORY";

    printf("Testing 20\n");
    c1 = my_malloc(SBRK_SIZE-TOTAL_METADATA_SIZE*WORD_SIZE);
    c2 = my_malloc(SBRK_SIZE-TOTAL_METADATA_SIZE*WORD_SIZE);
    c3 = my_malloc(SBRK_SIZE-TOTAL_METADATA_SIZE*WORD_SIZE);
    c4 = my_malloc(SBRK_SIZE-TOTAL_METADATA_SIZE*WORD_SIZE);

    ERRNO = 99;

    c5 = my_malloc(1);
    e1 = ERRNO;

    if (e1!=OUT_OF_MEMORY)
        deduct_points(ERR_OUT_OF_MEMORY_POINTS, tag, "Error code OUT_OF_MEMORY not used correctly;");
    else
        add_points(ERR_OUT_OF_MEMORY_POINTS, tag, "Error code OUT_OF_MEMORY used correctly;");

    printf("Finished EC part 2\n");
}
void check_EC_3() {
    char* tag = "check_EC_3 - test NO_ERROR";

    ERRNO = 99;
    c1 = my_malloc(48);
    e1 = ERRNO;

    ERRNO = 99;
    my_free(c1);
    e2 = ERRNO;

    /*[>Quick No_ERROR test<]*/
    if (e1!=NO_ERROR || e2 != NO_ERROR)
        deduct_points(ERR_NO_ERROR_POINTS, tag, "Error code NO_ERROR not used correctly in malloc/free;");
    else
        add_points(ERR_NO_ERROR_POINTS, tag, "Error code NO_ERROR used correctly;");
}
void check_EC_4_canary1() {
    char* tag = "check_EC_4_canary1 - test CANARY_CORRUPTED for beginning of buffer;";
    c1 = my_malloc(48);
    metadata_t* t = to_metadata(c1);
    t->canary += 0xDEADBEEF;

    ERRNO = 99;
    my_free(c1);
    e1 = ERRNO;

    ERRNO = 99;
    c1 = my_malloc(48);
    t = to_metadata(c1);
    t->size = 0xCAFEBABE;
    my_free(c1);
    e2 = ERRNO;

    /*[>Quick No_ERROR test<]*/
    if (e1 != CANARY_CORRUPTED || e2 != CANARY_CORRUPTED) {
        deduct_points(ERR_CANARY_CORRUPTED1, tag, "Error code CANARY_CORRUPTED not used correctly for corruption before beginning of buffer;");
    } else {
        add_points(ERR_CANARY_CORRUPTED1, tag, "Error code CANARY_CORRUPTED used correctly for corruption before beginning of buffer;");
    }
}

void check_EC_4_canary2() {
    char* tag = "check_EC_4_canary2 - test CANARY_CORRUPTED for end of buffer";

    int size = 500;
    c1 = my_malloc(size);
    ((char*)to_metadata(c1))[round_to_block(size)*WORD_SIZE - sizeof(long)] += 0x11;
    ERRNO = 99;
    my_free(c1);
    e1 = ERRNO;

    /*[>Quick No_ERROR test<]*/
    if (e1!=CANARY_CORRUPTED) {
        deduct_points(ERR_CANARY_CORRUPTED2, tag, "Error code CANARY_CORRUPTED not used correctly for corruption at end of buffer;");
    } else {
        add_points(ERR_CANARY_CORRUPTED2, tag, "Error code CANARY_CORRUPTED used correctly for corruption at end of buffer;");
    }
}

void check_EC_4_canary3() {
    char* tag = "check_EC_4_canary3 - test canary computed properly";

    int size = 500;
    c1 = my_malloc(size);
    metadata_t* t = to_metadata(c1);

    unsigned long canary = compute_canary(t);

    unsigned long end_canary = *(unsigned long*)((char*)t + round_to_block(size)*WORD_SIZE - sizeof(long));

    if (t->canary != canary || end_canary != canary) {
        deduct_points(ERR_CANARY_CORRUPTED3, tag, "canary not computed correctly from hash of metadata address, block_size, and request_size;");
    } else {
        add_points(ERR_CANARY_CORRUPTED3, tag, "canary hashed properly;");
    }
}

/*[>prints out what the freeList should look like<]*/
void printList() {
    printf("\n--------FREELIST--------\n");

    extern metadata_t* freelist;
    for (metadata_t* curr = freelist; curr; curr = curr->next) {
        printf("%p - { next = %p, size = %lu }\n", (void*)curr, (void*)curr->next, curr->size);
    }
}
