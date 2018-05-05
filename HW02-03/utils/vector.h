#ifndef BIGINT_VECTOR_H
#define BIGINT_VECTOR_H

#include <cstddef>
#include <cstring>

class Vector
{
private:
    using value_type = unsigned int;
    using size_type = size_t;

public:
    Vector();
    Vector(size_type new_size, value_type elem = 0);
    Vector(const Vector &other) noexcept;
    Vector &operator=(Vector const &other) noexcept;
    Vector(Vector &&other) noexcept;
    Vector &operator=(Vector &&other) noexcept;
    ~Vector();

    void push_back(value_type x);
    value_type pop_back();

    inline const value_type &back() const
    {
        return true_ptr[length - 1];
    }

    inline value_type &back()
    {
        return true_ptr[length - 1];
    }

    inline const value_type *begin() const
    {
        return true_ptr;
    }

    inline value_type *begin()
    {
        return true_ptr;
    }

    inline const value_type *end() const
    {
        return true_ptr + length;
    }

    inline value_type *end()
    {
        return true_ptr + length;
    }

    void resize(size_type new_size, value_type elem = 0);
    void reserve(size_type new_capacity);

    inline const value_type &operator[](size_type index) const
    {
        return true_ptr[index];
    }

    inline value_type &operator[](size_type index)
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
        size_type ref_count;
        value_type data[];

        shared_array();
        shared_array(const shared_array &other) = delete;
        static shared_array *construct_shared_array(size_type size);
        static void destruct_shared_array(shared_array *target);
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
            if (current->ref_count == 0)
                shared_array::destruct_shared_array(current);
        }
        ~shared_array();
    };

    struct pool_ref {
        size_type capacity;
        shared_array *memory_location;
    };

    static constexpr size_type small_object_len =
        sizeof(pool_ref) / sizeof(value_type);

    size_type length;
    value_type *true_ptr;
    union {
        pool_ref long_object;
        value_type small_object[small_object_len];
    };

    void change_location(size_type new_size);
};

#endif // BIGINT_VECTOR_H
