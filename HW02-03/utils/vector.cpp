#include "vector.h"
#include <cstring>
#include <iostream>
#include <stdexcept>

using value_type = unsigned int;
using size_type = size_t;

// shared_array implementation
Vector::shared_array *Vector::shared_array::construct_shared_array(size_type size)
{
    void *p = operator new((size) * sizeof(value_type) + sizeof(size_type));
    shared_array *result = new (p) shared_array();
    return result;
}

Vector::shared_array::shared_array()
    : ref_count(0)
{
}

Vector::shared_array::~shared_array()
{
}

void Vector::shared_array::destruct_shared_array(shared_array *target)
{
    if (target->ref_count == 0) {
        operator delete(target);
    }
}

//vector implementation
Vector::Vector() : length(0), true_ptr(small_object) {}

Vector::Vector(size_type new_size, value_type elem) : length(new_size)
{
    if (new_size <= small_object_len) {
        for (size_type i = 0; i < small_object_len; i++) {
            small_object[i] = elem;
        }
        true_ptr = small_object;
    } else {
        long_object.capacity = new_size;
        shared_array *temporary = shared_array::construct_shared_array(new_size);
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
    if (length > small_object_len) {
        long_object.capacity = other.long_object.capacity;
        long_object.memory_location = other.long_object.memory_location;
        long_object.memory_location = shared_array::connect(other.long_object.memory_location);
        true_ptr = long_object.memory_location->data;
    } else {
        for (size_type i = 0; i < small_object_len; ++i) {
            small_object[i] = other.small_object[i];
        }
        true_ptr = small_object;
    }
}

Vector &Vector::operator=(Vector const &other) noexcept
{
    if (&other == this)
        return *this;
    if (length > small_object_len) {
        shared_array::disconnect(this->long_object.memory_location);
    }
    Vector tmp(other);
    length = tmp.length;
    if (length <= small_object_len) {
        for (size_type i = 0; i < small_object_len; ++i) {
            small_object[i] = other.small_object[i];
        }
        true_ptr = small_object;
    } else {
        true_ptr = tmp.true_ptr;
        long_object.memory_location = tmp.long_object.memory_location;
        long_object.capacity = tmp.long_object.capacity;
        tmp.long_object.memory_location = nullptr;
    }
    return *this;
}

Vector::Vector(Vector &&other) noexcept : length(other.length)
{
    true_ptr = length <= small_object_len ? small_object : other.long_object.memory_location->data;
    if (length > small_object_len) {
        long_object.capacity = other.long_object.capacity;
        long_object.memory_location = other.long_object.memory_location;
        other.long_object.memory_location = nullptr;
    } else {
        for (size_type i = 0; i < small_object_len; ++i) {
            small_object[i] = other.small_object[i];
        }
    }
}

Vector &Vector::operator=(Vector &&other) noexcept
{
    if (length > small_object_len) {
        shared_array::disconnect(this->long_object.memory_location);
    }
    length = other.length;
    if (length > small_object_len) {
        long_object.capacity = other.long_object.capacity;
        long_object.memory_location = other.long_object.memory_location;
        true_ptr = other.true_ptr;
        other.long_object.memory_location = nullptr;
    } else {
        for (size_type i = 0; i < small_object_len; ++i) {
            small_object[i] = other.small_object[i];
        }
        true_ptr = small_object;
    }
    return *this;
}

Vector::~Vector()
{
    if (length > small_object_len) {
        shared_array::disconnect(long_object.memory_location);
    }
}

void Vector::push_back(value_type x)
{
    if (length > small_object_len) {
        if (length == long_object.capacity || long_object.memory_location->ref_count != 1) {
            change_location(long_object.capacity * 2);
        }
        long_object.memory_location->data[length] = x;
    } else if (length < small_object_len) {
        small_object[length] = x;
    } else {
        change_location(small_object_len * 4);
        long_object.memory_location->data[length] = x;
    }
    ++length;
}

value_type Vector::pop_back()
{
    value_type x{};
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

void Vector::resize(size_type new_size, value_type elem)
{
    change_location(new_size);
    if (new_size > small_object_len && new_size > length) {
        for (value_type *ptr = long_object.memory_location->data + length;
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
                   small_object_len * sizeof(value_type));

            shared_array::disconnect(tmp);
        }
        true_ptr = small_object;
    } else {
        shared_array *new_data = shared_array::construct_shared_array(new_size);
        if (length <= small_object_len) {
            memcpy(new_data->data, small_object, small_object_len * sizeof(value_type));
        } else {
            memcpy(new_data->data, long_object.memory_location->data,
                   std::min(length, new_size) * sizeof(value_type));
            shared_array::disconnect(long_object.memory_location);
        }
        long_object.memory_location = shared_array::connect(new_data);
        long_object.capacity = new_size;
        true_ptr = long_object.memory_location->data;
    }
}
