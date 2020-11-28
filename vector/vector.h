#include <cstdlib>
#include <memory>
#include <new>
#include <cstring>
#include <cassert>

template <typename T>
struct vector
{
    using iterator = T*;
    using const_iterator = T const*;

    vector() : size_(0), capacity_(0), data_(nullptr) {}

    vector(vector const& other) : size_(other.size_), capacity_(other.size_), data_(nullptr) {
        if (!other.empty()) {
            data_ = copy_from(other.data_, capacity_);
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
            ::operator delete(data_);
            data_ = nullptr;
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
            ::new(data_ + size_) T(element);
            size_++;
        }
    }

    void pop_back() {
        if (!empty()) {
            size_--;
            end()->~T();
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
            ::operator delete(data_);
            data_ = nullptr;
            capacity_ = 0;
            return;
        }
        if (size_ != capacity_) {
            new_buffer(size_);
        }
    }

    void clear() {
        if (!empty() && data_ != nullptr) {
            for (size_t el = 0; el < size_; ++el) {
                (data_ + el)->~T();
            }
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
        ptrdiff_t offset = pos - begin();
        push_back(element);
        iterator new_position = begin() + offset;
        for (auto it = &back(); it != new_position; --it) {
             std::swap(*it, it[-1]);
        }
        return new_position;
    }

    iterator insert(const_iterator pos, T const& element) {
        return insert(const_cast<iterator>(pos), element);
    }

    iterator erase(iterator pos) {
        if (pos == &back()) {
            pop_back();
            return end();
        }
        return erase(pos, pos + 1);
    }

    iterator erase(const_iterator pos) {
        return erase(const_cast<iterator>(pos));
    }

    iterator erase(iterator first, iterator last) {
        for (; last != end(); ++last, ++first) {
            *first = *last;
        }
        while(end() != first) {
            pop_back();
        }
        return first;
    }

    iterator erase(const_iterator first, const_iterator last) {
        auto mfirst = const_cast<iterator>(first);
        auto mlast = const_cast<iterator>(last);
        return erase(mfirst, mlast);
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
        assert(new_capacity >= size_);
        size_t elements = size_;
        T* temp_data = copy_from(data_, new_capacity);
        this->~vector<T>();
        data_ = temp_data;
        capacity_ = new_capacity;
        size_ = elements;
    }

    T* copy_from(T* src, size_t new_capacity) {
        T* dest = nullptr;
        size_t constructed = 0;
        try {
            dest = static_cast<T*>(::operator new(sizeof (T) * new_capacity));
            for (; constructed < size_; ++constructed) {
                ::new(dest + constructed) T(src[constructed]);
            }
        }
        catch(...) {
            if (dest != nullptr) {
                for (size_t el = 0; el < constructed; ++el) {
                    (dest + el)->~T();
                }
                ::operator delete(dest);
            }
            throw;
        }
        return dest;
    }

    size_t static constexpr DEFAULT_INITIAL = 10;
    size_t static constexpr ENSURE_COEFFICIENT = 2;

    size_t size_;
    size_t capacity_;

    T* data_;
};
