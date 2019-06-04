/**
 * allocator.c
 *
 * Explores memory management at the C runtime level.
 *
 * Author: <your team members go here>
 *
 * Compile: gcc -o allocator.so -Wall -fPIC -shared allocator.c
 *
 * To use (one specific command):
 * LD_PRELOAD=$(pwd)/allocator.so command
 * ('command' will run with your allocator)
 *
 * To use (all following commands):
 * export LD_PRELOAD=$(pwd)/allocator.so
 * (Everything after this point will use your custom allocator -- be careful!)
 */

#include <stdio.h>
#include <stdlib.h>
#include <sys/mman.h>
#include <string.h>
#include <unistd.h>
#include <stdbool.h>

#define DEBUG 0
#define LOG(fmt, ...) \
        do { if (DEBUG) fprintf(stderr, "%s:%d:%s(): " fmt, __FILE__, \
                                __LINE__, __func__, __VA_ARGS__); } while (0)

struct mem_block {
    /* Size of the memory region */
    size_t size;

    /* Space used */
    size_t used;

    /* Whether or not this region has been freed */
    bool free;

    /* Name/description for our block of memory. */
    char description[64];

    /* Next block in the chain */
    struct mem_block *next;
};

struct mem_block *g_head = NULL;
struct mem_block *tail = NULL;
unsigned long g_allocations = 0;

/*
*print block
*input: mem_block address - memory location of the header for block data
*output: prints the block data
*/
void print_block(struct mem_block *addr) {
    struct mem_block *block = addr;
    char print[500];
    sprintf( print, "\"%s\" : size %zu: space: %zu: status: [%s]\n",
         block->description, block->size, block->used, (block->free == false ? "In Use" : "Free"));
    write(fileno(stdout), print, strlen(print));
}
/*
* malloc_description
* input: size - size of memory to be allocated, description - description to be added to block
* output: memory location of the data block
*/
void *malloc_description(size_t size, char *description){
    char *algo = getenv("ALLOCATOR_ALGORITHM");
    char *scribble= getenv("ALLOCATOR_SCRIBBLE");
    struct mem_block *tmp = g_head;
    struct mem_block *best_fit = NULL;
    struct mem_block *worst_fit = NULL;
    size_t best_size = 99999;
    size_t worst_size = 0;
    //check for the algorithm selected in environment variable
    if(algo == NULL){
        algo = "first_fit";
    }
    if(scribble == NULL){
        scribble = "0";
    }
    //iterates through blocks already present, checks for the algorithm and finds appropriate block

    if(g_head != NULL){
        while(tmp && tmp->next != tmp){
            struct mem_block *next = (struct mem_block *)tmp->next;
            if(tmp->free == true && tmp->size >= size){
                if(strcmp("first_fit", algo) == 0){
                    tmp->used = tmp->size - size;
                    tmp->free = false;
                    return tmp + 1;
                }
                if(strcmp("best_fit", algo) == 0){
                    if(tmp->size - size < best_size){
                        best_size = tmp->size - size;
                        best_fit = tmp;
                    }
                }
                if(strcmp("worst_fit", algo) == 0){
                    if(tmp->size > worst_size){
                        worst_size = tmp->size;
                        worst_fit = tmp;
                    }
                }
            }
            tmp = next;
        }
        if(strcmp("best_fit", algo) == 0 && best_fit != NULL){
            best_fit->used = best_fit->size - size;
            best_fit->free = false;
            return best_fit + 1;
        }
        else if(strcmp("worst_fit", algo) == 0 && worst_fit != NULL){
            worst_fit->used = worst_fit->size - size;
            worst_fit->free = false;
            return worst_fit + 1;
        }
    }
    //allocating necessary parameters for block data
    char *desc = description;
    int page_size = getpagesize();
    size_t usage = size + sizeof(struct mem_block);
    size_t num_pages = usage / page_size;
    if(size % page_size != 0){
        num_pages++;
    }
    size_t block_size = num_pages + page_size;

    LOG("Allocation Request: %zu bytes (%zu page(s) / %zu bytes\n",
        size, num_pages, block_size);
    //mmap maps memory of the block size, gives protection bits and maps it to private
    struct mem_block *block = mmap(
        NULL,
        block_size,
        PROT_READ | PROT_WRITE,
        MAP_PRIVATE | MAP_ANONYMOUS,
        -1,
        0
    );
    if(block == MAP_FAILED){
        perror("mmap failed");
        return NULL;
    }
    //sets parameters for block
    g_allocations++;
    block->size = size;
    block->used = block_size - size;
    block->free = false;
    block->next = NULL;
    if(desc == NULL){
        sprintf(block->description, "Allocation %lu", g_allocations);
    }
    else{
        strcpy(block->description, desc);
    }
    //sets the head and tail if not already present 
    if(g_head == NULL){
        g_head = block;
        tail = block;
    }
    
    void *start_addr = block + 1;
    LOG("Allocation successful; address of '%s': %p\n",
        block->description, start_addr
    );
    tail->next = block;
    tail = tail->next;
    //scribble fills malloc data with 0xAA if environment variable set to 1
    if(strcmp(scribble,"1") == 0){
        memset(start_addr, 0xAA,size);
    }
    return start_addr;
}
/*
* malloc calls malloc description but sets the description to NULL by default
*/
void *malloc(size_t size){
    return malloc_description(size, NULL);
}
/*
* input: description - description to look for in the block chain
* output: address of block found, or NULL if not there
*/
void *malloc_lookup(char *description){
    struct mem_block *tmp = g_head;
    while(tmp){
        if(strcmp(tmp->description, description) == 0){
            return tmp + 1;
        }
        tmp = tmp->next;
    }
    return NULL;
}
/* free
* input: pointer - pointer address of block
* sets the block to free and used memory to 0
*/
void free(void *ptr){
    if(ptr == NULL){
        return;
    }
    LOG("Freeing block at: %p\n", ptr);
    struct mem_block *block = (struct mem_block *) ptr - 1;
    block->free = true;
    block->used = 0;
    //if entire page block is on is empty, delete page
}
/*calloc
* alloc, but sets memory to 0
*/
void *calloc(size_t nmemb, size_t size){
    void *addr = malloc(nmemb * size);
    memset(addr, 0, nmemb * size);
    return addr;
}
/*realloc
* input: ptr - pointer address of block, size - size of new memory to be allocated
* output: pointer of address block
*/
void *realloc(void *ptr, size_t size){
    if(ptr == NULL){
        return malloc(size);
    }

    if(size == 0){
        free(ptr);
    }

    LOG("Realloc block at %p to %zu bytes\n", ptr, size);
    struct mem_block *block = (struct mem_block *) ptr - 1;
    size_t total_req_sz = size + sizeof(struct mem_block);       

    if (total_req_sz <= block->used) {                           
        block->used = total_req_sz;                              
        return ptr;                                              
    }     

    if (total_req_sz <= block->size) {                           
        block->used = total_req_sz;                              
        return ptr;                                              
    }   

    void *new = malloc(size);
    if(new){
        memcpy(new, ptr, block->used);
        free(ptr);
    }
    return new;
}
/*print memory
* input: void
*output: iterates through block chain and calls printblock
*/
void print_memory(){
    struct mem_block *tmp = g_head;
    if(g_head->next == NULL){
        print_block(tmp);
    }
    else{
        while(tmp->next != NULL){
            
            print_block(tmp);
            tmp = tmp->next;
        }
    }
    print_block(tmp);
}

