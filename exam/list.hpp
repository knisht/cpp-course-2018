#ifndef EXAM_LIST_HPP
#define EXAM_LIST_HPP
#include <algorithm>
#include <cassert>
#include <cstddef>

namespace exam
{
template <typename T>
class List
{

    struct Node {
        Node *next;
        Node *prev;
        Node()
        {
            next = this;
            prev = this;
        }
        virtual T *get_data()
        {
            assert(false);
            return nullptr;
        }
        virtual ~Node() {}
    };

    struct DataNode : Node {
        T value;
        T *get_data()
        {
            return &value;
        }
        DataNode(T const &t) : value(t) {}
        ~DataNode() {}
    };

    Node center;
    size_t length_;

public:
    typedef T value_type;
    typedef T *pointer;
    typedef T &reference;

    List() noexcept;
    List(List const &);
    List &operator=(List const &);
    ~List() noexcept;

    template <typename Q>
    struct generic_iterator {
        typedef std::ptrdiff_t difference_type;
        typedef Q value_type;
        typedef Q *pointer;
        typedef Q &reference;
        typedef std::bidirectional_iterator_tag iterator_category;

        Node *node;
        friend class List;

    private:
        generic_iterator(Node *nd) : node(nd)
        {
        }
        generic_iterator(Node const *nd) : node(const_cast<Node *>(nd))
        {
        }

    public:
        operator generic_iterator<const Q>()
        {
            return generic_iterator<const Q>(this->node);
        }

        generic_iterator operator++()
        {
            node = node->next;
            return *this;
        }

        generic_iterator operator++(int)
        {
            generic_iterator tmp(*this);
            operator++();
            return tmp;
        }

        generic_iterator operator--()
        {
            node = node->prev;
            return *this;
        }

        generic_iterator operator--(int)
        {
            generic_iterator tmp(*this);
            operator--();
            return tmp;
        }

        bool operator==(generic_iterator other)
        {
            return (node == other.node);
        }

        bool operator!=(generic_iterator other)
        {
            return !operator==(other);
        }

        reference operator*()
        {
            return *node->get_data();
        }
    };

    typedef generic_iterator<T> iterator;
    typedef generic_iterator<T const> const_iterator;
    typedef std::reverse_iterator<iterator> reverse_iterator;
    typedef std::reverse_iterator<const_iterator> const_reverse_iterator;

    void push_front(T const &);
    void pop_front() noexcept;
    void push_back(T const &);
    void pop_back() noexcept;

    iterator begin() noexcept;
    iterator end() noexcept;
    T &front();
    T const &front() const;
    T &back();
    T const &back() const;
    const_iterator begin() const noexcept;
    const_iterator end() const noexcept;
    reverse_iterator rbegin() noexcept;
    reverse_iterator rend() noexcept;
    const_reverse_iterator rbegin() const noexcept;
    const_reverse_iterator rend() const noexcept;
    bool empty() const noexcept;
    void clear() noexcept;
    size_t size() const noexcept;
    iterator insert(const_iterator pos, T const &value);
    iterator erase(const_iterator pos) noexcept;
    iterator erase(const_iterator first, const_iterator last) noexcept;
    void splice(const_iterator pos, List &other, const_iterator first, const_iterator last);

    template <typename Q>
    friend void swap(List<Q> &a, List<Q> &b) noexcept;
};

template <typename Q>
void swap(List<Q> &a, List<Q> &b) noexcept
{
    std::swap(a.center.next, b.center.next);
    std::swap(a.center.prev, b.center.prev);
    std::swap(a.length_, b.length_);
    if (a.length_ != 0) {
        a.center.next->prev = &a.center;
        a.center.prev->next = &a.center;
    } else {
        a.center.next = &a.center;
        a.center.prev = &a.center;
    }
    if (b.length_ != 0) {
        b.center.next->prev = &b.center;
        b.center.prev->next = &b.center;
    } else {
        b.center.next = &b.center;
        b.center.prev = &b.center;
    }
}

template <typename T>
List<T>::List() noexcept : center({}), length_(0)
{
}

template <typename T>
List<T>::List(List const &other) : center({}), length_(0)
{
    if (other.size() != 0) {
        try {
            std::for_each(other.begin(), other.end(), [&](T t) { this->push_back(t); });
        } catch (std::exception &) {
            this->~List<T>();
            throw;
        }
    }
}

template <typename T>
List<T> &List<T>::operator=(List const &other)
{
    if (&(this->center) == &(other.center))
        return *this;
    List<T> tmp{other};
    swap(*this, tmp);
    return *this;
}

template <typename T>
List<T>::~List<T>() noexcept
{
    while (!this->empty()) {
        this->pop_back();
    }
}

template <typename T>
void List<T>::pop_back() noexcept
{
    assert(length_ > 0);
    Node *tmp = center.prev;
    center.prev = center.prev->prev;
    center.prev->next = &center;
    delete tmp;
    --length_;
}

template <typename T>
void List<T>::push_back(T const &t)
{
    Node *new_element = new DataNode(t);
    if (length_ == 0) {
        center.next = new_element;
        center.prev = new_element;
        new_element->next = &center;
        new_element->prev = &center;
    } else {
        center.prev->next = new_element;
        new_element->prev = center.prev;
        new_element->next = &center;
        center.prev = new_element;
    }
    ++length_;
}

template <typename T>
void List<T>::pop_front() noexcept
{
    assert(length_ > 0);
    Node *tmp = center.next;
    center.next->next->prev = &center;
    center.next = center.next->next;
    delete tmp;
    --length_;
}

template <typename T>
void List<T>::push_front(T const &t)
{
    Node *new_element = new DataNode(t);
    if (length_ == 0) {
        center.next = new_element;
        center.prev = new_element;
        new_element->next = &center;
        new_element->prev = &center;
    } else {
        center.next->prev = new_element;
        new_element->next = center.next;
        new_element->prev = &center;
        center.next = new_element;
    }
    ++length_;
}

template <typename T>
typename List<T>::iterator List<T>::begin() noexcept
{
    return iterator(center.next);
}

template <typename T>
typename List<T>::const_iterator List<T>::begin() const noexcept
{
    return const_iterator(center.next);
}

template <typename T>
typename List<T>::iterator List<T>::end() noexcept
{
    return iterator(&center);
}

template <typename T>
typename List<T>::const_iterator List<T>::end() const noexcept
{
    return const_iterator(&center);
}

template <typename T>
typename List<T>::reverse_iterator List<T>::rbegin() noexcept
{
    return reverse_iterator(&center);
}

template <typename T>
typename List<T>::const_reverse_iterator List<T>::rbegin() const noexcept
{
    return const_reverse_iterator(&center);
}

template <typename T>
typename List<T>::reverse_iterator List<T>::rend() noexcept
{
    return reverse_iterator(center.next);
}

template <typename T>
typename List<T>::const_reverse_iterator List<T>::rend() const noexcept
{
    return const_reverse_iterator(center.next);
}

template <typename T>
T &List<T>::front()
{
    return *(center.next->get_data());
}

template <typename T>
T const &List<T>::front() const
{
    return *(center.next->get_data());
}

template <typename T>
T &List<T>::back()
{
    return *(center.prev->get_data());
}

template <typename T>
T const &List<T>::back() const
{
    return *(center.prev->get_data());
}

template <typename T>
bool List<T>::empty() const noexcept
{
    return length_ == 0;
}

template <typename T>
void List<T>::clear() noexcept
{
    *this = List<T>{};
}

template <typename T>
size_t List<T>::size() const noexcept
{
    return length_;
}

template <typename T>
typename List<T>::iterator List<T>::insert(const_iterator pos, T const &value)
{
    Node *new_element = new DataNode(value);
    new_element->next = pos.node;
    new_element->prev = pos.node->prev;
    pos.node->prev->next = new_element;
    pos.node->prev = new_element;
    ++length_;
    return iterator(new_element);
}

template <typename T>
typename List<T>::iterator List<T>::erase(const_iterator pos) noexcept
{
    Node *tmp = pos.node;
    Node *nexttmp = tmp->next;
    tmp->prev->next = tmp->next;
    tmp->next->prev = tmp->prev;
    delete tmp;
    --length_;
    return iterator(nexttmp);
}

template <typename T>
typename List<T>::iterator List<T>::erase(const_iterator first, const_iterator last) noexcept
{
    size_t amount_of_deletions = std::distance(first, last);
    while (amount_of_deletions) {
        first = erase(first);
        --amount_of_deletions;
    }
    return iterator(first.node);
}

template <typename T>
void List<T>::splice(const_iterator pos, List &other, const_iterator first, const_iterator last)
{
    if (&(other.center) == &(this->center)) {
        Node *first_out = first.node->prev;
        Node *last_in = last.node->prev;
        first_out->next = last.node;
        last.node->prev = first_out;
        first.node->prev = pos.node->prev;
        pos.node->prev->next = first.node;
        last_in->next = pos.node;
        pos.node->prev = last_in;
    } else {
        size_t dist = std::distance(first, last);
        while (dist > 0) {
            pos = ++insert(pos, *first);
            first = other.erase(first);
            --dist;
        }
    }
}

} // namespace exam

#endif // exam_list_hpp
