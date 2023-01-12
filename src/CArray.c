#include "CArray.h"

struct CArray
{
    uint64_t msize;
    uint64_t size;
    uint64_t capacity;
    void* elements;
};

uint64_t carray_sizeof(void)
{
    return sizeof(struct CArray);
}

bool carray_create(CArray array, const void* in_ele, uint64_t at)
{
    if (at > array->size)
    {
        errno = EBOUNDS;

        return false;
    }

    // resize if needed
    if (array->size == array->capacity)
    {
        void* new_elements = calloc(array->capacity * 2U, array->msize);
        if (new_elements == NULL)
        {
            errno = ENOMEM;

            return false;
        }

        memcpy(new_elements, (char*)array->elements, array->msize * array->size);

        free(array->elements);

        array->elements = new_elements;

        array->capacity *= 2U;
    }

    // shift elements right to make room
    for (uint64_t i = array->size; i > at; --i)
    {
        memmove(
            (char*)array->elements + (i * array->msize),
            (char*)array->elements + ((i - 1U) * array->msize),
            array->msize
        );
    }

    // copy in ele
    memcpy(
        (char*)array->elements + (at * array->msize),
        in_ele,
        array->msize
    );

    ++array->size;

    return true;
}

bool carray_read(const CArray array, void* out_ele, uint64_t at)
{
    if (array->size == 0U)
    {
        errno = EEMPTY;
        return false;
    }

    if (at >= array->size)
    {
        errno = EBOUNDS;

        return false;
    }

    memcpy(
        (char*)out_ele,
        (char*)array->elements + (at * array->msize),
        array->msize
    );

    return true;
}

bool carray_update(CArray array, const void* in_ele, uint64_t at)
{
    if (array->size == 0U)
    {
        errno = EEMPTY;
        return false;
    }

    if (at >= array->size)
    {
        errno = EBOUNDS;

        return false;
    }

    memcpy(
        (char*)array->elements + (at * array->msize),
        (char*)in_ele,
        array->msize
    );

    return true;
}

bool carray_delete(CArray array, uint64_t at)
{
    if (array->size == 0U)
    {
        errno = EEMPTY;
        return false;
    }

    if (at >= array->size)
    {
        errno = EBOUNDS;

        return false;
    }

    --array->size;

    // shift elements left to fill gap
    for (uint64_t i = at; i < array->size; ++i)
    {
        memmove(
            (char*)array->elements + (i * array->msize),
            (char*)array->elements + ((i + 1U) * array->msize),
            array->msize
        );
    }

    return true;
}

bool carray_search(
    const CArray array,
    const void* search_ele,
    uint64_t* found_at,
    bool (*equals)(const void*, const void*)
    )
{
    if (array->size == 0U)
    {
        errno = EEMPTY;

        return false;
    }

    for (uint64_t i = 0; i < array->size; ++i)
    {
        if (equals(
            (char*)array->elements + (i * array->msize),
            search_ele
        ) == true)
        {
            *found_at = i;

            return true;
        }
    }

    errno = ENOTFOUND;

    return false;
}

bool carray_equals(
    const CArray array1,
    const CArray array2,
    bool(*equals)(const void*, const void*)
    )
{
    if (array1->msize != array2->msize)
    {
        errno = EMSIZE;

        return false;
    }

    if (array1->size == 0U || array2->size == 0U)
    {
        errno = EEMPTY;

        return false;
    }

    if (array1->size != array2->size)
    {
        errno = ESIZENE;



        return false;
    }

    for (uint64_t i = 0; i < array1->size; ++i)
    {
        if (equals(
            (char*)array1->elements + (i * array1->msize),
            (char*)array2->elements + (i * array2->msize)
        ) == false)
        {
            return true;
        }
    }

    return true;
}

uint64_t carray_msize(const CArray array)
{
    return array->msize;
}

uint64_t carray_size(const CArray array)
{
    return array->size;
}

uint64_t carray_capacity(const CArray array)
{
    return array->capacity;
}

CArray carray_new(uint64_t msize)
{
    if (msize > CARRAY_MSIZE_MAX || msize < CARRAY_MSIZE_MIN)
    {
        errno = EMSIZE;

        return NULL;
    }

    CArray array = malloc(sizeof(struct CArray));
    if (array == NULL)
    {
        errno = ENOMEM;

        return NULL;
    }

    void* elements = calloc(CARRAY_CAPACITY_DEFAULT, msize);
    if (elements == NULL)
    {
        free(array);

        errno = ENOMEM;

        return NULL;
    }

    array->msize = msize;
    array->size = CARRAY_SIZE_DEFAULT;
    array->capacity = CARRAY_CAPACITY_DEFAULT;
    array->elements = elements;

    return array;
}

void carray_del(CArray* array)
{
    if (array == NULL)
    {
        return;
    }

    if (*array == NULL)
    {
        return;
    }

    free((*array)->elements);

    free(*array);

    *array = NULL;
}

bool carray_resize(CArray array, uint64_t new_capacity)
{
    uint64_t resized_capacity = new_capacity < CARRAY_CAPACITY_DEFAULT ? CARRAY_CAPACITY_DEFAULT : new_capacity;
    uint64_t resized_size = new_capacity < array->size ? new_capacity : array->size;

    void* resized_elements = calloc(resized_capacity, array->msize);
    if (resized_elements == NULL)
    {
        errno = ENOMEM;

        return false;
    }

    if (resized_size > 0U)
    {
        memcpy(
            (char*)resized_elements,
            (char*)array->elements,
            resized_size * array->msize
        );
    }

    free(array->elements);

    array->elements = resized_elements;

    array->size = resized_size;
    array->capacity = resized_capacity;

    return true;
}

CArray carray_copy(const CArray array)
{
    CArray array_cpy = carray_new(array->msize);
    if (array_cpy == NULL)
    {
        return NULL;
    }

    if (array->size == 0U)
    {
        return array_cpy;
    }

    char* read_buf = calloc(1U, array->msize);
    if (read_buf == NULL)
    {
        carray_del(&array_cpy);

        errno = ENOMEM;

        return NULL;
    }

    for (uint64_t i = 0; i < array->size; ++i)
    {
        if (carray_read(array, read_buf, i) == false)
        {
            free(read_buf);
            carray_del(&array_cpy);
            return NULL;
        }

        if (carray_create(array_cpy, read_buf, i) == false)
        {
            free(read_buf);
            carray_del(&array_cpy);
            return NULL;
        }
    }

    free(read_buf);

    return array_cpy;
}