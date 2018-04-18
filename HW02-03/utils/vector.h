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
        return true_ptr[length - 1];
    }

    inline elem_type &back()
    {
        return true_ptr[length - 1];
    }

    inline const elem_type *begin() const
    {
        return true_ptr;
    }

    inline elem_type *begin()
    {
        return true_ptr;
    }

    inline const elem_type *end() const
    {
        return true_ptr + length;
    }

    inline elem_type *end()
    {
        return true_ptr + length;
    }

    void resize(size_type new_size, elem_type elem = 0);
    void reserve(size_type new_capacity);

    inline const elem_type &operator[](size_type index) const
    {
        return true_ptr[index];
    }

    inline elem_type &operator[](size_type index)
    {
        return true_ptr[index];
    }

    inline size_type size() const { return length; }

    inline bool shared()
    {
        if (length <= small_object_len)
            return false;
        if (long_object.memory_location->ref_count == 1)
            return false;
        else
            return true;
    }

    void make_unique()
    {
        if (shared()) {
            resize(length);
        }
    }

private:
    struct shared_array {
        elem_type *data;
        size_type &ref_count;

        shared_array(size_type size);
        shared_array(const shared_array &other) = delete;
        shared_array &operator=(shared_array const &other) = delete;
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

    static constexpr size_type small_object_len =
        sizeof(pool_ref) / sizeof(elem_type);

    size_type length;
    elem_type *true_ptr;
    union {
        pool_ref long_object;
        elem_type small_object[small_object_len];
    };

    void change_location(size_type new_size);
    static void become_free(shared_array *target);
};

#endif // BIGINT_VECTOR_H
