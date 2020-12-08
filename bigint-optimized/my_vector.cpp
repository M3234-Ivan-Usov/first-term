#include"my_vector.h"
#include<cstring>

my_vector::my_vector() : is_big(false), elements(0) {}

my_vector::my_vector(my_vector const & other) {
    copy(other);
}

void my_vector::fork() {
    if (is_shared()) {
        big_object.reset(new std::vector<uint32_t>(*big_object));
    }
}

bool my_vector::is_shared() {
    return is_big && big_object.use_count() != 1;
}

uint32_t& my_vector::operator[](size_t index) {
    fork();
    return is_big? big_object->at(index) : small_object[index];
}

uint32_t my_vector::operator[](size_t index) const {
    return is_big? big_object->at(index) : small_object[index];
}

my_vector& my_vector::operator=(my_vector const &a) {
    if (this == &a) {
        return *this;
    }
    this->~my_vector();
    copy(a);
    return *this;
}

void my_vector::copy(my_vector const & other) {
    if (other.is_big) {
        new (&big_object) shared_value(other.big_object);
    }
    else {
        std::copy(other.small_object, other.small_object + SMALL_SIZE, small_object);
    }
    is_big = other.is_big;
    elements = other.elements;
}

void my_vector::push_back(uint32_t x) {
    fork();
    if (elements == SMALL_SIZE) {
        expand();
    }
    if (is_big) {
        big_object->push_back(x);
    } else {
        small_object[elements] = x;
    }
    elements++;
}

void my_vector::pop_back() {
    fork();
    elements--;
    if (is_big) {
        big_object->pop_back();
    }
}

void my_vector::resize(size_t new_size) {
    if (new_size == elements) {
        return;
    }
    if (is_big) {
        fork();
        big_object->resize(new_size);
    }
    if (!is_big && new_size > SMALL_SIZE) {
        expand();
        big_object->resize(new_size);
    }
    if (!is_big && new_size <= SMALL_SIZE && new_size > elements) {
        std::fill_n(small_object + elements, new_size - elements, 0);
    }
    elements = new_size;
}

size_t my_vector::size() const {
    return elements;
}

void my_vector::expand() {
    if (is_big) {
        return;
    }
    std::vector<uint32_t>* temp = new std::vector<uint32_t>(small_object, small_object + elements);
    new(&big_object) shared_value(temp);
    is_big = true;
}

my_vector::~my_vector(){
    if (is_big) {
        big_object.~shared_ptr();
    }
}
