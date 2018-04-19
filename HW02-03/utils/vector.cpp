#include "vector.h"
#include <cstring>
#include <iostream>
#include <stdexcept>

using elem_type = unsigned int;
using size_type = size_t;

// shared_array implementation
Vector::shared_array::shared_array(size_type size)
    : data((elem_type *)(malloc(size * sizeof(elem_type) + sizeof(size_type)))), ref_count(*data)
{
    ref_count = 0;
    for (size_t i = 0; i < sizeof(size_type)/sizeof(elem_type); i++)
        ++data;
}

Vector::shared_array::~shared_array()
{
    if (ref_count == 0) {
        for (size_t i = 0; i < sizeof(size_type) / sizeof(elem_type); i++)
            --data;
        free(data);
    }
}

//vector implementation
Vector::Vector() : length(0), true_ptr(small_object) {}

Vector::Vector(size_type new_size, elem_type elem) : length(new_size)
{
    if (new_size <= small_object_len) {
        for (size_type i = 0; i < small_object_len; i++) {
            small_object[i] = elem;
        }
        true_ptr = small_object;
    } else {
        long_object.capacity = new_size;
        shared_array *temporary = new shared_array(new_size);
        long_object.memory_location = shared_array::connect(temporary);
        for (size_type i = 0; i < new_size; i++) {
            long_object.memory_location->data[i] = elem;
        }
        true_ptr = long_object.memory_location->data;
    }
}

Vector::Vector(Vector const &other) noexcept
    : length(other.length)
{
    long_object.capacity = other.long_object.capacity;
    long_object.memory_location = other.long_object.memory_location;
    if (length > small_object_len) {
        long_object.memory_location = shared_array::connect(other.long_object.memory_location);
        true_ptr = long_object.memory_location->data;
    } else {
        true_ptr = small_object;
    }
}

Vector &Vector::operator=(Vector const &other) noexcept
{
    if (&other == this)
        return *this;
    if (length > small_object_len) {
        become_free(this->long_object.memory_location);
    }
    Vector tmp(other);
    length = tmp.length;
    true_ptr = tmp.true_ptr;
    if (length <= small_object_len)
        true_ptr = small_object;
    long_object.capacity = tmp.long_object.capacity;
    long_object.memory_location = tmp.long_object.memory_location;
    tmp.long_object.memory_location = nullptr;
    return *this;
}

Vector::Vector(Vector &&src) noexcept : length(src.length)
{
    true_ptr = length <= small_object_len ? small_object : src.long_object.memory_location->data;
    long_object.capacity = src.long_object.capacity;
    long_object.memory_location = src.long_object.memory_location;
    src.long_object.memory_location = nullptr;
}

Vector &Vector::operator=(Vector &&other) noexcept
{
    if (length > small_object_len) {
        become_free(this->long_object.memory_location);
    }
    length = other.length;
    long_object.memory_location = other.long_object.memory_location;
    true_ptr = length <= small_object_len ? small_object : other.true_ptr;
    if (other.length > small_object_len)
        other.long_object.memory_location = nullptr;
    long_object.capacity = other.long_object.capacity;
    return *this;
}

Vector::~Vector()
{
    if (length > small_object_len) {
        become_free(long_object.memory_location);
    }
}

void Vector::push_back(elem_type x)
{
    if (length > small_object_len) {
        if (length == long_object.capacity || long_object.memory_location->ref_count != 1) {
            change_location(long_object.capacity * 2);
        }
        long_object.memory_location->data[length] = x;
    } else {
        if (length < small_object_len) {
            small_object[length] = x;
        } else {
            change_location(small_object_len * 4);
            long_object.memory_location->data[length] = x;
        }
    }
    ++length;
}

elem_type Vector::pop_back()
{
    elem_type x{};
    if (length > small_object_len) {
        x = long_object.memory_location->data[length - 1];
    } else {
        x = small_object[length - 1];
    }
    if (length == small_object_len + 1) {
        change_location(small_object_len);
    }
    --length;
    return x;
}

void Vector::resize(size_type new_size, elem_type elem)
{
    change_location(new_size);
    if (new_size > small_object_len && new_size > length) {
        for (elem_type *ptr = long_object.memory_location->data + length;
             ptr != long_object.memory_location->data + new_size; ++ptr) {
            *ptr = elem;
        }
    } else {
        for (size_type i = new_size; i < small_object_len; i++) {
            small_object[i] = elem;
        }
    }
    length = new_size;
}

void Vector::reserve(size_type new_capacity)
{
    change_location(new_capacity);
}

void Vector::change_location(size_type new_size)
{
    if (new_size <= small_object_len) {
        if (length > small_object_len) {
            shared_array *tmp = long_object.memory_location;
            memcpy(small_object, long_object.memory_location->data,
                   small_object_len * sizeof(elem_type));

            become_free(tmp);
        }
        true_ptr = small_object;
    } else {
        shared_array *new_data = new shared_array(new_size);
        if (length <= small_object_len) {
            memcpy(new_data->data, small_object, small_object_len * sizeof(elem_type));
        } else {
            memcpy(new_data->data, long_object.memory_location->data,
                   std::min(length, new_size) * sizeof(elem_type));
            become_free(long_object.memory_location);
        }
        long_object.memory_location = shared_array::connect(new_data);
        long_object.capacity = new_size;
        true_ptr = long_object.memory_location->data;
    }
}

void Vector::become_free(Vector::shared_array *target)
{
    shared_array::disconnect(target);
    if (target && target->ref_count == 0)
        delete (target);
}
