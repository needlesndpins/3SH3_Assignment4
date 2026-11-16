#include <stdio.h>
#include <stdlib.h>
#include <sys/mman.h>
#include <fcntl.h>
#include <unistd.h>
#include <string.h>

// Logical address sapce: 2^16 bytes
// Physical address space: 2^15 bytes 
// Page size: 2^8 bytes = 256 bytes 
// Maximum entries in TLB = 16 (Page table cache) 

// Requirements
// 1. Simulate a MMU - Translates logical address to physical address
//   Check if LA exists in TLB 
//     If not in TLB check PT to see if in memory 
//       If not in memory then page fault 
//    LA -> TLB -> Hit (FOUND) 
//    LA -> TLB -> Miss -> PT -> (Found or Page Fault)
// Page fault: copying the page from backing store into memory 
//             Since LA space > Physical address space, page request
//             might involve replacing a page in memory wiuth a new page 
//             USE FIFO page replacement policy

// Part 1: 
// Use addresses.txt to simulate addresses (Integers storing addresses)
// Open addresses.txt (fopen())
//  Read each address from the file 
//  Compute page number and offset 
//  After computing query TLB 
#define NUM_TLB 16 // Num of entries in TLB
#define NUM_PAGES 256 // (2^16 / 2^8 = 2^8)
#define OFFSET_BITS 8 // 2^8 bytes within a page
#define PAGE_BITS 8   // 2^8 pages
#define PAGE_SIZE 256 // 2^8 bytes
#define OFFSET_MASK 255 // page size - 1
#define BUFF_SIZE 16

#define FILE_SIZE 65536 // Num bytes for the binary file

#define PHYSICAL_SIZE 32768 // Physical memory size 2^(15) bytes

typedef struct{
    int pageNum;
    int frameNum;
} TLBentry;

TLBentry TLB[NUM_TLB] = {0}; // Initialize to empty 
int next = 0;

// Page table and physical mem
int page_table[NUM_PAGES];
signed char *physical_mem; 
int next_physical = 0; // Store index of next 

// Backing store related info
int mmapfile_fd;
signed char *mapfptr;


//NOTE: check if this should return the frame number or the TLB entry itself
//searches TLB and returns frame number corresponding to given page number
int search_TLB(int page){
    for(int i = 0; i < NUM_TLB; i++){
        if(TLB[i].pageNum == page){
            return TLB[i].frameNum;
        }
    }
    return -1; //failed to find entry return -1
}

//adds to TLB in FIFO matter
void TLB_Add(int page, int frame){
    TLB[next].pageNum = page;
    TLB[next].frameNum = frame;

    next = (next + 1) % NUM_TLB; 
    return;
}

//updates TLB entry in place or if no page entry found adds the given entry
void TLB_Update(int page, int frame){
    for(int i = 0; i < NUM_TLB; i++){
        if(TLB[i].pageNum == page){
            TLB[i].frameNum = frame;
            return;
        }
    }
    TLB_Add(page, frame); //failed to find page in TLB so adds new TLB entry
    return;
}

// Handle page faults 
// Return frame number
int page_fault(int page_num, int offset){ 
    // Populate signed char physical memory 
    // Add to page table 
    // (LATER ADD TO TLB)
    // find_page(page_num); // Find the page from backing store

    // When copying to physical memory,
    //  copy the bytes of the backing store (all 256 bytes of them)
    //  Since each entry stores 1 byte, need to store the bytes
    
    //memcpy(destination, source, number_of_bytes);

    memcpy(physical_mem + next_physical, mapfptr + (page_num * PAGE_SIZE), PAGE_SIZE);

    int frame_num = next_physical / PAGE_SIZE;

    // Checking for page replacement
    for(int i = 0 ; i < NUM_PAGES ; i ++){ 
        if(page_table[i] == frame_num){ 
            page_table[i] = -1; // Set it to -1 since removed from memory
            break;
        }
    }
    // Now update this entry to frame_num
    page_table[page_num] = frame_num;    

    // Next physical location
    next_physical = (next_physical + PAGE_SIZE) % PHYSICAL_SIZE; 

    // Where we put page in physical memory (index) is the frame number
    // Put this in page table

    return frame_num; 
}

void update_pagetable(){ 

}

// // Go through the backingstore and find the page corresponding to page number
// void find_page(int page_num){

//     // Find the page in backing store (Base off index of page num)
//     //  E.x page 0 is first entry in backing store
//     // Once found, copy all PAGE_SIZE bytes into physical memory at frame f
//     // Then set page_table[page_num] = f

//     // memcpy -> physical_space pos 4; 
//     // page_table[page_num] = 4;



//     // Find page in Backing store based on index 

//     // return that position
// }


int main(){ 

    // int page_table[NUM_PAGES];
    char buff[BUFF_SIZE];

    // Open backing store in read only mode
    // Return the file descriptor (INT OS uses to ID the file)
    mmapfile_fd = open("BACKING_STORE.bin",O_RDONLY);

    if (mmapfile_fd == -1) {
        perror("open");
        return 1;
    }
    // Map file into memory 
    mapfptr = mmap(0,FILE_SIZE,PROT_READ, MAP_PRIVATE, mmapfile_fd,0);

     if (mapfptr == MAP_FAILED) {
        perror("mmap");
        close(mmapfile_fd);
        return 0;
    }

    // signed char *physical_mem; 
    physical_mem = (signed char *)malloc(PHYSICAL_SIZE);
    if (physical_mem == NULL) {
        perror("malloc failed");
        exit(EXIT_FAILURE);
    }


    // Initialize all values with -1 
    for(int i = 0; i < NUM_PAGES; i ++){ 
        page_table[i] = -1; 
    }

    // Initialize all TLB entries with -1 
    for (int i = 0; i < NUM_TLB; i++) {
        TLB[i].pageNum = -1;
        TLB[i].frameNum = -1;
    }

    FILE* fptr = fopen("addresses.txt", "r");
    if(fptr == NULL){ 
        return 0;
    }

    // Read all lines from addresses.txt
    while(fgets(buff,BUFF_SIZE,fptr) != NULL){ 
        // Compute page number and offset for each logical address

        int logical_add = atoi(buff);
        // printf("%d\n",logical_add);

        // Page Number = logical address >> OFFSET_BITS 
        int page_num = logical_add >> OFFSET_BITS;

        // Page offset = logical address & offset_MASK 
        int offset = logical_add & OFFSET_MASK;

        int frame_num; 

        // Use page num to index TLB 
        // Check if exists in there, if doesn't then go to page table 

        // Query TLB

        // IF page num in TLB get frame number 
        // Else go to page table
        //  If in PT get frame number
        //  else page fault
        // Page not in page table
        if(page_table[page_num] == -1){ 
            frame_num = page_fault(page_num,offset);
        }

        int physical_add = (frame_num << PAGE_BITS) | offset;

        //Virtual address: 16916 Physical address = 20 Value=0 
        int val = physical_mem[physical_add];

        printf("Virtual address: %d Physical address = %d Value = %d\n",
        logical_add, physical_add, val);

        // Must add entry to Page in TLB in either case



    }

    // Free and close memory mapped file
    munmap(mapfptr, FILE_SIZE);
    close(mmapfile_fd);
    free(physical_mem);

    return 0;

}

