#include "../h/MemoryAllocator.hpp"

#define NULL 0


MemoryAllocator::MemoryAllocator() {
    freeChunks = (ChunkHeader*)HEAP_START_ADDR;

    freeChunks->magicNum = CHUNK_MAGIC_FREE;
    freeChunks->size = bytesToBlocksFloor((char*)HEAP_END_ADDR - (char*)HEAP_START_ADDR) - HEADER_SIZE_BLOCKS;
    freeChunks->nextFree = NULL;
    freeChunks->prevFree = NULL;
}

MemoryAllocator* MemoryAllocator::Instance() {
    static MemoryAllocator instance;
    return &instance;
}

void* MemoryAllocator::countChunkStartAddr(ChunkHeader* ch) {
    return (void*)((char*)ch + blocksToBytes(HEADER_SIZE_BLOCKS));
}
// first-fit algorithm for continious memory allocation
void* MemoryAllocator::kmemAlloc(size_t size) {
    ChunkHeader* curr = freeChunks;
    while (curr) {
        if(curr->size >= size) break;
        curr = curr->nextFree;
    }
    // If there is no free chunk with size blocks we return NULL
    if (curr == NULL) return NULL;

    // The whole curr chunk is allocated (there is no free fragment left)
    if (curr->size <= size + HEADER_SIZE_BLOCKS) {
        // unlink curr from free chunks list and set it as used
        if (curr->prevFree) curr->prevFree->nextFree = curr->nextFree;
        else freeChunks = curr->nextFree;

        if(curr->nextFree) curr->nextFree->prevFree = curr->prevFree;

        curr->magicNum = CHUNK_MAGIC_USED;
        return countChunkStartAddr(curr);
    }

    // curr->size > size + HEADER_SIZE_BLOCKS, there will be free chunk left
    // create a new used chunk at the end of the curr free chunk
    // update curr size
    curr->size -= (HEADER_SIZE_BLOCKS + size);
    ChunkHeader* newUsed = (ChunkHeader*)((char*)countChunkStartAddr(curr) + blocksToBytes(curr->size));
    newUsed->magicNum = CHUNK_MAGIC_USED;
    newUsed->size = size;

    return countChunkStartAddr(newUsed);
}

int MemoryAllocator::kmemFree(void* mem) {

    if (mem == nullptr) {
        return -1;
    }
    ChunkHeader* newFree = (ChunkHeader*)((char*)mem - blocksToBytes(HEADER_SIZE_BLOCKS));
    if(newFree == NULL) return 0;
    if(newFree->magicNum != CHUNK_MAGIC_USED) return -1;
    
    newFree->magicNum = CHUNK_MAGIC_FREE;
    
    // find place for new free chunk in freeChunks list
    // which has its elements sorted by start address
    ChunkHeader* prev = NULL; // newFree will inserted after prev 
    ChunkHeader* next = freeChunks;    //address of the first free chunk
    while(next) {
        if((char*)newFree < (char*)next) break;
        prev = next;
        next = next->nextFree;
    }
    newFree->nextFree = next;
    newFree->prevFree = prev;
    if(prev) prev->nextFree = newFree;
    else freeChunks = newFree;
    if(next) next->prevFree = newFree;

    tryMerge(newFree, next);
    tryMerge(prev, newFree);
    return 0;
}

int MemoryAllocator::tryMerge(ChunkHeader* left, ChunkHeader* right) {
    
    if(left == NULL) return 0;
    if(right && (char*)countChunkStartAddr(left) + blocksToBytes(left->size) == (char*)right){
        left->size += (HEADER_SIZE_BLOCKS + right->size);
        left->nextFree = right->nextFree;
        if(left->nextFree) left->nextFree->prevFree = left;
        return 1;
    }
    return 0;

}

size_t MemoryAllocator::bytesToBlocksCeil(size_t numBytes) {
    return (numBytes + MEM_BLOCK_SIZE - 1) / MEM_BLOCK_SIZE;
}
size_t MemoryAllocator::bytesToBlocksFloor(size_t numBytes) {
    return numBytes / MEM_BLOCK_SIZE;
}
size_t MemoryAllocator::blocksToBytes(size_t numBlocks){
    return numBlocks*MEM_BLOCK_SIZE;
}


