#ifndef BIG_INTEGER_H
#define BIG_INTEGER_H

#include <cstddef>
#include <cstdint>
#include <iosfwd>
#include <vector>
#include <functional>
#include <limits>

__extension__ typedef unsigned __int128 uint128_t;

using bit_function = std::function<uint32_t(uint32_t, uint32_t)>;
using sign_function = std::function<bool(bool, bool)>;

struct big_integer {   
    big_integer();

    big_integer(big_integer const &other) = default;

    big_integer(int a);

    explicit big_integer(std::string const &str);

    ~big_integer() = default;

    big_integer &operator=(big_integer const &other);

    big_integer &operator+=(big_integer const &rhs);

    big_integer &operator-=(big_integer const &rhs);

    big_integer &operator*=(big_integer const &rhs);

    big_integer &operator/=(big_integer const &rhs);

    big_integer &operator%=(big_integer const &rhs);

    big_integer &operator&=(big_integer const &rhs);

    big_integer &operator|=(big_integer const &rhs);

    big_integer &operator^=(big_integer const &rhs);

    big_integer &operator<<=(int rhs);

    big_integer &operator>>=(int rhs);

    big_integer operator+() const;

    big_integer operator-() const;

    big_integer operator~() const;

    big_integer &operator++();

    big_integer operator++(int);

    big_integer &operator--();

    big_integer operator--(int);

    friend bool operator==(big_integer const &a, big_integer const &b);

    friend bool operator!=(big_integer const &a, big_integer const &b);

    friend bool operator<(big_integer const &a, big_integer const &b);

    friend bool operator>(big_integer const &a, big_integer const &b);

    friend bool operator<=(big_integer const &a, big_integer const &b);

    friend bool operator>=(big_integer const &a, big_integer const &b);

    friend big_integer operator+(big_integer const &a, big_integer const &b);

    friend big_integer operator-(big_integer const &a, big_integer const &b);

    friend big_integer operator*(big_integer const &a, big_integer const &b);

    friend big_integer operator/(big_integer const &a, big_integer const &b);

    friend big_integer operator%(big_integer const &a, big_integer const &b);

    friend big_integer operator&(big_integer const &a, big_integer const &b);

    friend big_integer operator|(big_integer const &a, big_integer const &b);

    friend big_integer operator^(big_integer const &a, big_integer const &b);

    friend big_integer operator<<(big_integer const &a, int b);

    friend big_integer operator>>(big_integer const &a, int b);

    friend std::string to_string(big_integer const &a);

private:
    using lim = std::numeric_limits<uint32_t>;
    using signed_lim = std::numeric_limits<int32_t>;
    void trim();

    big_integer binary() const;

    big_integer complementation() const;

    std::pair<big_integer, big_integer> normalise(big_integer const &rhs, size_t m);

    big_integer bit_operation(big_integer const &rhs, bit_function foo, sign_function bar);

    uint32_t get_or_default(size_t index) const;

    std::pair<big_integer, uint32_t> simple_division(uint32_t x) const;

    bool smaller(big_integer const &dq, uint64_t k, uint64_t m) const;

    uint32_t trial(uint64_t k, uint64_t m, uint128_t const d2) const;

    void difference(big_integer const &dq, uint64_t k, uint64_t m);

    bool is_zero() const;
    bool is_simple() const;
    bool absolute_compare(big_integer const & other);

    static constexpr uint32_t BITS_IN_CELL = 32;
    static constexpr uint64_t BASE = static_cast<uint64_t>(lim::max()) + 1;

    bool sign;
    std::vector<uint32_t> value;

};

big_integer operator+(big_integer const &a, big_integer const &b);

big_integer operator-(big_integer const &a, big_integer const &b);

big_integer operator*(big_integer const &a, big_integer const &b);

big_integer operator/(big_integer const &a, big_integer const &b);

big_integer operator%(big_integer const &a, big_integer const &b);

big_integer operator&(big_integer const &a, big_integer const &b);

big_integer operator|(big_integer const &a, big_integer const &b);

big_integer operator^(big_integer const &a, big_integer const &b);

big_integer operator<<(big_integer const &a, int b);

big_integer operator>>(big_integer const &a, int b);

bool operator==(big_integer const &a, big_integer const &b);

bool operator!=(big_integer const &a, big_integer const &b);

bool operator<(big_integer const &a, big_integer const &b);

bool operator>(big_integer const &a, big_integer const &b);

bool operator<=(big_integer const &a, big_integer const &b);

bool operator>=(big_integer const &a, big_integer const &b);

std::string to_string(big_integer const &a);

std::ostream &operator<<(std::ostream &s, big_integer const &a);

#endif  // BIG_INTEGER_H
