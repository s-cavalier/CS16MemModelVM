#ifndef M__QUEUE_H__
#define M__QUEUE_H__
#include "Error.h"

namespace ministl {

    typedef long ptrdiff_t;
    struct forward_iterator_tag {};

    template <typename T>
class queue {
    struct Node {
        T value;
        Node* next;
        Node* prev;

        Node(const T& v)
            : value(v), next(nullptr), prev(nullptr) {}
    };

    size_t _size;
    Node* head;
    Node* tail;

public:
    // Iterator for mutable access
    class iterator {
        Node* node;
    public:
        using value_type = T;
        using reference = T&;
        using pointer = T*;
        using difference_type = ptrdiff_t;
        using iterator_category = forward_iterator_tag;

        explicit iterator(Node* n) : node(n) {}
        reference operator*() const { return node->value; }
        pointer operator->() const { return &node->value; }

        iterator& operator++() {
            node = node->next;
            return *this;
        }
        iterator operator++(int) {
            iterator tmp = *this;
            node = node->next;
            return tmp;
        }

        bool operator==(const iterator& other) const { return node == other.node; }
        bool operator!=(const iterator& other) const { return node != other.node; }
    };

    // Iterator for const access
    class const_iterator {
        const Node* node;
    public:
        using value_type = T;
        using reference = const T&;
        using pointer = const T*;
        using difference_type = ptrdiff_t;
        using iterator_category = forward_iterator_tag;

        explicit const_iterator(const Node* n) : node(n) {}
        reference operator*() const { return node->value; }
        pointer operator->() const { return &node->value; }

        const_iterator& operator++() {
            node = node->next;
            return *this;
        }
        const_iterator operator++(int) {
            const_iterator tmp = *this;
            node = node->next;
            return tmp;
        }

        bool operator==(const const_iterator& other) const { return node == other.node; }
        bool operator!=(const const_iterator& other) const { return node != other.node; }
    };

    // Constructor
    queue()
        : _size(0), head(nullptr), tail(nullptr) {}

    // Destructor: free all nodes
    ~queue() {
        clear();
    }

    // Disable copy semantics
    queue(const queue&) = delete;
    queue& operator=(const queue&) = delete;

    // Enqueue: add to tail
    void enqueue(const T& value) {
        Node* node = new Node(value);
        if (!tail) {
            head = tail = node;
        } else {
            tail->next = node;
            node->prev = tail;
            tail = node;
        }
        ++_size;
    }

    // Dequeue: remove from head (assert if empty)
    T dequeue() {
        assert(head && "dequeue() called on empty queue");
        Node* node = head;
        T val = node->value;

        head = head->next;
        if (head)
            head->prev = nullptr;
        else
            tail = nullptr;

        delete node;
        --_size;
        return val;
    }

    // Peek at front element
    const T& front() const {
        assert(head && "front() called on empty queue");
        return head->value;
    }

    // Peek at back element
    const T& back() const {
        assert(tail && "back() called on empty queue");
        return tail->value;
    }

    // Number of elements
    size_t size() const noexcept {
        return _size;
    }

    // Empty check
    bool empty() const noexcept {
        return _size == 0;
    }

    // Clear all elements
    void clear() {
        while (head) {
            Node* tmp = head;
            head = head->next;
            delete tmp;
        }
        tail = nullptr;
        _size = 0;
    }

    // Iterator support
    iterator begin() {
        return iterator(head);
    }
    iterator end() {
        return iterator(nullptr);
    }
    const_iterator begin() const {
        return const_iterator(head);
    }
    const_iterator end() const {
        return const_iterator(nullptr);
    }
    const_iterator cbegin() const {
        return const_iterator(head);
    }
    const_iterator cend() const {
        return const_iterator(nullptr);
    }
};
    

}

#endif