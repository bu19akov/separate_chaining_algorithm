#ifndef ADS_SET_H
#define ADS_SET_H

#include <functional>
#include <algorithm>
#include <iostream>
#include <stdexcept>

template <typename Key, size_t N = 11>
class ADS_set {
public:
    class Iterator;
    using value_type = Key;
    using key_type = Key;
    using reference = value_type &;
    using const_reference = const value_type &;
    using size_type = size_t;
    using difference_type = std::ptrdiff_t;
    using const_iterator = Iterator;
    using iterator = const_iterator;
    using key_equal = std::equal_to<key_type>;
    using hasher = std::hash<key_type>;

private:
    struct Element {
        key_type key;
        Element *next;
        Element() : key {NULL}, next {NULL} {}
        Element(key_type key) : key {key}, next {NULL} {}
    };
    Element **table {nullptr};
    size_type table_size {0};
    size_type current_size {0};
    float max_lf {0.7};
    Element *add(const key_type &v);
    Element *locate(const key_type &key) const;
    size_type h(const key_type &v) const { return hasher{}(v) % table_size; }
    void reserve(size_type n);
    void rehash(size_type n);
public:
    ADS_set() { rehash(N); }
    ADS_set(std::initializer_list<key_type> ilist) : ADS_set{} { insert(ilist); }
    template<typename InputIt> ADS_set(InputIt first, InputIt last) : ADS_set{} { insert(first, last); }
    ADS_set(const ADS_set &other);
    ~ADS_set() {
        for (size_type idx {0}; idx < table_size; ++idx) {
            Element *help = table[idx];
            while (help != NULL) {
                Element *next = help;
                help = help->next;
                delete next;
                next = 0;
            }
        }
        delete[] table;
    }

    ADS_set &operator=(const ADS_set &other);
    ADS_set &operator=(std::initializer_list<key_type> ilist);

    size_type size() const { return current_size; }
    bool empty() const { return current_size == 0; }

    void insert(std::initializer_list<key_type> ilist) { insert(ilist.begin(), ilist.end()); }
    std::pair<iterator,bool> insert(const key_type &key);
    template<typename InputIt> void insert(InputIt first, InputIt last);

    void clear();
    size_type erase(const key_type &key);

    size_type count(const key_type &key) const { return locate(key) != nullptr; }
    iterator find(const key_type &key) const;

    void swap(ADS_set &other);

    const_iterator begin() const { return const_iterator{table[0], table, table_size, 0}; }
    const_iterator end() const { return const_iterator{table[table_size], table, table_size, table_size}; }

    void dump(std::ostream &o = std::cerr) const;

    friend bool operator==(const ADS_set &lhs, const ADS_set &rhs) {
        if (lhs.current_size != rhs.current_size)
            return false;
        for (size_type idx {0}; idx < lhs.table_size; ++idx) {
            Element *help = lhs.table[idx];
            while (help != NULL) {
                if (!rhs.count(help->key))
                    return false;
                else
                    help = help->next;
            }
        }
        return true;
    }
    friend bool operator!=(const ADS_set &lhs, const ADS_set &rhs) { return !(lhs == rhs); }
};

template <typename Key, size_t N>
void ADS_set<Key,N>::swap(ADS_set &other) {
    using std::swap;
    swap(table, other.table);
    swap(table_size, other.table_size);
    swap(current_size, other.current_size);
    swap(max_lf, other.max_lf);
}

template <typename Key, size_t N>
typename ADS_set<Key,N>::iterator ADS_set<Key,N>::find(const key_type &key) const {
    if (auto e {locate(key)})
        return iterator {e, table, table_size, h(key)};
    return end();
}

template <typename Key, size_t N>
typename ADS_set<Key,N>::size_type ADS_set<Key,N>::erase(const key_type &key) {
    if (auto e {locate(key)}) {
        size_type idx {h(key)};
        Element *help = table[idx], *prev = NULL;
        while (help != e) {
            prev = help;
            help = help->next;
        }
        if (prev == NULL)
            table[idx] = help->next;
        else
            prev->next = help->next;
        delete help;
        help = 0;
        --current_size;
        return 1;
    }
    return 0;
}

template <typename Key, size_t N>
void ADS_set<Key,N>::clear() {
    ADS_set tmp;
    swap(tmp);
}

template <typename Key, size_t N>
std::pair<typename ADS_set<Key,N>::iterator,bool> ADS_set<Key,N>::insert(const key_type &key) {
    if (auto e {locate(key)})
        return {iterator {e, table, table_size, h(key)}, false};
    reserve(current_size + 1);
    return {iterator {add(key), table, table_size, h(key)}, true};
}

template <typename Key, size_t N>
ADS_set<Key,N> &ADS_set<Key,N>::operator=(const ADS_set &other) {
    ADS_set tmp {other};
    swap(tmp);
    return *this;
}

template <typename Key, size_t N>
ADS_set<Key,N> &ADS_set<Key,N>::operator=(std::initializer_list<key_type> ilist) {
    ADS_set tmp {ilist};
    swap(tmp);
    return *this;
}

template <typename Key, size_t N>
ADS_set<Key,N>::ADS_set(const ADS_set &other) {
    rehash(other.table_size);
    for (size_type idx {0}; idx < other.table_size; ++idx) {
        Element *help = other.table[idx];
        while (help != NULL) {
            add(help->key);
            help = help->next;
        }
    }
}

template <typename Key, size_t N>
typename ADS_set<Key,N>::Element *ADS_set<Key,N>::add(const key_type &v) {
    size_type idx {h(v)};
    Element *help = new Element(v);
    if (table[idx] == NULL)
        table[idx] = help;
    else {
        Element *next = table[idx];
        table[idx] = help;
        help->next = next;
    }
    ++current_size;
    return help;
}

template <typename Key, size_t N>
typename ADS_set<Key,N>::Element *ADS_set<Key,N>::locate(const key_type &key) const {
    size_type idx {h(key)};
    Element *help = table[idx];
    while (help != NULL) {
        if (key_equal{}(help->key, key))
            return help;
        help = help->next;
    }
    return nullptr;
}

template <typename Key, size_t N>
template<typename InputIt> void ADS_set<Key,N>::insert(InputIt first, InputIt last) {
    for (auto it {first}; it != last; ++it) {
        if (!count(*it)) {
            reserve(current_size + 1);
            add(*it);
        }
    }
}

template <typename Key, size_t N>
void ADS_set<Key,N>::dump(std::ostream &o) const {
    o << "table_size = " << table_size << ", current_size = " << current_size << "\n";
    for (size_type idx {0}; idx < table_size + 1; ++idx) {
        Element *help = table[idx];
        o << idx << ": ";
        if (idx == table_size) {
            o << "--END";
        }
        else if (help == NULL) {
            o << "--FREE";
        }
        else {
            while (help != NULL) {
                o << " -> " << help->key;
                help = help->next;
            }
        }
        o << "\n";
    }
}

template <typename Key, size_t N>
void ADS_set<Key,N>::reserve(size_type n) {
    if (table_size * max_lf >= n)
        return;
    size_type new_table_size {table_size};
    while (new_table_size * max_lf < n)
        ++(new_table_size *= 2);
    rehash(new_table_size);
}

template <typename Key, size_t N>
void ADS_set<Key,N>::rehash(size_type n) {
    size_type new_table_size {std::max(N, std::max(n, size_type(current_size / max_lf)))};
    Element **new_table {new Element *[new_table_size+1]};
    for (size_type i {0}; i < new_table_size; i++)
        new_table[i] = 0;
    new_table[new_table_size] = 0;
    Element **old_table {table};
    size_type old_table_size {table_size};

    current_size = 0;
    table = new_table;
    table_size = new_table_size;

    for (size_type idx {0}; idx < old_table_size; ++idx) {
        Element *help = old_table[idx];
        while (help != NULL) {
            add(help->key);
            help = help->next;
        }
    }

    for (size_type idx {0}; idx < old_table_size; ++idx) {
        Element *help = old_table[idx];
        while (help != NULL) {
            Element *next = help;
            help = help->next;
            delete next;
            next = 0;
        }
    }
    delete[] old_table;
}

template <typename Key, size_t N>
class ADS_set<Key,N>::Iterator {
    Element *e;
    Element **line;
    size_type end;
    size_type idx;
public:
    using value_type = Key;
    using difference_type = std::ptrdiff_t;
    using reference = const value_type &;
    using pointer = const value_type *;
    using iterator_category = std::forward_iterator_tag;

    explicit Iterator(Element *e = NULL, Element **line = nullptr, size_type end = 0, size_type idx = 0): e{e}, line{line}, end{end}, idx{idx} { if(!e) skip(); }
    
    void skip() {
        while (e == NULL && idx < end) {
            ++idx;
            e = line[idx];
        }
    }

    reference operator*() const { return e->key; }
    pointer operator->() const { return &e->key; }
    
    Iterator &operator++() {
        e = e->next;
        skip();
        return (*this);
    }
    Iterator operator++(int) {
        auto rc {*this};
        ++(*this);
        return rc;
    }
    
    friend bool operator==(const Iterator &lhs, const Iterator &rhs) { return lhs.e == rhs.e; }
    friend bool operator!=(const Iterator &lhs, const Iterator &rhs) { return !(lhs == rhs); }
};

template <typename Key, size_t N>
void swap(ADS_set<Key,N> &lhs, ADS_set<Key,N> &rhs) { lhs.swap(rhs); }

#endif // ADS_SET_H
