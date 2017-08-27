Rules of Allocating:

a. If the block is exactly the same size,simply remove it from the freelist, set the
canaries, and return the block.
b. If the block is big enough to house a new block, we need to split off the portion we will
use. 
c. If the previous two conditions fail, then the smallest block? that’s big enough should
be selected? and removed from the freelist, its canaries set, and the block returned. (Note:
Any extra space not big enough to split into a new block should be included as part of
the user block before? the end canary).
d. If no suitable blocks are found at all, then call my_sbrk with SBRK_SIZE ?to get more
memory.


