/*******************************************************************************
***
***     Author: Sameer Ahmad
***     email:  sameerahmad990@gmail.com
***
*******************************************************************************/

#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <fdset.h>

// private functions start
int __fdset_resize(fdset* set, unsigned new_size);

void __fdset_iter_update(fdset *);
// private functions end

fdset* fdset_init_size(unsigned size) {
    fdset* s = malloc(sizeof(fdset));

    if (s == NULL) {
        return NULL;
    }

    // initialise values
    s->set_size = size;

    s->set_elems = malloc(NUM_MAJOR_CNT(s->set_size) * sizeof(uint64_t));

    if (s->set_elems == NULL) {
        free(s);
        return NULL;
    }

    fdset_clear(s);
    return s;
}

fdset* fdset_init() {
    return fdset_init_size(DEF_FDSET_SIZE);
}

fdset* fdset_clone(fdset* set) {
    fdset* new_set = fdset_init_size(set->set_size);

    if (new_set == NULL) {
        return NULL;
    }

    for (int i = 0; i < NUM_MAJOR_CNT(set->set_size); i++) {
        new_set->set_elems[i] = set->set_elems[i];
    }

    return new_set;
}

void fdset_clear(fdset *set) {
    set->current_elem = -1;

    for (int i = 0; i < NUM_MAJOR_CNT(set->set_size); i++) {
        set->set_elems[i] = 0;
    }
}

void fdset_destroy(fdset *set) {
    if (set != NULL) {
        if (set->set_elems != NULL) {
            free(set->set_elems);
        }
        free(set);
    }
}



int fdset_add(fdset *set, int elem) {
    // if elem is greater than the set size, then need to
    // resize fdset

    if (elem >= set->set_size) {
        int retcode = __fdset_resize(set, set->set_size * 2);
        if (retcode != FDSET_SUCCESS) {
            return retcode;
        }
    }

    // add the element
    unsigned majorIdx = MAJOR_IDX(elem);
    uint64_t mask = ELEM_MASK(elem);

    set->set_elems[majorIdx] |= mask;
    return FDSET_SUCCESS;
}

int fdset_remove(fdset *set, int elem) {
    if (elem >= set->set_size) {
        // out of range
        return FDSET_ERROR;
    }

    if (!fdset_contains(set, elem)) {
        return FDSET_NOT_PRESENT;
    }

    unsigned majorIdx = MAJOR_IDX(elem);
    uint64_t mask = ELEM_MASK(elem);

    set->set_elems[majorIdx] &= (~mask);

    return FDSET_SUCCESS;
}




int __fdset_resize(fdset* set, unsigned new_size) {

    if (new_size <= set->set_size) {
        return FDSET_ERROR; // can't shrink set
    }

    // save old size
    unsigned old_size = set->set_size;

    // reallocate memory
    uint64_t* new_block = realloc(set->set_elems, NUM_MAJOR_CNT(new_size) * sizeof(uint64_t));

    if (new_block == NULL) {
        return FDSET_MALLOC_ERROR;
    }

    // set new indices to 0

    for (int i = NUM_MAJOR_CNT(old_size); i < NUM_MAJOR_CNT(new_size); i++) {
        new_block[i] = 0;
    }

    set->set_elems = new_block;
    set->set_size = new_size;

    return FDSET_SUCCESS;
}


int fdset_contains(fdset *set, int elem) {
    if (elem >= set->set_size) {
        return FDSET_FALSE;
    }

    unsigned majorIdx = MAJOR_IDX(elem);
    uint64_t mask = ELEM_MASK(elem);

    return (set->set_elems[majorIdx] & mask) ? FDSET_TRUE : FDSET_FALSE;
}

fdset* fdset_union(fdset *s1, fdset *s2) {
    // MAX of the two
    unsigned new_size = s1->set_size > s2->set_size ? s1->set_size : s2->set_size;

    // Iterate upto the min of two
    unsigned iter_len = s1->set_size < s2->set_size ? s1->set_size : s2->set_size;

    fdset* new_set = fdset_init_size(new_size);
    
    if (new_set == NULL) {
        return NULL;
    }

    for (int i = 0; i < NUM_MAJOR_CNT(iter_len); i++) {
        new_set->set_elems[i] = s1->set_elems[i] | s2->set_elems[i];
    }

    // set rest same as with the larger item
    fdset* larger_set = s1->set_size > s2->set_size ? s1 : s2;

    for (int i = NUM_MAJOR_CNT(iter_len); i < NUM_MAJOR_CNT(larger_set->set_size); i++) {
        new_set->set_elems[i] = larger_set->set_elems[i];
    }


    return new_set;
}

fdset* fdset_intersection(fdset *s1, fdset *s2) {
    // Min of the two
    unsigned new_size = s1->set_size < s2->set_size ? s1->set_size : s2->set_size;

    // Iterate upto the min of two
    unsigned iter_len = new_size;

    fdset* new_set = fdset_init_size(new_size);
    
    if (new_set == NULL) {
        return NULL;
    }

    for (int i = 0; i < NUM_MAJOR_CNT(iter_len); i++) {
        new_set->set_elems[i] = s1->set_elems[i] & s2->set_elems[i];
    }
    return new_set;
}

fdset* fdset_difference(fdset *s1, fdset *s2) {

    fdset* new_set = fdset_init_size(s1->set_size);
    
    if (new_set == NULL) {
        return NULL;
    }

     // Iterate upto the min of two
    unsigned iter_len = s1->set_size < s2->set_size ? s1->set_size : s2->set_size;

    for (int i = 0; i < NUM_MAJOR_CNT(iter_len); i++) {
        new_set->set_elems[i] = s1->set_elems[i] & (~s2->set_elems[i]);
    }

    for (int i = NUM_MAJOR_CNT(iter_len); i < NUM_MAJOR_CNT(s1->set_size); i++) {
        new_set->set_elems[i] = s1->set_elems[i];
    }

    return new_set;
}


void __fdset_iter_update(fdset *set) {

    for (int k = set->current_elem + 1; k < set->set_size; k++) {
        if (fdset_contains(set, k)) {
            set->current_elem = k;
            return;
        }
    }
    // otherwise set current_elem to -1
    set->current_elem = -1;
}

int fdset_iter_begin(fdset *set) {
    set->current_elem = -1;
    // update iterator
    __fdset_iter_update(set);

    return set->current_elem;
}

int fdset_iter_more(fdset *set) {
    return (set->current_elem == -1) ? FDSET_FALSE : FDSET_TRUE;
}

int fdset_iter_next(fdset *set) {
    __fdset_iter_update(set);
    return set->current_elem;
}