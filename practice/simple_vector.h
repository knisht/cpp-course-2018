#ifndef SIMPLE_VECTOR
#define SIMPLE_VECTOR

#include <algorithm>
#include <cstddef>
#include <cstring>
#include <iostream>
#include <type_traits>
#include <utility>

template <typename T, bool Tag>
struct copyTag;

template <typename T>
struct copyTag<T, true> {
    static void copy(T *dst, T *src, size_t n)
    {
        memcpy(dst, src, n * sizeof(T));
    }
};

template <typename T>
struct copyTag<T, false> {
    static void copy(T *dst, T *src, size_t n)
    {
        for (size_t i = 0; i < n; ++i) {
            try {
                new (dst + i) T(src[i]);
            } catch (std::runtime_error e) {
                for (size_t j = 0; j <= i; ++j) {
                    dst[j].~T();
                }
                throw;
            }
        }
    }
};

template <typename T, bool Tag>
struct destructTag;

template <typename T>
struct destructTag<T, true> {
    static void destruct(T *start, T *end)
    {
    }
};

template <typename T>
struct destructTag<T, false> {
    static void destruct(T *start, T *end)
    {
        if (start && end)
            for (T *it = start; it != end; ++it) {
                it->~T();
            }
    }
};

template <typename T>
class simple_vector
{
public:
    simple_vector();
    simple_vector(size_t n, T const &elem = T());
    simple_vector(simple_vector const &other);
    simple_vector &operator=(simple_vector const &other);
    ~simple_vector();

    size_t size() const;
    void push_back(T const &t);
    void pop_back();
    T const &back() const;
    T &back();
    T &operator[](size_t index);
    T const &operator[](size_t index) const;
    T *data();
    T const *data() const;
    T const *begin() const;
    T const *end() const;
    T *begin();
    T *end();

    void reserve(size_t n);
    size_t capacity() const;
    void shrink_to_fit();

    void clear();

    T *insert(T *pos, T const &other);
    T *erase(T *pos);
    T *erase(T *first, T *last);

    void resize(size_t n, T value = T());
    void assign(size_t n, T value = T());

    template <typename Iter>
    void assign(Iter first, Iter last)
    {
        T *it = enter;
        destructTag<T, std::is_trivially_destructible<T>::value>::destruct(first, last);
        for (Iter it_other = first; it_other != last; ++it_other) {
            (*(it++)) = *it_other;
        }
    }

    template <typename Q>
    friend void swap(simple_vector<Q> &a, simple_vector<Q> &b);

private:
    T *enter;
    T *leave;
    T *storage_end;
};

template <typename T>
void swap(simple_vector<T> &a, simple_vector<T> &b)
{
    std::swap(a.enter, b.enter);
    std::swap(a.leave, b.leave);
    std::swap(a.storage_end, b.storage_end);
}

template <typename T>
simple_vector<T>::simple_vector()
    : enter(nullptr),
      leave(nullptr),
      storage_end(nullptr)
{
}

template <typename T>
simple_vector<T>::simple_vector(size_t n, T const &elem)
{
    enter = (T *)operator new(sizeof(T) * n);
    new (enter) T[n];
    leave = enter + n;
    storage_end = leave;
    *enter = elem;
    for (size_t i = 0; i < n; ++i) {
        enter[i] = elem;
    }
}

template <typename T>
simple_vector<T>::simple_vector(simple_vector<T> const &other)
{
    enter = (T *)operator new(sizeof(T) * other.size());

    try {
        copyTag<T, std::is_trivially_copy_assignable<T>::value>::copy(enter, other.enter, other.size());
    } catch (std::runtime_error e) {
        operator delete(enter);
        throw;
    }
    leave = enter + other.size();
    storage_end = leave;
}

template <typename T>
simple_vector<T> &simple_vector<T>::operator=(simple_vector<T> const &other)
{
    simple_vector tmp{other};
    swap(*this, tmp);
}

template <typename T>
simple_vector<T>::~simple_vector()
{
    destructTag<T, std::is_trivially_destructible<T>::value>::destruct(enter, leave);
    operator delete(enter);
}

template <typename T>
size_t simple_vector<T>::size() const
{
    return leave - enter;
}

template <typename T>
void simple_vector<T>::push_back(T const &t)
{
    if (leave == storage_end) {
        reserve(std::max(4u, size() * 2));
    }
    new (leave) T(t);
    ++leave;
}

template <typename T>
void simple_vector<T>::pop_back()
{
    T &last = back();
    last.~T();
    --leave;
}

template <typename T>
T &simple_vector<T>::back()
{
    return *(leave - 1);
}

template <typename T>
T const &simple_vector<T>::back() const
{
    return *(leave - 1);
}

template <typename T>
T const &simple_vector<T>::operator[](size_t n) const
{
    return enter[n];
}

template <typename T>
T &simple_vector<T>::operator[](size_t n)
{
    return enter[n];
}

template <typename T>
T *simple_vector<T>::data()
{
    return enter;
}

template <typename T>
T const *simple_vector<T>::data() const
{
    return enter;
}

template <typename T>
T const *simple_vector<T>::begin() const
{
    return enter;
}

template <typename T>
T const *simple_vector<T>::end() const
{
    return leave;
}

template <typename T>
T *simple_vector<T>::begin()
{
    return enter;
}

template <typename T>
T *simple_vector<T>::end()
{
    return leave;
}

template <typename T>
void simple_vector<T>::reserve(size_t n)
{
    simple_vector<T> tmp;
    tmp.enter = (T *)operator new(sizeof(T) * n);
    //new (tmp.enter) T[std::min(size(), n)];
    copyTag<T, std::is_trivially_copy_assignable<T>::value>::copy(tmp.enter, enter, std::min(size(), n));
    tmp.leave = tmp.enter + std::min(size(), n);
    tmp.storage_end = tmp.enter + n;
    swap(*this, tmp);
}

template <typename T>
size_t simple_vector<T>::capacity() const
{
    return storage_end - enter;
}

template <typename T>
void simple_vector<T>::shrink_to_fit()
{
    reserve(size());
}

template <typename T>
void simple_vector<T>::clear()
{
    simple_vector<T> tmp;
    swap(tmp, *this);
}

template <typename T>
T *simple_vector<T>::insert(T *pos, T const &other)
{
    T x = *pos;
    T y = other;
    for (T *it = pos; it != leave; ++it) {
        x = *it;
        it->~T();
        *it = y;
        y = x;
    }
    push_back(x);
    return pos;
}

template <typename T>
T *simple_vector<T>::erase(T *pos)
{
    for (T *it = pos; it != leave - 1; ++it) {
        *it = *(it + 1);
    }
    pop_back();
    return pos;
}

template <typename T>
T *simple_vector<T>::erase(T *first, T *last)
{
    size_t diff = last - first;
    for (T *it = first; it != leave - diff; ++it) {
        *it = *(it + diff);
    }
    for (size_t i = 0; i < diff; ++i)
        pop_back();
    return first;
}

template <typename T>
void simple_vector<T>::resize(size_t n, T value)
{
    if (n > capacity())
        reserve(n);
    if (n > size()) {
        T *tmp = enter;
        size_t oldsize = size();
        enter = leave;
        assign(n - oldsize, value);
        enter = tmp;
    }
    leave = enter + n;
}

template <typename T>
void simple_vector<T>::assign(size_t n, T value)
{
    for (size_t i = 0; i < n; ++i) {
        (enter + i)->~T();
        enter[i] = value;
    }
}
#endif
