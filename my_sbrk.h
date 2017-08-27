#ifndef __MY_SBRK_H__
#define __MY_SBRK_H__

/* this function will emulate the system call sbrk(2). if you do not
 * have enough free heap space to satisfy a memory request, then you
 * must call this function to allocate memory to your allocator.
 */
void* my_sbrk(int);

#endif /* __MY_SBRK_H__ */
