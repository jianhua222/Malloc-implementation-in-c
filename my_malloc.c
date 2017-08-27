/*
 * Author: Jian Hua
 */

/* we need this for uintptr_t */
#include <stdint.h>
/* we need this for memcpy/memset */
#include <string.h>
/* we need this for my_sbrk */
#include "my_sbrk.h"
/* we need this for the metadata_t struct and my_malloc_err enum definitions */
#include "my_malloc.h"

/* You *MUST* use this macro when calling my_sbrk to allocate the
 * appropriate size. Failure to do so may result in an incorrect
 * grading!
 */
#define SBRK_SIZE 2048

/* All sizes will be in multiples of the word size defined below */
#define WORD_SIZE 8

/* This is the size of the metadata struct and canary footer in words */
#define TOTAL_METADATA_SIZE ((sizeof(metadata_t) + sizeof(long)) / WORD_SIZE)

/* This is the minimum size of a block in words, where it can
 * store the metadata, canary footer, and at least 1 word of data
 */
#define MIN_BLOCK_SIZE (TOTAL_METADATA_SIZE + 1)

/* If you want to use debugging printouts, it is HIGHLY recommended
 * to use this macro or something similar. If you produce output from
 * your code then you may receive a 20 point deduction. You have been
 * warned.
 */
#ifdef DEBUG
#define DEBUG_PRINT(x) printf(x)
#else
#define DEBUG_PRINT(x)
#endif

/* Our freelist structure - this is where the current freelist of
 * blocks will be maintained. failure to maintain the list inside
 * of this structure will result in no credit, as the grader will
 * expect it to be maintained here.
 * DO NOT CHANGE the way this structure is declared
 * or it will break the autograder.
 */
metadata_t* freelist;

/* MALLOC
 * See my_malloc.h for documentation
 */
void* my_malloc(size_t size) {
	int neededSize = 0;
	if( size%WORD_SIZE ==0){
		neededSize = size /WORD_SIZE + TOTAL_METADATA_SIZE;
	}else{
	 neededSize = (size + (WORD_SIZE - size%WORD_SIZE))/WORD_SIZE + TOTAL_METADATA_SIZE;
	
	}
	
	if(size ==0  ){
			ERRNO =NO_ERROR;
		return NULL;
	}
	
	if(neededSize>SBRK_SIZE/WORD_SIZE ){
		ERRNO = SINGLE_REQUEST_TOO_LARGE;
		return NULL;
	}
	
	metadata_t *cur = freelist;
	metadata_t *pre = NULL;
	metadata_t *fit = NULL;
	metadata_t *prefit = NULL;
	metadata_t *split = NULL;
	metadata_t* toReturn;
	while(cur!= NULL){
		if(cur->size  >= neededSize){
			if(fit==NULL||fit->size > cur->size){
			fit = cur;
			prefit = pre;
			}
		
		}
		if(cur->size >= neededSize+MIN_BLOCK_SIZE){
			if(split==NULL||split->size > cur->size){
			split = cur;
			}
		
		}
		pre = cur;
		cur = cur ->next;
	}
	if(fit==NULL ){
		if(freelist ==NULL){
			freelist = my_sbrk(SBRK_SIZE);
			if(freelist == NULL){
			ERRNO = OUT_OF_MEMORY;
			return NULL;
			}
			freelist->size = SBRK_SIZE / WORD_SIZE  ;
			fit = freelist;
			fit->next = NULL;
			fit->canary = (uintptr_t)fit^ fit->size;
			*(unsigned long*)((char*)fit + fit->size*WORD_SIZE-sizeof(long)) =  fit->canary;
			if(freelist->size>= neededSize+MIN_BLOCK_SIZE){
			split = freelist;
			}
		}
		else{
			pre->next = my_sbrk(SBRK_SIZE);
			if(pre->next == NULL){
			ERRNO = OUT_OF_MEMORY;
			return NULL;
			}
			pre->next->size = SBRK_SIZE / WORD_SIZE  ;
			fit = pre->next;
			fit->next =NULL;
			fit->canary = (uintptr_t)fit^ fit->size;
			*(unsigned long*)((char*)fit + fit->size*WORD_SIZE-sizeof(long)) =   fit->canary;
			if( pre->next->size  >= neededSize+MIN_BLOCK_SIZE){
			split =  pre->next;
			}
		}
	}
	if( fit->size == neededSize){
		toReturn = fit;
		if(fit==freelist){
			freelist = freelist->next;			
		}
		else{
			prefit ->next = prefit -> next->next;
		}
	}else if(split !=NULL){
		toReturn = (metadata_t*)((char*)(split) + (split->size*WORD_SIZE)  -  neededSize*WORD_SIZE);
		toReturn->next = NULL;
		toReturn->size = neededSize;
		split->size = split->size - toReturn->size ;
		split-> canary = (uintptr_t)split^ split->size;
		*(unsigned long*)((char*)split + split->size*WORD_SIZE - sizeof(long)) = split-> canary;
		
	}else{
		toReturn = fit;
		if(fit==freelist){
			freelist = freelist->next;			
		}
		else{
			prefit ->next = prefit -> next->next;
		}
	}
	ERRNO =NO_ERROR;
	toReturn->canary = (uintptr_t)toReturn ^ toReturn->size;
	*(unsigned long*)((char*)toReturn +toReturn->size*WORD_SIZE -sizeof(long)) =  toReturn->canary;
	return (void*)((char*)toReturn+sizeof(metadata_t));
}

/* REALLOC
 * See my_malloc.h for documentation
 */
void* my_realloc(void* ptr, size_t size) {
	if(ptr==NULL){
	return my_malloc(size);
	}
	if(size==0){
	my_free(ptr);
	return NULL;
	}
	void* temp = my_malloc(size);
	memcpy(temp,ptr,size);
	my_free(ptr);
	ptr =temp;
	return ptr;
	
}

/* CALLOC
 * See my_malloc.h for documentation
 */
void* my_calloc(size_t nmemb, size_t size) {
	void* toReturn = my_malloc(nmemb*size);
	for (int i = 0; i < nmemb*size; i++) {
        	((char*)toReturn)[i] = 0 ;
        }
	return toReturn;	
}

/* FREE
 * See my_malloc.h for documentation
 */
void my_free(void* ptr) {
	metadata_t *toFree = (metadata_t*)((char*)ptr - sizeof(metadata_t));
	if(	(toFree->canary) != ((uintptr_t)toFree ^ toFree->size)){
		ERRNO =  CANARY_CORRUPTED;
		return ;
	}
	if(	 *(unsigned long*)((char*)toFree + toFree->size*WORD_SIZE -sizeof(long)) !=  ((uintptr_t)toFree ^ toFree->size)){
		ERRNO =  CANARY_CORRUPTED;
		return ;
	}
	if(freelist ==NULL){
			freelist = my_sbrk(SBRK_SIZE);
			if(freelist == NULL){
			ERRNO = OUT_OF_MEMORY;
			return;
			}
			freelist->size = SBRK_SIZE / WORD_SIZE  ;
			
			freelist->next = NULL;
			freelist->canary = (uintptr_t)freelist^ freelist->size;
			*(unsigned long*)((char*)freelist + freelist->size*WORD_SIZE-sizeof(long)) =  freelist->canary;
			ERRNO =NO_ERROR;
			return;
		}
	metadata_t *cur = freelist;
	metadata_t *pre = NULL;
	unsigned long start = (uintptr_t)toFree; 
	unsigned long end = (uintptr_t)((char*)toFree+toFree->size*WORD_SIZE);
	if( end<= (uintptr_t)freelist){
		if(end == (uintptr_t)freelist){
		toFree->next = freelist->next;
		toFree->size = toFree->size + freelist->size;
		toFree->canary = (uintptr_t)toFree ^ toFree->size;
		*(unsigned long*)((char*)toFree +toFree->size*WORD_SIZE -sizeof(long)) =  toFree->canary;
		freelist = toFree;
		ERRNO =NO_ERROR;
		return;
		}else{
		toFree->next = freelist;
		ERRNO =NO_ERROR;
		return;
		}
	}
	while(cur!=NULL ){
		if(pre !=NULL){
			if( (uintptr_t)((char*)pre+pre->size*WORD_SIZE)<=start && end<=(uintptr_t)cur ){
				if((uintptr_t)((char*)pre+pre->size*WORD_SIZE)==start && end==(uintptr_t)cur){
				pre->next = cur->next;
				cur->next = NULL;
				pre->size = pre->size + cur->size + toFree->size;
				pre->canary = (uintptr_t)pre ^ pre->size;
				*(unsigned long*)((char*)pre +pre->size*WORD_SIZE -sizeof(long)) =  pre->canary;
				ERRNO =NO_ERROR;
				return;
				}
				if((uintptr_t)((char*)pre+pre->size*WORD_SIZE)==start ){
					pre->size = pre->size + toFree->size;
					pre->canary = (uintptr_t)pre ^ pre->size;
					*(unsigned long*)((char*)pre +pre->size*WORD_SIZE -sizeof(long)) =  pre->canary;
					ERRNO =NO_ERROR;
					return;
				}
				else if(end==(uintptr_t)cur){
				
					pre->next = toFree;
					toFree ->next = cur->next;
					cur->next = NULL;	
					toFree -> size = toFree->size + cur->size;
					toFree->canary = (uintptr_t)toFree ^ toFree->size;
					*(unsigned long*)((char*)toFree +toFree->size*WORD_SIZE -sizeof(long)) =  toFree->canary;
					ERRNO =NO_ERROR;
					return;
				}
				else{
				pre->next = toFree;
				toFree->next = cur;
				ERRNO =NO_ERROR;
					return;
				}
				
			}
			
		}
	pre = cur;
	cur = cur->next;
	}
	if(start>=(uintptr_t)pre){
		if((uintptr_t)((char*)pre+pre->size*WORD_SIZE)==start){
		pre->size = pre->size + toFree->size;
		pre->canary = (uintptr_t)pre ^ pre->size;
		*(unsigned long*)((char*)pre +pre->size*WORD_SIZE -sizeof(long)) =  pre->canary;
		ERRNO =NO_ERROR;
		return;
		}
		else{
		pre->next = toFree;
		ERRNO =NO_ERROR;
		return;
		}
	}
}
