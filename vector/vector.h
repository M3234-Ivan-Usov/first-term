#include <cstdlib>
#include <functional>
#include <memory>
#include <new>
#include <cstring>

template <typename T>
struct vector
{
    using iterator = T*;
    using const_iterator = T const*;

    vector() : size_(0), capacity_(0), data_(nullptr) {}

    vector(vector const& other) : size_(other.size_), capacity_(other.size_), data_(nullptr) {
        if (!other.empty()) {
            size_t constructed = 0;
            try {
                data_ = allocator.allocate(capacity_);
                for (; constructed < size_; ++constructed) {
                     allocator.construct_at(data_ + constructed, other.data_[constructed]);
               }
            } catch (...) {
                if (data_ != nullptr) {
                    allocator.destroy_n(data_, constructed);
                    data_ = allocator.deallocate(data_, capacity_);
                }
                size_ = 0, capacity_ = 0;
                throw std::runtime_error("Exception in copy constructor");
            }
        }
    }

    vector& operator=(vector const& other) {
        vector<T> temp(other);
        swap(temp);
        return *this;
    }

    ~vector() {
        clear();
        if (data_ != nullptr) {
            data_ = allocator.deallocate(data_, capacity_);
            capacity_ = 0;
        }
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
        if (size_ == capacity_) {
            T value = element;
            push_back_realloc(value);
        }
        else {
            allocator.construct_at(data_ + size_, element);
            size_++;
        }
    }

    void pop_back() {
        if (!empty()) {
            size_--;
            allocator.destroy_at(end());
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
            new_buffer(new_size);
        }
    }

    void shrink_to_fit() {
        if (empty() && data_!= nullptr) {
            data_ = allocator.deallocate(data_, capacity_);
            capacity_ = 0;
            return;
        }
        if (size_ != capacity_) {
            new_buffer(size_);
        }
    }

    void clear() {
        if (!empty() && data_ != nullptr) {
            allocator.destroy_n(data_, size_);
            size_ = 0;
        }
    }

    void swap(vector& other) {
        std::swap(data_, other.data_);
        std::swap(capacity_, other.capacity_);
        std::swap(size_, other.size_);
    }

    iterator begin() {
        return data_;
    }

    iterator end() {
        return data_ + size_;
    }

    const_iterator begin() const {
        return data_;
    }

    const_iterator end() const {
        return data_ + size_;
    }

    iterator insert(iterator pos, T const& element) {
        auto as_const = const_cast<const_iterator>(pos);
        return insert(as_const, element);
    }

    iterator insert(const_iterator pos, T const& element) {
        if (pos == end()) {
            push_back(element);
            return end() - 1;
        }
        vector<T> temp;
        temp.reserve(capacity_);
        for (auto it = begin(); it != pos; ++it) {
            temp.push_back(*it);
        }
        temp.push_back(element);
        iterator new_element = &temp.back();
        for (auto it = pos; it != end(); ++it) {
            temp.push_back(*it);
        }
        swap(temp);
        return new_element;
    }

    iterator erase(iterator pos) {
        auto as_const = const_cast<const_iterator>(pos);
        return erase(as_const);
    }

    iterator erase(const_iterator pos) {
        if (pos == end() - 1) {
            pop_back();
            return end();
        }
        vector<T> temp;
        temp.reserve(capacity_);
        iterator new_element = nullptr;
        for (auto it = begin(); it != end(); ++it) {
            if (it != pos) { temp.push_back(*it); }
            else { new_element = temp.end(); }
        }
        swap(temp);
        return new_element;
    }

    iterator erase(iterator first, iterator last) {
        auto cfirst = const_cast<const_iterator>(first);
        auto clast = const_cast<const_iterator>(last);
        return erase(cfirst, clast);
    }

    iterator erase(const_iterator first, const_iterator last) {
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

private:
    size_t get_new_capacity() const  {
        return (empty())? DEFAULT_INITIAL : capacity_ * ENSURE_COEFFICIENT;
    }

    void push_back_realloc(T const& element) {
        new_buffer(get_new_capacity());
        push_back(element);
    }

    void new_buffer(size_t new_capacity) {
        T* temp_data = nullptr;
        size_t elements = size_;
        size_t constructed = 0;
        try {
            temp_data = allocator.allocate(new_capacity);
            if (!empty()) {
                for (; constructed < elements; ++constructed) {
                    allocator.construct_at(temp_data + constructed, data_[constructed]);
                }
            }
            this->~vector<T>();
            data_ = temp_data;
            capacity_ = new_capacity;
            size_ = elements;
        }
        catch(...) {
            if (temp_data != nullptr) {
                allocator.destroy_n(temp_data, constructed);
                temp_data = allocator.deallocate(temp_data, new_capacity);
            }
            throw std::runtime_error("Failed to allocate new buffer");
        }
    }

    size_t static constexpr DEFAULT_INITIAL = 10;
    size_t static constexpr ENSURE_COEFFICIENT = 2;

    size_t size_;
    size_t capacity_;

    T* data_;

    struct My_Allocator {

        T* allocate(size_t amount) { return static_cast<T*>(::operator new(sizeof (T) * amount)); }

        void construct_at(T* ptr, T const & value) { ::new(ptr) T(value); }

        void destroy_at(T* ptr) { ptr->~T(); }

        void destroy_n(T* first, size_t amount) {
            for (size_t el = 0; el < amount; ++el) {
                destroy_at(first + el);
            }
        }

        T* deallocate(T* ptr, size_t) {
            ::operator delete(ptr);
            return nullptr;
        }

    } allocator;
};
