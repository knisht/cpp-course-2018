#ifndef BIGINT_VECTOR_H
#define BIGINT_VECTOR_H

#include <cstddef>
#include <cstring>

class Vector
{
private:
    using elem_type = unsigned int;
    using size_type = size_t;

public:
    Vector();
    Vector(size_type new_size, elem_type elem = 0);
    Vector(const Vector &other) noexcept;
    Vector &operator=(Vector const &other) noexcept;
    Vector(Vector &&other) noexcept;
    Vector &operator=(Vector &&other) noexcept;
    ~Vector();

    void push_back(elem_type x);
    elem_type pop_back();

    inline const elem_type &back() const
    {
        return (length <= small_object_len) ? small_object[length - 1]
                                            : long_object.memory_location->data[length - 1];
    }

    inline elem_type &back()
    {
        if (length <= small_object_len)
            return small_object[length - 1];
        if (long_object.memory_location->ref_count > 1)
            change_location(long_object.capacity);
        return long_object.memory_location->data[length - 1];
    }

    inline const elem_type *begin() const
    {
        return (length <= small_object_len) ? small_object : long_object.memory_location->data;
    }

    inline elem_type *begin()
    {
        if (length <= small_object_len)
            return small_object;
        if (long_object.memory_location->ref_count > 1)
            change_location(long_object.capacity);
        return long_object.memory_location->data;
    }

    inline const elem_type *end() const
    {
        return ((length <= small_object_len) ? small_object
                                             : long_object.memory_location->data) +
               length;
    }

    inline elem_type *end()
    {
        if (length <= small_object_len)
            return small_object + length;
        if (long_object.memory_location->ref_count > 1)
            change_location(long_object.capacity);
        return long_object.memory_location->data + length;
    }

    void resize(size_type new_size, elem_type elem = 0);
    void reserve(size_type new_capacity);

    inline const elem_type &operator[](size_type index) const
    {
        return (length <= small_object_len) ? small_object[index]
                                            : long_object.memory_location->data[index];
    }
    inline elem_type &operator[](size_type index)
    {
        if (length <= small_object_len)
            return small_object[index];
        if (long_object.memory_location->ref_count == 1)
            return long_object.memory_location->data[index];
        change_location(long_object.capacity);
        return long_object.memory_location->data[index];
    }

    inline size_type size() const { return length; }

private:
    struct shared_array {
        size_type ref_count;
        elem_type *data;

        shared_array(size_type size);
        shared_array(const shared_array &other) = delete;
        shared_array &operator=(shared_array const &other) = delete;
        shared_array *make_copy(size_t size);
        static inline shared_array *connect(shared_array *current)
        {
            ++current->ref_count;
            return current;
        }
        static inline void disconnect(shared_array *current)
        {
            if (!current)
                return;
            --current->ref_count;
        }
        ~shared_array();
    };

    struct pool_ref {
        size_type capacity;
        shared_array *memory_location;
    };

    static constexpr size_t small_object_len =
        sizeof(pool_ref) / sizeof(elem_type);

    size_type length;
    union {
        pool_ref long_object;
        elem_type small_object[small_object_len];
    };

    static elem_type *allocate(size_type new_capacity);
    static void deallocate(elem_type *dataptr);
    void change_location(size_type new_size);
};

#endif // BIGINT_VECTOR_H
