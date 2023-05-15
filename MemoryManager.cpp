#include "MemoryManager.h"
#include <vector>
#include <string>
#include <iostream>
#include <fstream>
#include <functional>
#include <cmath>
#include <unistd.h>
#include <fcntl.h>
#include <sys/stat.h>
#include <sys/types.h>

using namespace std;
    





    //Constructor; sets native word size (in bytes, for alignment) and default allocator for finding a memory hole.

      MemoryManager::MemoryManager(unsigned wordSize, std::function<int(int, void *)> allocator){
        dWordSize = wordSize;
        defAllocator = allocator;
        memStart = nullptr;
        memLimit = 0;

      }

      MemoryManager::~MemoryManager(){
            shutdown();
      }
    //Releases all memory allocated by this object without leaking memory.

      void MemoryManager::initialize(size_t sizeInWords){
            
                shutdown();
            
            const size_t maxSizeInWords = 65536;
            sizeInWords = min(sizeInWords, maxSizeInWords);

            //declare dynamic chunk of memory
            // char* memBlock= new char[sizeInWords * dWordSize];
            // memStart =  reinterpret_cast<uint32_t*>(memBlock);

            memStart = new uint8_t[sizeInWords * dWordSize];
            memLimit = sizeInWords * dWordSize;
            //fill(memStart, memStart + memLimit, 65);

            // for the array method
            arr = vector<int>(sizeInWords, 1);

            //for the linked list method
            // head = new Node();
            // head->offset = 0;
            // head->size = sizeInWords;
            // head->next = nullptr;

      }
    //Instantiates block of requested size, no larger than 65536 words; cleans up previous block if applicable.

      void MemoryManager::shutdown(){
        
            delete[] memStart;
            memStart = nullptr;
            memLimit = 0;
        
      }
    //Releases memory block acquired during initialization, if any. This should only include memory created for
    //long term use not those that returned such as getList() or getBitmap() as whatever is calling those should
    //delete it instead.

      void* MemoryManager::allocate(size_t sizeInBytes){
            size_t sizeInWords = ceil((float)sizeInBytes / dWordSize);
             if (sizeInWords <= 0 || sizeInWords > arr.size() || !memStart) {
                    return nullptr;
                }
            void* list = getList();
            int index = defAllocator(sizeInWords, list);
            delete[] list;
            if(index == -1){
              cout << "didn't find index";
                return nullptr;
            }
            else{
              //cout << index << ", size : " << sizeInWords << endl;
                for(int i = index; i < index + sizeInWords; ++i){
                        arr[i] = 0;
                }
                
            }

            allocation_table[index] = sizeInWords;
            uint8_t* actualMemoryAdd = memStart + (index * dWordSize);
            cout << "actual: " << index << " Size: " << sizeInWords << endl;
            return actualMemoryAdd;
      }
    //Allocates a memory using the allocator function. If no memory is available or size is invalid, returns
    //nullptr.

      void MemoryManager::free(void *address){
            uint8_t* newAddress = reinterpret_cast<uint8_t*>(address);
            //cout << "deleteAddress: " << newAddress << endl;
            //int temp = reinterpret_cast<intptr_t>(memStart);
            int fakeAddress = reinterpret_cast<intptr_t>(newAddress - memStart) / dWordSize;
            //cout << "free Addy: " << fakeAddress << endl;
            int size = allocation_table[fakeAddress];
            
            for(int i = fakeAddress; i < fakeAddress + size; i++){
                arr[i] = 1;
            }

          allocation_table.erase(fakeAddress);
          //vector<int>& temps = *static_cast<vector<int>*>(getList2());
          //vector<uint8_t> tempie = *static_cast<vector<uint8_t>*>(getBitmap());

          // cout << "list after free: " << endl;
          // for(int i = 0; i < tempie.size(); ++i){
          //   cout << tempie[i] << endl;
          // }
      }
    //Frees the memory block within the memory manager so that it can be reused.

      void MemoryManager::setAllocator(std::function<int(int, void *)> allocator){
        defAllocator = allocator;
      }
    //Changes the allocation algorithm to identifying the memory hole to use for allocation.

      int MemoryManager::dumpMemoryMap(char *filename){
        int fd = open(filename, O_WRONLY | O_CREAT | O_TRUNC, S_IRUSR | S_IWUSR);
        if (fd == -1) {
            return -1;
          }

        void* temp = getList();
        uint16_t* holeList = static_cast<uint16_t*>(temp);
        uint16_t* listEntryPoint = holeList;
        uint16_t holeListlength = *holeList++;

        for(int i = 1; i < (holeListlength) * 2; i += 2) {
            string line = "yes";
          if(i != ((holeListlength)*2) - 1){
             line = "[" + to_string(holeList[i - 1]) + ", " + to_string(holeList[i]) + "] - ";
          }
          else{
              line = "[" + to_string(holeList[i - 1]) + ", " + to_string(holeList[i]) + "]";
          }
          write(fd, line.c_str(), line.size());
        }
        delete [] listEntryPoint;


        // int* listArr = reinterpret_cast<int*>(temp);
        // for(int i = 1; i < sizeof(listArr); i+=2){
        //   string line = "yes";
        //   if(i != sizeof(listArr) - 2){
        //      line = "[" + to_string(listArr[i]) + ", " + to_string(listArr[i+1]) + "] - ";
        //   }
        //   else{
        //       line = "[" + to_string(listArr[i]) + ", " + to_string(listArr[i+1]) + "]";
        //   }
        //   write(fd, line.c_str(), line.size());
        // }
        // delete[] temp;


        return 0;
      }
    //Uses standard POSIX calls to write hole list to filename as text, returning -1 on error and 0 if successful.
    //Format: "[START, LENGTH] - [START, LENGTH] ...", e.g., "[0, 10] - [12, 2] - [20, 6]"
      void* MemoryManager::getList(){
          vector<uint16_t> holes;
          for(int i = 0; i < arr.size(); i++){
            uint16_t start = i;
            if (arr[i] == 0){
                continue;
            }            
            uint16_t count = 0;
              while(arr[i] != 0 && i < arr.size()){
                  i++;
                  count++;
              }
            holes.push_back(start);
            holes.push_back(count);
          }
          holes.insert(holes.begin() + 0, holes.size() / 2);
          int n = holes.size();
          uint16_t* temp = new uint16_t[n];
          for(int i = 0; i < holes.size(); ++i){
            temp[i] = holes[i];
          }
          //vector<uint16_t>* temp = new vector<uint16_t>(holes);
          //return static_cast<void*>(temp->data());
          return temp;
      }


      void* MemoryManager::getList2(){
        vector<int> holes;
        for(int i = 0; i < arr.size(); i++){
          int start = i;
          if (arr[i] == 0){
              continue;
          }            
          int count = 0;
            while(arr[i] != 0 && i < arr.size()){
                i++;
                count++;
            }
          holes.push_back(start);
          holes.push_back(count);
        }
        holes.insert(holes.begin() + 0, holes.size() / 2);
        int n = holes.size();
          int* temp = new int[n];
          for(int i = 0; i < holes.size(); ++i){
            temp[i] = holes[i];
          }
        //return static_cast<void*>(temp->data());
        return temp;
      }
    //Returns an array of information (in decimal) about holes for use by the allocator function (little-Endian).
    //Offset and length are in words. If no memory has been allocated, the function should return a NULL pointer.
    //Format: Example: [3, 0, 10, 12, 2, 20, 6]

      void* MemoryManager::getBitmap(){
        //everything has to be in size 8, convert to hex, 
        vector<uint8_t> bitMap;
        uint16_t numBytes = (arr.size() + 7) / 8; 

        // Add the size of the bitmap (little-endian)
        bitMap.push_back(numBytes & 0xff);
        bitMap.push_back(numBytes >> 8);

        
        // Add the bitmap, word-wise
        for (int i = 0; i <= arr.size(); i += 8) {
            unsigned char byte = 0;
            int j = 7;
            for (arr.size() - i > 7 ?  j = 7 : j = arr.size() - i - 1; j >=0; j--) {
                  int bit = 0;
                  if (arr[i+j] == 0) {
                      bit = pow(2, j);
                  } 
                  byte += bit;
            }
            bitMap.push_back(byte);
        }

        int n = bitMap.size();
          uint8_t* temp = new uint8_t[n];
          for(int i = 0; i < bitMap.size(); ++i){
            temp[i] = bitMap[i];
          }
        //vector<uint8_t>* temp = new vector<uint8_t>(bitMap);

        return temp;
      }
    //Returns a bit-stream of bits in terms of an array representing whether words are used (1) or free (0). The
    //first two bytes are the size of the bitmap (little-Endian); the rest is the bitmap, word-wise.
    //Note : In the following example B0, B2, and B4 are holes, B1 and B3 are allocated memory.
    //Hole-0 Hole-1 Hole-2 ┌─B4─┐ ┌ B2┐ ┌───B0 ──┐ ┌─Size (4)─┐┌This is Bitmap in Hex┐
    //Example: [0,10]-[12,2]-[20,6][00 00001111 11001100 00000000]  [0x04,0x00,0x00,0xCC,0x0F,0x00]
    //┕─B3─┙ ┕B1┙, Returned Array: [0x04,0x00,0x00,0xCC,0x0F,0x00] or [4,0,0,204,15,0]

/*            int byte = 0;
            for (int i = arr.size(); i >= 0; i -= 8) {
                for (int j = 7; j >=0 && i-j >= 0; j--) {
                      int bit = 0;
                      if (arr[i-j] == 0) {
                          bit = pow(2, j);;
                      } 
                      byte += bit;
                }
                bitMap.push_back(byte);
        }
        return bitMap;
      }*/


      unsigned MemoryManager::getWordSize(){
        return dWordSize;
      }
    //Returns the word size used for alignment.

      void* MemoryManager::getMemoryStart(){
        return reinterpret_cast<uint64_t*>(memStart);
      }
    //Returns the byte-wise memory address of the beginning of the memory block.

      unsigned MemoryManager::getMemoryLimit(){
        return memLimit;
      }
    //Returns the byte limit of the current memory block
