#ifndef BIGINT_VECTOR_H
#define BIGINT_VECTOR_H

#include <cstddef>

class Vector
{
private:
    using elem_type = unsigned int;
    using size_type = size_t;
    struct pool_ref {
        size_type capacity;
        elem_type *data;
    };
    static constexpr size_t small_object_len =
        sizeof(pool_ref) / sizeof(elem_type);

public:
    Vector();
    Vector(size_type new_size, elem_type elem = 0);
    Vector(const Vector &other);
    Vector &operator=(Vector const &other);
    Vector(Vector &&other);
    Vector &operator=(Vector &&other);
    ~Vector();

    void push_back(elem_type x);
    elem_type pop_back();

    inline const elem_type &back() const
    {
        return (length <= small_object_len)
                   ? small_object[length - 1]
                   : long_object.data[length - 1];
    }
    inline elem_type &back()
    {
        return (length <= small_object_len)
                   ? small_object[length - 1]
                   : long_object.data[length - 1];
    }
    inline const elem_type *begin() const
    {
        return (length <= small_object_len) ? small_object
                                            : long_object.data;
    }
    inline elem_type *begin()
    {
        return (length <= small_object_len) ? small_object
                                            : long_object.data;
    }
    inline const elem_type *end() const
    {
        return ((length <= small_object_len) ? small_object
                                             : long_object.data) +
               length;
    }
    inline elem_type *end()
    {
        return ((length <= small_object_len) ? small_object
                                             : long_object.data) +
               length;
    }

    void resize(size_type new_size, elem_type elem = 0);
    void reserve(size_type new_capacity);

    inline const elem_type &operator[](size_type index) const
    {
        return (length <= small_object_len) ? small_object[index]
                                            : long_object.data[index];
    }
    inline elem_type &operator[](size_type index)
    {
        return (length <= small_object_len) ? small_object[index]
                                            : long_object.data[index];
    }

    inline size_type size() const { return length; }

private:
    size_type length;
    union {
        pool_ref long_object;
        elem_type small_object[small_object_len];
    };

    elem_type *allocate(size_type new_capacity);
    void deallocate(elem_type *dataptr);
    void change_location(size_type new_size);
};

#endif // BIGINT_VECTOR_H
