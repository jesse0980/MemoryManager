#include <iostream>
#include <unordered_map>
#include <vector>
#include <functional>
#pragma once

using namespace std;

//allocator algorithms

inline int bestFit(int sizeInWords, void* list)
{

    int min = 100000;
    int ind = -1;
    uint16_t* holeList = static_cast<uint16_t*>(list);
    //uint16_t* listEntryPoint = holeList;
    uint16_t holeListlength = *holeList++;
    for(uint16_t i = 1; i < (holeListlength) * 2; i += 2) {
        if(holeList[i] >= sizeInWords && holeList[i] < min) {
            min = holeList[i];
            ind = holeList[i - 1];
        }
    }
    //delete [] listEntryPoint;
    return ind;
}

inline int worstFit(int sizeInWords, void* list)
{

    int max = 0;
    int ind = -1;
    uint16_t* holeList = static_cast<uint16_t*>(list);
    //uint16_t* listEntryPoint = holeList;
    uint16_t holeListlength = *holeList++;

    for(uint16_t i = 1; i < (holeListlength) * 2; i += 2) {
        if(holeList[i] >= sizeInWords && holeList[i] > max) {
            max = holeList[i];
            ind = holeList[i - 1];
        }
    }
    //delete [] listEntryPoint;
    return ind;
}



//best fit
// inline int bestFit(int sizeInWords, void* arrPtr){
//           vector<int>& arr = *static_cast<vector<int>*>(arrPtr);
//           // for(int i = 0; i < arr.size(); ++i){
//           //             cout << arr[i] << endl;
//           // }
//           int min = 100000;
//           int currInd = 0;
//           for(int i = 1; i < arr.size(); i+=2){
//           int start = arr[i];          
//           int count = arr[i+1];
//           if (count >= sizeInWords && count < min){
//             min = count;
//             currInd = start;
//           }
//           }
//           if(min == 100000){
//             return -1;
//           }
//          // cout << currInd << endl;
//           return currInd;
//     }
    //worst fit
    // inline int worstFit(int sizeInWords, void* arrPtr){
    //       vector<int>& arr = *static_cast<vector<int>*>(arrPtr);
    //       int max = 0;
    //       int currInd = 0;
    //       for(int i = 1; i < arr.size(); i+=2){
    //         int start = arr[i];          
    //         int count = arr[i+1];
    //         if (count >= sizeInWords && count > max){
    //           max = count;
    //           currInd = start;
    //         }
    //       }
    //       if(max == 0){
    //         return -1;
    //       }
    //       return currInd;
    // }

    
class MemoryManager{
    public:
     MemoryManager(unsigned wordSize, std::function<int(int, void *)> allocator);
    //Constructor; sets native word size (in bytes, for alignment) and default allocator for finding a memory hole.

      ~MemoryManager();
    //Releases all memory allocated by this object without leaking memory.

      void initialize(size_t sizeInWords);
    //Instantiates block of requested size, no larger than 65536 words; cleans up previous block if applicable.

      void shutdown();
    //Releases memory block acquired during initialization, if any. This should only include memory created for
    //long term use not those that returned such as getList() or getBitmap() as whatever is calling those should
    //delete it instead.

      void *allocate(size_t sizeInBytes);
    //Allocates a memory using the allocator function. If no memory is available or size is invalid, returns
    //nullptr.

      void free(void *address);
    //Frees the memory block within the memory manager so that it can be reused.

      void setAllocator(std::function<int(int, void *)> allocator);
    //Changes the allocation algorithm to identifying the memory hole to use for allocation.

      int dumpMemoryMap(char *filename);
    //Uses standard POSIX calls to write hole list to filename as text, returning -1 on error and 0 if successful.
    //Format: "[START, LENGTH] - [START, LENGTH] ...", e.g., "[0, 10] - [12, 2] - [20, 6]"

      void *getList2();

      void *getList();
    //Returns an array of information (in decimal) about holes for use by the allocator function (little-Endian).
    //Offset and length are in words. If no memory has been allocated, the function should return a NULL pointer.
    //Format: Example: [3, 0, 10, 12, 2, 20, 6]

      void *getBitmap();
    //Returns a bit-stream of bits in terms of an array representing whether words are used (1) or free (0). The
    //first two bytes are the size of the bitmap (little-Endian); the rest is the bitmap, word-wise.
    //Note : In the following example B0, B2, and B4 are holes, B1 and B3 are allocated memory.
    //Hole-0 Hole-1 Hole-2 ┌─B4─┐ ┌ B2┐ ┌───B0 ──┐ ┌─Size (4)─┐┌This is Bitmap in Hex┐
    //Example: [0,10]-[12,2]-[20,6][00 00001111 11001100 00000000]  [0x04,0x00,0x00,0xCC,0x0F,0x00]
    //┕─B3─┙ ┕B1┙, Returned Array: [0x04,0x00,0x00,0xCC,0x0F,0x00] or [4,0,0,204,15,0]

      unsigned getWordSize();
    //Returns the word size used for alignment.

      void *getMemoryStart();
    //Returns the byte-wise memory address of the beginning of the memory block.

      unsigned getMemoryLimit();
    //Returns the byte limit of the current memory block
    private:
        unsigned dWordSize;
        std::function<int(int, void *)> defAllocator;
        uint8_t* memStart;
        unsigned memLimit;
        unordered_map<int, int> allocation_table;
        std::vector<int> arr;

};










/*#include <cstring>
#include <cstdio>
#include <functional>

class MemoryManager {
public:
    MemoryManager(unsigned wordSize, std::function<int(int, void *)> allocator);
    ~MemoryManager();
    void initialize(size_t sizeInWords);
    void shutdown();
    void *allocate(size_t sizeInBytes);
    void free(void *address);
    void setAllocator(std::function<int(int, void *)> allocator);
    int dumpMemoryMap(char *filename);
    void *getList();
    void *getBitmap();
    unsigned getWordSize();
    void *getMemoryStart();
    unsigned getMemoryLimit();
private:
    unsigned dWordSize;
    std::function<int(int, void *)> defAllocator;
    void* memStart;
    unsigned memLimit;
    int* arr;
};

MemoryManager::MemoryManager(unsigned wordSize, std::function<int(int, void *)> allocator)
    : dWordSize(wordSize), defAllocator(allocator), memStart(nullptr), memLimit(0), arr(nullptr) {}

MemoryManager::~MemoryManager() {
    shutdown();
}

void MemoryManager::initialize(size_t sizeInWords) {
    shutdown();

    if (sizeInWords > 65536) {
        sizeInWords = 65536;
    }

    memStart = new char[sizeInWords * dWordSize];
    memLimit = sizeInWords * dWordSize;

    arr = new int[sizeInWords];
    memset(arr, 0, sizeInWords * sizeof(int));
}

void MemoryManager::shutdown() {
    if (memStart) {
        delete[] static_cast<char*>(memStart);
        memStart = nullptr;
    }

    if (arr) {
        delete[] arr;
        arr = nullptr;
    }

    memLimit = 0;
}

void *MemoryManager::allocate(size_t sizeInBytes) {
    if (sizeInBytes == 0 || !memStart) {
        return nullptr;
    }

    int sizeInWords = (sizeInBytes + dWordSize - 1) / dWordSize;
    int index = defAllocator(sizeInWords, arr);

    if (index == -1) {
        return nullptr;
    }

    void* result = static_cast<char*>(memStart) + index * dWordSize;
    memset(result, 0, sizeInWords * dWordSize);
    arr[index] = sizeInWords;

    return result;
}

void MemoryManager::free(void *address) {
    if (!memStart || !address) {
        return;
    }

    int index = static_cast<char*>(address) - static_cast<char*>(memStart);
    index /= dWordSize;

    if (index < 0 || index >= memLimit / dWordSize) {
        return;
    }

    int sizeInWords = arr[index];
    arr[index] = 0;

    for (int i = index + sizeInWords - 1; i >= index; i--) {
        arr[i] = 0;
    }
}

void MemoryManager::setAllocator(std::function<int(int, void *)> allocator) {
    defAllocator = allocator;
}

int MemoryManager::dumpMemoryMap(char *filename) {
    if (!memStart) {
        return -1;
    }

    FILE* fp = fopen(filename, "w");

    if (!fp) {
        return -1;
    }

    bool first = true;

    for (int i = 0; i < memLimit / dWordSize; i++) {
        if (arr[i] == 0) {
            if (!first) {
                fprintf(fp, " - ");
        else {
            first = false;
            }
        int start = i;
        int length = 1;
        while (i + 1 < memLimit / dWordSize && arr[i + 1] == 0) {
        length++;
        i++;
        }
        fprintf(fp, "[%d, %d]", start, length);
        }
        fclose(fp);
        return 0;
        }
            }*/