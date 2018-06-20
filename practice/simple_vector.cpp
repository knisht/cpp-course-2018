#include "simple_vector.h"
#include <cstring>
#include <utility>

template <typename T>
simple_vector<T>::simple_vector()
    : enter(nullptr),
      leave(nullptr),
      storage_end(nullptr)
{
}

template <typename T>
simple_vector<T>::simple_vector(size_t n, T elem)
{
    enter = (T *)operator new(sizeof(T) * n);
    leave = enter + n;
    storage_end = leave;
    for (size_t i = 0; i < n; ++i) {
        enter[i](elem);
    }
}

template <typename T>
simple_vector<T>::simple_vector(simple_vector<T> const &other)
{
    enter = (T *)operator new(sizeof(T) * other.size());
    for (size_t i = 0; i != other.size(); ++i) {
        enter[i](other.enter[i]);
    }
    leave = enter + other.size();
    storage_end = leave;
}

template <typename T>
simple_vector<T> &simple_vector<T>::operator=(simple_vector<T> const &other)
{
    simple_vector tmp{other};
    std::swap(tmp.enter, enter);
    std::swap(tmp.leave, leave);
    std::swap(tmp.storage_end, storage_end);
}

template <typename T>
simple_vector<T>::~simple_vector()
{
    for (T *it = enter; it != leave; ++it) {
        it->~T();
    }
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
        reserve(size() * 2);
    }
    (*leave)(t);
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
    return *(--leave);
}

template <typename T>
T const &simple_vector<T>::back() const
{
    return *(--leave);
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
void simple_vector<T>::reserve(size_t n)
{
    simple_vector<T> tmp;
    tmp.enter = (T *)operator new()
}
