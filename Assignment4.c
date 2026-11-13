#include <stdio.h>
#include <stdlib.h>

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


int main(){ 

    int page_table[NUM_PAGES];
    char buff[BUFF_SIZE];

    // Initialize all values with -1 
    for(int i = 0; i < NUM_PAGES; i ++){ 
        page_table[i] = -1; 
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

        // Use page num to index TLB 
        // Check if exists in there, if doesn't then go to page table 

        // Query TLB

        // IF page num in TLB get frame number 
        // Else go to page table
        //  If in PT get frame number
        //  else page fault


        // Must add entry to Page in TLB in either case
    }

}

