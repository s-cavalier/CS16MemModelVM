#ifndef M__QUEUE_H__
#define M__QUEUE_H__
#include "Error.h"
#include "UniquePtr.h"
#include "InitializerList.h"
#include "Iterator.h"

namespace ministl {

    template <typename T>
    class list {
        struct Node {
            Node* prev;
            Node* next;
            T value;

            template <typename... Args>
            Node(Args&&... args) : prev(nullptr), next(nullptr), value(ministl::forward<Args>(args)...) {}

            ~Node() = default;
        };

        Node* head;
        Node* tail;
        size_t _size;

    public:
        
        list() : head(nullptr), tail(nullptr), _size(0) {}
        explicit list(size_t count) : head(nullptr), tail(nullptr), _size(count) {
            if (count == 0) return;
            
            head = new Node();
            tail = head;
            Node* it = head;

            for (size_t i = 1; i < count; ++i) {
                it->next = new Node();
                it->next->prev = it;
                it = it->next;
                tail = it;
            }

        }

        explicit list(size_t count, const T& value) : head(nullptr), tail(nullptr), _size(count) {
            if (count == 0) return;

            head = new Node(value);
            tail = head;
            Node* it = head;

            for (size_t i = 1; i < count; ++i) {
                it->next = new Node(value);
                it->next->prev = it;
                it = it->next;
                tail = it;
            }
        }

        list(const list& other) : head(nullptr), tail(nullptr), _size(0) {
            if (!other.head) return;

            Node* otherit = other.head;
            head = new Node(otherit->value);
            Node* it = head;

            otherit = otherit->next;
            while (otherit) {
                Node* n = new Node(otherit->value);
                it->next = n;
                n->prev = it;
                it = n;
                otherit = otherit->next;
            }
            tail = it;               
            _size = other._size;
        }


        list(list&& other) {
            head = other.head;
            tail = other.tail;
            _size = other._size;

            other.head = nullptr;
            other.tail = nullptr;
            other._size = 0;
        }

        list(std::initializer_list<T> il) : head(nullptr), tail(nullptr), _size(il.size()) {
            if (_size == 0) return;
            auto ilIt = il.begin();

            head = new Node(*ilIt);
            Node* it = head;
            ++ilIt;

            for (size_t i = 1; i < _size; ++i, ++ilIt) {
                it->next = new Node(*ilIt);
                it->next->prev = it;
                it = it->next;
            }

            tail = it;
        }


        void clear() {
            Node* it = head;

            while (it) {
                Node* temp = it->next;
                delete it;
                it = temp;
            }

            _size = 0;
            head = nullptr;
            tail = nullptr;
        }

        list& operator=(const list& other) {
            if (this == &other) return *this;
            if (!other.head) {
                clear();
                return *this;
            }

            list tmp;  

            if (other.head) {
                Node* oit = other.head;
                tmp.head = new Node(oit->value);
                Node* it = tmp.head;
                oit = oit->next;
                while (oit) {
                    Node* n = new Node(oit->value);
                    it->next = n;
                    n->prev = it;
                    it = n;
                    oit = oit->next;
                }
                tmp.tail = it;
                tmp._size = other._size;
            }

            Node* th = head; head = tmp.head; tmp.head = th;
            Node* tt = tail; tail = tmp.tail; tmp.tail = tt;
            size_t ts = _size; _size = tmp._size; tmp._size = ts;

            return *this;
        }


        list& operator=(list&& other) {
            if (this == &other) return *this;
            clear();
            head = other.head;
            tail = other.tail;
            _size = other._size;

            other.head = nullptr;
            other.tail = nullptr;
            other._size = 0;    
            return *this;
        }

        ~list() {
            clear();
        }

        T& front() {
            assert(head);
            return head->value;
        }
        const T& front() const {
            assert(head);
            return head->value;
        }
        T& back() {
            assert(tail);
            return tail->value;
        }
        const T& back() const {
            assert(tail);
            return tail->value;
        }

        bool empty() const {
            return _size == 0;
        }

        size_t size() const {
            return _size;
        }

        template <class... Args>
        T& emplace_back(Args&&... args) {
            if (_size == 0) {
                head = tail = new Node(ministl::forward<Args>(args)...);
                ++_size;
                return tail->value;
            }

            tail->next = new Node( ministl::forward<Args>(args)... );
            tail->next->prev = tail;
            tail = tail->next;
            ++_size;
            return tail->value;
        }

        void push_back(const T& value) {
            emplace_back(value);
        }

        void push_back(T&& value) {
            emplace_back(ministl::move(value));
        }

        template <class... Args>
        T& emplace_front(Args&&... args) {
            if (_size == 0) {
                head = tail = new Node(ministl::forward<Args>(args)...);
                ++_size;
                return head->value;
            }

            head->prev = new Node( ministl::forward<Args>(args)... );
            head->prev->next = head;
            head = head->prev;
            ++_size;
            return head->value;
        }

        void push_front(const T& value) {
            emplace_front(value);
        }

        void push_front(T&& value) {
            emplace_front( ministl::move(value) );
        }

        void pop_back() {
            assert(_size > 0);

            if (_size == 1) {
                delete tail;
                head = tail = nullptr;
                --_size;
                return;
            }

            tail = tail->prev;
            delete tail->next;
            tail->next = nullptr;
            --_size;
        }

        void pop_front() {
            assert(_size > 0);
            
            if (_size == 1) {
                delete head;
                head = tail = nullptr;
                --_size;
                return;
            }

            head = head->next;
            delete head->prev;
            head->prev = nullptr;
            --_size;
        }

        void resize(size_t count) {
            while (_size > count) pop_back();
            while (_size < count) emplace_back();
        }

        void resize(size_t count, const T& value) {
            while (_size > count) pop_back();
            while (_size < count) emplace_back(value);
        }

        class iterator {
            Node* ptr; 

        public:
            using difference_type   = ministl::ptrdiff_t;
            using value_type        = T;
            using pointer           = T*;
            using reference         = T&;
            using iterator_category = ministl::bidirectional_iterator_tag;

            iterator(Node* p = nullptr) : ptr(p) {}

            reference operator*() const { return ptr->value; }
            pointer operator->() const { return &ptr->value; }
            
            iterator& operator++() {
                ptr = ptr->next;
                return *this;
            }
            
            iterator operator++(int) {
                iterator tmp = *this;
                ptr = ptr->next;
                return tmp;
            }

            iterator& operator--() {
                ptr = ptr->prev;
                return *this;
            }

            iterator operator--(int) {
                iterator tmp = *this;
                ptr = ptr->prev;
                return tmp;
            }

            bool operator==(const iterator& other) const { return ptr == other.ptr; }
            bool operator!=(const iterator& other) const { return ptr != other.ptr; }

            friend class list;
        };

        class const_iterator {
            const Node* ptr;
        public:
            using difference_type   = ministl::ptrdiff_t;
            using value_type        = T;
            using pointer           = const T*;
            using reference         = const T&;
            using iterator_category = ministl::bidirectional_iterator_tag;

            const_iterator(const Node* p = nullptr) : ptr(p) {}
            reference operator*() const { return ptr->value; }
            pointer operator->() const { return &ptr->value; }

            const_iterator& operator++() { ptr = ptr->next; return *this; }
            const_iterator operator++(int) { auto tmp=*this; ptr=ptr->next; return tmp; }
            const_iterator& operator--() { ptr = ptr->prev; return *this; }
            const_iterator operator--(int) { auto tmp=*this; ptr=ptr->prev; return tmp; }

            bool operator==(const const_iterator& o) const { return ptr == o.ptr; }
            bool operator!=(const const_iterator& o) const { return ptr != o.ptr; }
        };

        iterator begin() { return iterator(head); }
        iterator end()   { return iterator(nullptr); }

        const_iterator begin() const { return const_iterator(head); }
        const_iterator end()   const { return const_iterator(nullptr); }

        const_iterator cbegin() const { return const_iterator(head); }
        const_iterator cend()   const { return const_iterator(nullptr); }

    };
    

}

#endif