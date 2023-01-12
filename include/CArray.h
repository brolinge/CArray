#ifndef CARRAY_H
#define CARRAY_H

#include <stdbool.h>
#include <stdlib.h>
#include <stdint.h>
#include <string.h>
#include <errno.h>

#define CARRAY_CAPACITY_DEFAULT 0x10U
#define CARRAY_SIZE_DEFAULT 0U
#define CARRAY_MSIZE_MIN 1U
#define CARRAY_MSIZE_MAX 0xFFFU

#ifndef ERANGE
#define ERANGE 34U
#endif // ERANGE

#ifndef ENOMEM
#define ENOMEM 12U
#endif // ENOMEM

#ifndef EEMPTY
#define EEMPTY 899U
#endif // EEMPTY

#ifndef ENOTFOUND
#define ENOTFOUND 999U
#endif // ENOTFOUND

#ifndef EMSIZE
#define EMSIZE 9191U
#endif // EMSIZE

#ifndef ESIZE
#define ESIZE 9292U
#endif // ESIZE

#ifndef EBOUNDS
#define EBOUNDS 9393U
#endif // ESIZE

#ifndef ESIZENE
#define ESIZENE 8481U
#endif // ESIZENE

typedef struct CArray* CArray;

uint64_t carray_sizeof(void);

//
// NOTE: bool return values indicate success/failure of function
//       on failure check errno for error code

extern int Free(void* mem);

//
// CRUD

// Create
//
// create does not overwrite existing elements
// shifts elements and resizes automatically
// makes a deep copy of in element, changes to in element do not affect
// array
// can return false on:
//  - index out of bounds (errno == EBOUNDS)
//  - resize fail (errno == ENOMEM) - array is not changed in that case
bool carray_create(CArray array, const void* in_ele, uint64_t at);

// Read
//
// writes a deep copy of the element to out_ele
// does not modify array in any way or expose underlying pointers
// can return false on:
//  - index out of bounds (errno == EBOUNDS)
//  - array is empty (errno == EEMPTY)
bool carray_read(const CArray array, void* out_ele, uint64_t at);

// Update
//
// deep copies over existing element, replacing with in ele
// in ele is not bound to the underlying array
// can return false on:
//  - index out of bounds (errno == EBOUNDS)
//  - array is empty (errno == EEMPTY)
bool carray_update(CArray array, const void* in_ele, uint64_t at);

// Delete
//
// doesn't free/resize any memory
// must use carray_resize to shrink/grow memory allocated for array
// can return false on:
//  - index out of bounds (errno == EBOUNDS)
//  - array is empty (errno == EEMPTY)
bool carray_delete(CArray array, uint64_t at);

// END: CRUD



//
// Utility

// Linear Search
//
//
bool carray_search(
    const CArray array,
    const void* search_ele,
    uint64_t* found_at,
    bool (*equals)(const void*, const void*)
);

// Equals
//
//
bool carray_equals(
    const CArray array1,
    const CArray array2,
    bool (*equals)(const void*, const void*)
);

// END: Utility



//
// CArray state information

// MSize
//
// size of individual element in bytes
uint64_t carray_msize(const CArray array);

// Size
//
// number of current elements in array
uint64_t carray_size(const CArray array);

// Capacity
//
// current number of total possible elements
//  - array is dynamic, capacity will automatically grow
//  - NOT total bytes of memory used
// total bytes: capacity * msize
uint64_t carray_capacity(const CArray array);

// END: CArray state information



//
// CArray object lifetime management

// New
//
// creates a new empty array with element members of msize length in bytes
// errno == ENOMEM if either malloc fails
// NOTE: msize limit is CARRAY_MSIZE_MAX, if over return NULL and errno == EMSIZE
CArray carray_new(uint64_t msize);

// Del (free)
//
// frees both underlying array and the array object itself
void carray_del(CArray* array);

// Resize
//
// grow or shrink the underlying array so it can hold new_capacity num
// of elements
// if new_capacity is less than current size the array is truncated
// NOTE: this is not a size in bytes of new array
// NOTE: will always be at least DEFAULT_CARRAY_CAPACITY
// return false on resize fail with no change to the CArray object
bool carray_resize(CArray array, uint64_t new_capacity);


// Copy
//
CArray carray_copy(const CArray array);

// END: CArray object lifetime management



#endif // CARRAY_H