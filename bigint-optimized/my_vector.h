#ifndef MY_VECTOR_H
#define MY_VECTOR_H

#include <cstdint>
#include <vector>
#include <memory>

struct my_vector {
public:
    my_vector();
    my_vector(my_vector const & other);


    my_vector& operator=(my_vector const &a);

    uint32_t& operator[](size_t index);
    uint32_t operator[](size_t index) const;

    void push_back(uint32_t x);
    void pop_back();
    void resize(size_t new_size);
    size_t size() const;

    ~my_vector();

private:
    using shared_value = std::shared_ptr<std::vector<uint32_t>>;

    static constexpr size_t SMALL_SIZE = 4;

    union {
        shared_value big_object;
        uint32_t small_object[SMALL_SIZE];
    };

     bool is_big;
     size_t elements;

     void expand();
     void fork();
     void copy(my_vector const & other);
     bool is_shared();
};


#endif // MY_VECTOR_H
