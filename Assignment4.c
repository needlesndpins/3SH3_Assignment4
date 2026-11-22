#include <stdio.h>
#include <stdlib.h>
#include <sys/mman.h>
#include <fcntl.h>
#include <unistd.h>
#include <string.h>

// Logical address sapce: 2^16 bytes
// Physical address space: 2^15 bytes 
// Page size: 2^8 bytes = 256 bytes 
// Frame size: 2^8 bytes = 256 bytes
// Maximum entries in TLB = 16 (Page table cache) 

#define NUM_TLB 16 // Num of entries in TLB
#define NUM_PAGES 256 // (2^16 / 2^8 = 2^8)
#define OFFSET_BITS 8 // 2^8 bytes within a page
#define PAGE_BITS 8   // 2^8 pages
#define PAGE_SIZE 256 // 2^8 bytes
#define OFFSET_MASK 255 // page size - 1
#define BUFF_SIZE 16
#define FILE_SIZE 65536 // Num bytes for the binary file
#define PHYSICAL_SIZE 32768 // Physical memory size 2^(15) bytes

// Define TLB entry struct
typedef struct{
    int pageNum;
    int frameNum;
} TLBentry;

// Create TLB array of TLBentrys
TLBentry TLB[NUM_TLB] = {0}; // Initialize to empty 
int next = 0;  // index for next available TLB spot

int page_table[NUM_PAGES];  // Page table

signed char *physical_mem;  // Physical memory
int next_physical = 0;      // Store index of next available spot in physical memory

// Backing store related info
int mmapfile_fd;
signed char *mapfptr;


// Searches TLB for entry corresponding to provided page number 
// Returns framenum if found, else -1
int search_TLB(int page){
    for(int i = 0; i < NUM_TLB; i++){
        if(TLB[i].pageNum == page){
            return TLB[i].frameNum;
        }
    }
    return -1; //failed to find entry return -1
}

// Add to TLB using circular array (FIFO)
void TLB_Add(int page, int frame){
    TLB[next].pageNum = page;
    TLB[next].frameNum = frame;

    next = (next + 1) % NUM_TLB; 
    return;
}

//updates TLB entry in place or if no page entry found adds the given entry
void TLB_Update(int page, int frame){
    for(int i = 0; i < NUM_TLB; i++){
        if(TLB[i].frameNum == frame){
            TLB[i].pageNum = page;
            return;
        }
    }
    TLB_Add(page, frame); //failed to find page in TLB so adds new TLB entry
    return;
}

// Handle page faults 
// Return frame number
int page_fault(int page_num, int offset){ 
    
    // Copy bytes of backing store into physical mem + next_physical location (offset)
    // Copy PAGE_SIZE bytes starting from mapfptr + offset 
    memcpy(physical_mem + next_physical, mapfptr + (page_num * PAGE_SIZE), PAGE_SIZE);

    // each entry is PAGE_SIZE so get frame num by dividing to get index
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

    return frame_num; 
}


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
    // Allocate spaace for physical memory
    physical_mem = (signed char *)malloc(PHYSICAL_SIZE);
    if (physical_mem == NULL) {
        perror("malloc failed");
        exit(EXIT_FAILURE);
    }

    // Initialize all values of page table with -1 
    for(int i = 0; i < NUM_PAGES; i ++){ 
        page_table[i] = -1; 
    }

    FILE* fptr = fopen("addresses.txt", "r");
    if(fptr == NULL){ 
        return 0;
    }
    int num_faults = 0;
    int num_addresses = 0;
    int num_TLBhits = 0;

    // Read all lines from addresses.txt
    while(fgets(buff,BUFF_SIZE,fptr) != NULL){ 
        // Compute page number and offset for each logical address

        int logical_add = atoi(buff);

        // Page Number = logical address >> OFFSET_BITS 
        int page_num = logical_add >> OFFSET_BITS;

        // Page offset = logical address & offset_MASK 
        int offset = logical_add & OFFSET_MASK;

        int frame_num; 

        // If not found in TLB
        if(search_TLB(page_num) == -1){

            // If not in page table 
            if(page_table[page_num] == -1){ 
                frame_num = page_fault(page_num,offset);
                num_faults ++;
            }
            else{ 
                // Get from page table
                frame_num = page_table[page_num];
            }
            // Add to TLB 
            TLB_Update(page_num,frame_num);
        }else{ // In TLB so retrieve it 
            frame_num = search_TLB(page_num);
            num_TLBhits++;
        }
        // Compute physical address
        int physical_add = (frame_num << PAGE_BITS) | offset;

        // Get the value at this physical address
        int val = physical_mem[physical_add];

        printf("Virtual address: %d Physical address = %d Value = %d\n",
        logical_add, physical_add, val);
        num_addresses ++;
    }

    printf("Total addresses = %d\n",num_addresses);
    printf("Page_faults = %d\n",num_faults);
    printf("TLB Hits = %d\n",num_TLBhits);

    // Free and close memory mapped file
    munmap(mapfptr, FILE_SIZE);
    close(mmapfile_fd);
    free(physical_mem);

    return 0;
}