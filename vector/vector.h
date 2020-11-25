#include <cstdlib>
#include <functional>
#include <memory>
#include <new>
#include <cstring>

template <typename T>
struct vector
{
    typedef T* iterator;
    typedef T const* const_iterator;
    typedef std::allocator<T> allocator;

    vector() :
        size_(0), capacity_(0), data_(nullptr) {}

    vector(vector const& other) :
        size_(other.size_), capacity_(other.size_), data_(nullptr) {
        size_t constructed = 0;
        try {
            if (!other.empty()) {
                data_ = mem_allocator.allocate(capacity_);
                for (; constructed < size_; ++constructed) {
                    mem_allocator.construct(data_ + constructed,
                                            other.data_[constructed]);
                }
            }
        } catch (...) {
            if (data_ != nullptr) {
                std::destroy_n(data_, constructed);
                mem_allocator.deallocate(data_, capacity_);
                data_ = nullptr;
            }
            size_ = 0, capacity_ = 0;
            throw std::runtime_error("Exception in copy constructor");
        }
    }

    vector& operator=(vector const& other) {
        try {
            vector<T> temp(other);
            full_destroy();
            capacity_ = 0;
            size_ = 0;
            swap(temp);
        }
        catch (...) {
            throw std::runtime_error("Exception in operator=");
        }
        return *this;
    }

    ~vector() {
        full_destroy();
    }

    T& operator[](size_t i) {
        return data_[i];
    }

    T const& operator[](size_t i) const {
        return data_[i];
    }

    T* data() {
        return data_;
    }

    T const* data() const {
        return data_;
    }

    size_t size() const {
        return size_;
    }

    T& front() {
        return data_[0];
    }

    T const& front() const {
        return data_[0];
    }

    T& back() {
        return data_[size_ - 1];
    }

    T const& back() const {
        return data_[size_ - 1];
    }

    void push_back(T const& element) {
        T val = element;
        if (size_ == capacity_) {
            try {
                push_back_realloc(val);
            }
            catch(...) {
                throw std::runtime_error("Exception in push_back()");
            }
        }
        else {
            mem_allocator.construct(data_ + size_, val);
            size_++;
        }
    }

    void pop_back() {
        if (!empty()) {
            size_--;
            std::destroy_at(end());
        }
    }

    bool empty() const {
        return size_ == 0;
    }

    size_t capacity() const {
        return capacity_;
    }

    void reserve(size_t new_size) {
        if (new_size > capacity_) {
            try {
                new_buffer(new_size);
            }
            catch(...) {
                throw std::runtime_error("Exception in reserve()");
            }
        }
    }

    void shrink_to_fit() {
        if (empty() && data_!= nullptr) {
            mem_allocator.deallocate(data_, capacity_);
            data_ = nullptr;
            return;
        }
        if (size_ != capacity_) {
            try {
                new_buffer(size_);
            }
            catch(...) {
                throw std::runtime_error("Exception in shrink_to_fit()");
            }
        }
    }

    void clear() {
        if (!empty() && data_ != nullptr) {
            std::destroy_n(data_, size_);
            size_ = 0;
        }
    }

    void swap(vector& other) {
        T* temp_data = data_;
        data_ = other.data_;
        other.data_ = temp_data;
        size_t temp = size_;
        size_ = other.size_;
        other.size_ = temp;
        temp = capacity_;
        capacity_ = other.capacity_;
        other.capacity_ = temp;
    }

    iterator begin() {
        return &data_[0];
    }

    iterator end() {
        return &data_[size_];
    }

    const_iterator begin() const {
        return &data_[0];
    }

    const_iterator end() const {
        return &data_[size_];
    }

    iterator insert(iterator pos, T const& element) {
        auto as_const = const_cast<const_iterator>(pos);
        return insert(as_const, element);
    }

    iterator insert(const_iterator pos, T const& element) {
        vector<T> temp;
        try {
            if (pos == end()) {
                push_back(element);
                return end() - 1;
            }
            temp.reserve(capacity_);
            for (auto it = begin(); it != pos; ++it) {
                temp.push_back(*it);
            }
            temp.push_back(element);
            iterator new_element = &temp.back();
            for (auto it = pos; it != end(); ++it) {
                temp.push_back(*it);
            }
            clear();
            capacity_ = 0;
            swap(temp);
            return new_element;
        }
        catch(...) {
            throw std::runtime_error("Exception in insert()");
        }
    }

    iterator erase(iterator pos) {
        auto as_const = const_cast<const_iterator>(pos);
        return erase(as_const);
    }

    iterator erase(const_iterator pos) {
        try {
            if (pos == end() - 1) {
                pop_back();
                return end();
            }
            vector<T> temp;
            temp.reserve(capacity_);
            iterator new_element = nullptr;
            for (auto it = begin(); it != end(); ++it) {
                if (it != pos) {
                    temp.push_back(*it);
                }
                else {
                    new_element = temp.end();
                }
            }
            clear();
            capacity_ = 0;
            swap(temp);
            return new_element;
        }
        catch(...) {
            throw std::runtime_error("Exception in erase()");
        }
    }

    iterator erase(iterator first, iterator last) {
        auto cfirst = const_cast<const_iterator>(first);
        auto clast = const_cast<const_iterator>(last);
        return erase(cfirst, clast);
    }

    iterator erase(const_iterator first, const_iterator last) {
        try {
            vector<T> temp;
            temp.reserve(capacity_);
            size_t position = 0;
            for (auto it = begin(); it != first; ++it, ++position) {
                temp.push_back(*it);
            }
            for (auto it = last; it != end(); ++it) {
                temp.push_back(*it);
            }
            swap(temp);
            return begin() + position;
        }
        catch(...) {
             throw std::runtime_error("Exception in erase()");
        }
    }

private:
    size_t increase_capacity() const  {
        return (empty())? DEFAULT_INITIAL : capacity_ * ENSURE_COEFFICIENT;
    }

    void push_back_realloc(T const& element) {
        new_buffer(increase_capacity());
        push_back(element);
    }

    void new_buffer(size_t new_capacity) {
        T* temp_data = nullptr;
        size_t elements = size_;
        size_t constructed = 0;
        try {
            temp_data = mem_allocator.allocate(new_capacity);
            if (!empty()) {
                for (; constructed < elements; ++constructed) {
                    mem_allocator.construct(temp_data + constructed, data_[constructed]);
                }
            }
            empty_swap(temp_data, new_capacity, elements);
        }
        catch(...) {
            if (temp_data != nullptr) {
                std::destroy_n(temp_data, constructed);
                mem_allocator.deallocate(temp_data, new_capacity);
            }
            throw std::runtime_error("Failed to allocate new buffer");
        }
    }

    void full_destroy() {
        clear();
        if (data_ != nullptr) {
            mem_allocator.deallocate(data_, capacity_);
            data_ = nullptr;
        }
    }

    void empty_swap(T* temp_data, size_t capacity, size_t size) {
        full_destroy();
        data_ = temp_data;
        capacity_ = capacity;
        size_ = size;
    }

    size_t static const DEFAULT_INITIAL = 10;
    size_t static const ENSURE_COEFFICIENT = 2;

    size_t size_;
    size_t capacity_;

    allocator mem_allocator;
    T* data_;
};
