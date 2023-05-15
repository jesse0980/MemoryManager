# MemoryManager
Created custom memory manager class for a Linux operating system. This manager allocates memory of the size given and returns the address of the memory start. It tracks memory with a bitmap and allocates memory by default using a first fit algorithm. This can be changed by setting the allocator function. 
