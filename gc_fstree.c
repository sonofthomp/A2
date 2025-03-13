#include <stdio.h>
#include "fstree.h"
#include "gc.h"

/* Traverse the fstree data-structure and call mark_one(void *vptr) on
 * each piece of memory that is still used in the fstree
 *
 * mark_one returns 0 if the chunk was marked for the first time and 1 if
 * it has been seen already. We need this to avoid revisiting pieces of the
 * tree that we have already marked -- where hard links have created cycles
 */

void mark_fstree(void *head) {
    if (head == NULL || mark_one(head) == 1) return;

    struct subdir *cur_subdir = ((struct fsnode*) head)->links;

    while (cur_subdir != NULL) {
        mark_fstree(cur_subdir->fptr);
        cur_subdir = cur_subdir->next;
    }
}
