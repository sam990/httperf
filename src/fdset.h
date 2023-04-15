#ifndef FDSET_H
#define FDSET_H
/*******************************************************************************
***
***     Author: Sameer Ahmad
***     email:  sameerahmad990@gmail.com
***
*******************************************************************************/

#ifdef __cplusplus
extern "C" {
#endif

#include <inttypes.h>       /* uint64_t */

/* https://gcc.gnu.org/onlinedocs/gcc/Alternate-Keywords.html#Alternate-Keywords */
#ifndef __GNUC__
#define __inline__ inline
#endif

#define DEF_FDSET_SIZE 1024 // The default fd set size

typedef struct {
    unsigned set_size;
    uint64_t *set_elems;

    // Iterator fields
    int current_elem;
} fdset;


#define MAJOR_IDX(e) ((e)/64)
#define MINOR_IDX(e) ((e) & 0x3f)
#define ELEM_MASK(e) (1UL << MINOR_IDX(e))

#define NUM_MAJOR_CNT(size) (((size) + 63) / 64)
#define FDSET_NUM_ELEMS(s) ((s)->num_elems)

/* 
    Initialize the fd set with default size.
    You need to free the memory by calling fdset_destroy

    Returns: 
        Pointer to the set: On Success
        NULL: On Error
*/
fdset* fdset_init();

/* 
    Clones a given fdset
    You need to free the memory by calling fdset_destroy

    Returns: 
        Pointer to the set: On Success
        NULL: On Error
*/
fdset* fdset_clone(fdset* set);


/* 
    Initialize the fd set with a given size.
    You need to free the memory by calling fdset_destroy

    Returns: 
        Pointer to the set: On Success
        NULL: On Error
*/
fdset* fdset_init_size(unsigned size);



/*
    Frees the memory used by the set.
*/
void fdset_destroy(fdset *set);

/*
    Makes the set empty
*/
void fdset_clear(fdset *set);


/*
    Adds an element to the set, if not already added.

    Returns:
        FDSET_SUCCESS: On success
        FDSET_MALLOC_ERROR: If an error occured setting up memory
        FDSET_ERROR: Any other error
*/
int fdset_add(fdset *set, int elem);


/*
    Adds an element to the set, if present.

    Returns:
        FDSET_SUCCESS: On success
        FDSET_NOT_PRESENT: If element is not present
        FDSET_ERROR: Any other error
*/
int fdset_remove(fdset *set, int elem);

/*
    Checks if a set contains a given alement

    Returns:
        FDSET_TRUE: Contains the elements
        FDSET_FALSE: Doesn;t containt the element
*/
int fdset_contains(fdset *set, int elem);


/*
    Returns a new set which is
    union of the given two sets

    Returns: 
        Pointer to the set: On Success
        NULL: On Error
*/
fdset* fdset_union(fdset *s1, fdset *s2);


/*
    Returns a new set which is
    intersection of the given two sets

    Returns: 
        Pointer to the set: On Success
        NULL: On Error
*/
fdset* fdset_intersection(fdset *s1, fdset *s2);

/*
    Returns a new set which is
    difference of the given two sets s1 - s2

    Returns: 
        Pointer to the set: On Success
        NULL: On Error
*/
fdset* fdset_difference(fdset *s1, fdset *s2);


/*
    Initialises iterator for the fdset

    Returns:
        First element of the set
*/
int fdset_iter_begin(fdset *set);


/*
    Checks if there are more elements to iterate

    Returns:
        FDSET_TRUE: If there are more elements
        FDSET_FALSE: If all elements have been iterated
*/
int fdset_iter_more(fdset *set);

/*
    Updates iterator to the next element

    Returns:
        Next element of the set
*/
int fdset_iter_next(fdset *set);


#define FDSET_SUCCESS 0
#define FDSET_ERROR -1
#define FDSET_MALLOC_ERROR -2
#define FDSET_NOT_PRESENT -3
#define FDSET_TRUE 1
#define FDSET_FALSE 0

#ifdef __cplusplus
} // extern "C"
#endif

#endif /* END SIMPLE SET HEADER */