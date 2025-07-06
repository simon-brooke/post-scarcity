
# Free list

A free list is a list of FREE cells consed together. When a cell is deallocated, it is consed onto the front of the free list, and the system free-list pointer is updated to point to it. A cell is allocated by popping the front cell off the free list.

If we attempt to allocate a new cell and the free list is empty, we allocate a new code page, cons all its cells onto the free list, and then pop the front cell off it.

However, because we wish to localise volatility in memory in order to make maintaining a consistent backup image easier, it may be worth maintaining a separate free list for each page, and allocating cells not from the front of the active free list but from the free list of the currently most active page.
