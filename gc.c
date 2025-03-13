#include <stdio.h>
#include <stdlib.h>
#include "gc.h"

// global variable to be head of allocated pieces
struct mem_chunk *memory_list_head = NULL;

// global variable for debugging
int debug = 0;

void print_memory_list() {
    struct mem_chunk *current = memory_list_head;
    while (current != NULL) {
        printf("%lx (%d) -> ",(unsigned long) current->address, current->in_use);
        current = current->next;
    }
    printf("\n");
}

void *gc_malloc(int nbytes) {
    void *ptr = malloc(nbytes);
    struct mem_chunk *new_node = malloc(sizeof(struct mem_chunk));
    new_node->in_use = 1;
    new_node->address = ptr;

    if (memory_list_head == NULL) {
        new_node->next = NULL;
        memory_list_head = new_node;
    } else {
        struct mem_chunk *cur_node = memory_list_head;
        while (cur_node->next != NULL) {
            cur_node = cur_node->next;
        }
        cur_node->next = new_node;
    }
    return ptr;
}

/* Executes the garbage collector.
 * obj is the root of the data structure to be traversed
 * mark_obj is a function that will traverse the data structure rooted at obj
 *
 * The function will also write to the LOGFILE the results messages with the
 * following format strings:
 * "Mark_and_sweep running\n"
 * "Chunks freed this pass: %d\n"
 * "Chunks still allocated: %d\n"
 */

void mark_and_sweep(void *obj, void (*mark_obj)(void *)) {
    printf("\nRunning mark and sweep...\n");

    printf("At the start, the memory list is:\n");
    print_memory_list();
    // Part one
    struct mem_chunk *cur_node = memory_list_head;
    
    while (cur_node != NULL) {
        cur_node->in_use = 0;
        cur_node = cur_node->next;
    }

    printf("After part 1, the memory list is:\n");
    print_memory_list();

    printf("Starting with node %p\n", obj);

    // Part two
    mark_obj(obj);

    printf("After part 2, the memory list is:\n");
    print_memory_list();

    // Part three
    cur_node = memory_list_head;
    while (cur_node->next != NULL) {
        // Free the trailing zeros
        if (cur_node->in_use == 0) {
            struct mem_chunk *old_cur_node = cur_node;
            cur_node = cur_node->next;
            memory_list_head = cur_node;
            free(old_cur_node);
            continue;
        }
        // From the first 1 and onwards, skip each 0
        // Keep it so that cur_node is always on a 1
        struct mem_chunk *next_node = cur_node->next;
        if (next_node->in_use == 0) {
            cur_node->next = next_node->next;
            free(next_node);
        } else {
            cur_node = cur_node->next;
        }
    }
    // Handles the case where the linked list was all 0's
    // Once we hit a 1, we stay on 1's exclusively. So if this condition is true, it means we never hit a 1
    if (cur_node->in_use == 0) {
        memory_list_head = NULL;
    }

    printf("After part 3, the memory list is:\n");
    print_memory_list();

    printf("Finished running mark and sweep\n\n");
}

/*
 Mark the chunk of memory pointed to by vptr as in use.
 Return codes:
   0 if successful
   1 if chunk already marked as in use
   2 if chunk is not found in memory list

   Here is a print statement to print an error message:
   fprintf(stderr, "ERROR: mark_one address not in memory list\n");
 */
int mark_one(void *vptr) {
    struct mem_chunk *cur_node = memory_list_head;

    while (cur_node != NULL) {
        if (cur_node->address == vptr) {
            if (cur_node->in_use == 0) {
                cur_node->in_use = 1;
                return 0;
            } else {
                return 1;
            }
        }
        cur_node = cur_node->next;
    }

    fprintf(stderr, "ERROR: mark_one address not in memory list\n");
    return 2;
}
