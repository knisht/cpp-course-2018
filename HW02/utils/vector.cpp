#include "vector.h"
#include <cstring>
#include <iostream>
#include <stdexcept>

using elem_type = unsigned int;
using size_type = size_t;

Vector::Vector() : length(0) {}

Vector::Vector(size_type new_size, elem_type elem) : length(new_size)
{
    if (new_size <= small_object_len) {
        for (size_t i = 0; i < small_object_len; i++) {
            small_object[i] = elem;
        }
    } else {
        long_object.capacity = new_size;
        long_object.data = allocate(new_size);
        for (size_type i = 0; i < new_size; i++) {
            long_object.data[i] = elem;
        }
    }
}

Vector::Vector(Vector const &other)
{
    length = other.length;
    long_object.capacity = other.long_object.capacity;
    long_object.data = other.long_object.data;
    if (length > small_object_len) {
        long_object.data = allocate(long_object.capacity);
        memcpy(long_object.data, other.long_object.data,
               length * sizeof(elem_type));
    }
}

Vector &Vector::operator=(Vector const &other)
{
    if (&other == this)
        return *this;
    Vector tmp(other);
    if (length > small_object_len) {
        deallocate(long_object.data);
    }
    length = tmp.length;
    long_object.capacity = tmp.long_object.capacity;
    long_object.data = tmp.long_object.data;
    tmp.long_object.data = nullptr;
    return *this;
}

Vector::Vector(Vector &&src) : length(src.length)
{
    long_object.capacity = src.long_object.capacity;
    long_object.data = std::move(src.long_object.data);
    src.long_object.data = nullptr;
}

Vector &Vector::operator=(Vector &&other)
{
    if (length > small_object_len)
        deallocate(long_object.data);
    if (other.length > small_object_len) {
        long_object.data = other.long_object.data;
        other.long_object.data = nullptr;
    } else {
        long_object.data = other.long_object.data;
    }
    long_object.capacity = other.long_object.capacity;
    length = other.length;
    return *this;
}

Vector::~Vector()
{
    if (length > small_object_len)
        deallocate(long_object.data);
}

void Vector::push_back(elem_type x)
{
    if (length > small_object_len) {
        if (length == long_object.capacity) {
            change_location(long_object.capacity +
                            long_object.capacity / 2);
        }
        long_object.data[length] = x;
    } else {
        if (length < small_object_len) {
            small_object[length] = x;
        } else {
            change_location(small_object_len * 4);
            long_object.data[length] = x;
        }
    }
    ++length;
}

elem_type Vector::pop_back()
{
    elem_type x{};
    if (length > 2) {
        x = long_object.data[length - 1];
    }
    if (length <= 2) {
        x = small_object[length - 1];
    }
    if (length == 3) {
        change_location(2);
    }
    --length;
    return x;
}

void Vector::resize(size_type new_size, elem_type elem)
{
    if (new_size > 2) {
        if ((length <= 2) ||
            ((length > 2) && (new_size > long_object.capacity))) {
            change_location(new_size);
        }
        for (elem_type *ptr = long_object.data + length;
             ptr != long_object.data + new_size; ++ptr) {
            *ptr = elem;
        }
    } else {
        if (length > 2)
            change_location(new_size);
        if (length < 2)
            small_object[1] = elem;
        if (length < 1)
            small_object[0] = elem;
    }
    length = new_size;
}

void Vector::reserve(size_type new_capacity)
{
    if (long_object.capacity > new_capacity)
        return;
    change_location(new_capacity);
}

elem_type *Vector::allocate(size_type new_capacity)
{
    return static_cast<elem_type *>(
        malloc(new_capacity * sizeof(elem_type)));
}

void Vector::deallocate(elem_type *dataptr) { free(dataptr); }

void Vector::change_location(size_type new_size)
{
    if (new_size <= 2) {
        if (length > 2) {
            elem_type *tmp = long_object.data;
            memmove(small_object, tmp,
                    small_object_len * sizeof(elem_type));
            deallocate(tmp);
        }
        return;
    }

    elem_type *new_data = allocate(new_size);
    if (length <= 2) {
        memmove(new_data, small_object,
                small_object_len * sizeof(elem_type));
    } else {
        memmove(new_data, long_object.data,
                std::min(length, new_size) * sizeof(elem_type));
        deallocate(long_object.data);
    }
    long_object.data = new_data;
    long_object.capacity = new_size;
}
