#include "big_integer.h"

#include <algorithm>
#include <iostream>
#include <stdexcept>
#include <string>

__extension__ typedef unsigned __int128 uint128_t;

big_integer::big_integer() {
    value.push_back(0);
    sign = false;
}

big_integer::big_integer(big_integer const &other) {
    value = other.value;
    sign = other.sign;
}

big_integer::big_integer(int a) {
    if (a == INT32_MIN) {
        value.push_back(static_cast<uint32_t>(INT32_MAX) + 1);
    } else {
        value.push_back(static_cast<uint32_t>(abs(a)));
    }
    sign = (a < 0) ? true : false;
}

big_integer::big_integer(std::string const &str) {
    value.push_back(0);
    big_integer position = 1;
    for (auto i = static_cast<int32_t>(str.length()) - 1; i >= 0; --i) {
        int next = str[static_cast<size_t>(i)] - '0';
        if (next < 0 || next > 9) {
            if (i == 0 && str[0] == '-') {
                sign = true;
                break;
            }
            throw std::runtime_error("Parsing error");
        }
        *this += position * next;
        position *= 10;
    }
    trim();
}

big_integer::~big_integer() {
}

bool big_integer::is_simple() const{
    return value.size() == 1;
}

void big_integer::trim() {
    while (value.size() > 1 && value[value.size() - 1] == 0) {
        value.pop_back();
    }
}

big_integer big_integer::binary() const {
    if (!sign) {
        return *this;
    }
    big_integer res;
    res.value.pop_back();
    for (auto i : value) {
        res.value.push_back(~i);
    }
    res.value.push_back(UINT32_MAX);
    res += 1;
    res.sign = (is_zero()) ? false : true;
    return res;
}

big_integer big_integer::complementation() const {
    if (!sign) {
        return *this;
    }
    big_integer res;
    res.value.pop_back();
    for (auto i : value) {
        res.value.push_back(~i);
    }
    res += 1;
    res.trim();
    res.sign = (is_zero()) ? false :true;
    return res;
}

bool big_integer::is_zero() const {
    return (is_simple() && value[0] == 0);
}

big_integer &big_integer::operator=(big_integer const &other) {
    value = other.value;
    sign = other.sign;
    return *this;
}

big_integer &big_integer::operator+=(big_integer const &rhs) {
    *this = *this + rhs;
    return *this;
}

big_integer &big_integer::operator-=(big_integer const &rhs) {
    *this = *this - rhs;
    return *this;
}

big_integer &big_integer::operator*=(big_integer const &rhs) {
    *this = *this * rhs;
    return *this;
}

big_integer &big_integer::operator/=(big_integer const &rhs) {
    *this = *this / rhs;
    return *this;
}

big_integer &big_integer::operator%=(big_integer const &rhs) {
    *this = *this % rhs;
    return *this;
}

big_integer &big_integer::operator&=(big_integer const &rhs) {
    *this = *this & rhs;
    return *this;
}

big_integer &big_integer::operator|=(big_integer const &rhs) {
    *this = *this | rhs;
    return *this;
}

big_integer &big_integer::operator^=(big_integer const &rhs) {
    *this = *this ^ rhs;
    return *this;
}

big_integer &big_integer::operator<<=(int rhs) {
    *this = *this << rhs;
    return *this;
}

big_integer &big_integer::operator>>=(int rhs) {
    *this = *this >> rhs;
    return *this;
}

big_integer big_integer::operator+() const {
    return *this;
}

big_integer big_integer::operator-() const {
    big_integer result(*this);
    result.sign = (result.is_zero()) ? false : !sign;
    return result;
}

big_integer big_integer::operator~() const {
    big_integer result(*this);
    return -(result + 1);
}

big_integer &big_integer::operator++() {
    *this = *this+1;
    return *this;;
}

big_integer big_integer::operator++(int) {
    big_integer result = *this;
    *this = *this + 1;
    return result;
}

big_integer &big_integer::operator--() {
    *this = *this - 1;
    return *this;
}

big_integer big_integer::operator--(int) {
    big_integer result = *this;
    *this = *this - 1;
    return result;
}

big_integer operator+(big_integer const &a, big_integer const &b) {
    if (b.sign) {
        return a - (-b);
    } else if (a.sign) {
        return b - (-a);
    }
    big_integer result = a;
    uint64_t carry = 0;
    size_t maxlen = std::max(a.value.size(), b.value.size());
    for (size_t i = 0; i < maxlen || carry; ++i) {
        if (i == result.value.size()) {
            result.value.push_back(0);
        }
        uint64_t next = result.value[i] + carry + (i < b.value.size() ? b.value[i] : 0);
        carry = (next >= (uint64_t(UINT32_MAX) + 1) ? 1 : 0);
        if (carry) {
            next -= (uint64_t(UINT32_MAX) + 1);
        }
        result.value[i] = static_cast<uint32_t>(next);
    }
    return result;
}

big_integer operator-(big_integer const &a, big_integer const &b) {
    if (a.sign) {
        return -((-a) + b);
    } else if (b.sign) {
        return a + (-b);
    }
    if (a == b) {
        return big_integer();
    }
    bool result_sign = (a < b) ? true : false;
    auto big = a, small = b;
    if (result_sign) {
        std::swap(big, small);
    }
    int64_t carry = 0;
    for (size_t i = 0; i < small.value.size() || carry; ++i) {
        int64_t next = big.value[i] - (carry + (i < small.value.size() ? small.value[i] : 0));
        carry = (next < 0) ? 1 : 0;
        if (carry) {
            next += (uint64_t(UINT32_MAX) + 1);
        }
        big.value[i] = static_cast<uint32_t>(next);
    }
    big.trim();
    big.sign = result_sign;
    return big;
}

std::pair<big_integer, uint32_t> big_integer::simple_division(uint32_t divisor) const {
    if (divisor == 0) {
        throw std::runtime_error("Division by zero");
    }
    uint64_t carry = 0;
    big_integer result;
    result.value.pop_back();
    result.value.resize(value.size());
    for (int32_t i = static_cast<int32_t>(value.size()) - 1; i >= 0; --i) {
        uint64_t next = value[static_cast<size_t>(i)] + carry * (uint64_t(UINT32_MAX) + 1);
        result.value[static_cast<size_t>(i)] = static_cast<uint32_t>(next / divisor);
        carry = next % divisor;
    }
    result.trim();
    result.sign = sign;
    return {result, carry};
}

big_integer operator*(big_integer const &a, big_integer const &b) {
    big_integer result;
    result.value.resize(a.value.size() + b.value.size());
    uint64_t carry = 0;
    for (uint32_t i = 0; i < a.value.size(); ++i) {
        for (uint32_t j = 0; j < b.value.size() || carry; j++) {
            uint128_t next = result.value[i + j] + a.value[i] * 1ull * (j < b.value.size() ? b.value[j] : 0) + carry;
            result.value[i + j] = static_cast<uint32_t>(next % (uint64_t(UINT32_MAX) + 1));
            carry = static_cast<uint32_t>(next / (uint64_t(UINT32_MAX) + 1));
        }
    }
    result.trim();
    result.sign = a.sign ^ b.sign;
    return result;
}

bool big_integer::smaller(big_integer const &dq, uint64_t k, uint64_t m) const {
    uint64_t i = m, j = 0;
    while (i != j) {
        if (value[i + k] != dq.value[i]) {
            j = i;
        } else {
            i--;
        }
    }
    return value[i + k] < dq.value[i];
}

uint32_t big_integer::trial(uint64_t k, uint64_t m, uint128_t const d2) const {
    uint64_t km = k + m;
    uint128_t b = static_cast<uint64_t>(UINT32_MAX) + 1;
    uint128_t r3 = static_cast<uint128_t>(value[km]) * b * b +
            static_cast<uint128_t>(value[km - 1]) * b +
            static_cast<uint128_t>(value[km - 2]);
    return uint32_t(std::min(r3 / d2, static_cast<uint128_t>(UINT32_MAX)));
}

void big_integer::difference(big_integer const &dq, uint64_t k, uint64_t m) {
    int64_t borrow = 0, diff;
    const int64_t b = int64_t(UINT32_MAX) + 1;
    for (uint64_t i = 0; i <= m; i++) {
        diff = static_cast<int64_t>(value[i + k]) - static_cast<int64_t>(dq.value[i]) - borrow + b;
        value[i + k] = uint32_t(diff % b);
        borrow = 1 - diff / b;
    }
}

operands normalise(big_integer const &a, big_integer const &b, size_t m) {
    auto f = (1L << a.BITS_IN_CELL) / (uint64_t(b.value[m - 1]) + 1);
    auto normalised = std::make_pair(
                a * static_cast<int>(f), b * static_cast<int>(f));
    normalised.first.sign = false;
    normalised.second.sign = false;
    return normalised;
}

big_integer operator/(big_integer const &a, big_integer const &b) {
    if (b.is_zero()) {
        throw std::runtime_error("Division by zero");
    } else if (a.value.size() < b.value.size()) {
        return big_integer();
    } else if (b.is_simple()) {
        uint32_t divisor = b.value[0];
        big_integer result = (a.simple_division(divisor).first);
        return (b.sign)? -result : result;
    }
    size_t n = a.value.size(), m = b.value.size();
    operands norm = normalise(a, b, m);
    big_integer q;
    q.value.resize(n - m + 1);
    norm.first.value.push_back(0);
    const uint64_t d2 = (static_cast<uint64_t>(norm.second.value[m - 1]) << a.BITS_IN_CELL) +
            static_cast<uint64_t>(norm.second.value[m - 2]);
    for (auto k = int32_t(n - m); k >= 0; --k) {
        auto qt = norm.first.trial(uint64_t(k), m, static_cast<uint128_t>(d2));
        big_integer qt_as_big;
        qt_as_big.value[0] = qt;
        big_integer dq = qt_as_big * norm.second;
        dq.value.resize(m + 1);
        if (norm.first.smaller(dq, uint64_t(k), m)) {
            qt--;
            dq = norm.second * static_cast<int>(qt);
        }
        q.value[static_cast<size_t>(k)] = qt;
        norm.first.difference(dq, uint64_t(k), m);
    }
    q.trim();
    q.sign = a.sign ^ b.sign;
    return q;
}

big_integer operator%(big_integer const &a, big_integer const &b) {
   return a - ((a / b) * b);
}

big_integer bit_operation(big_integer const &a, big_integer const &b,
                          bit_function foo, sign_function bar) {
    big_integer first = a.binary();
    big_integer second = b.binary();
    if (first.value.size() < second.value.size()) {
        std::swap(first, second);
    }
    second.value.resize(first.value.size());
    for (size_t i = 0; i < first.value.size(); ++i) {
        first.value[i] = foo(first.value[i], second.value[i]);
    }
    first.trim();
    first.sign = (first.is_zero())? false : bar(a.sign, b.sign);
    return first.complementation();
}

big_integer operator&(big_integer const &a, big_integer const &b) {
    bit_function foo = [](uint32_t x, uint32_t y) { return x & y; };
    sign_function bar = [](bool x, bool y) { return (x && y) ? true : false; };
    return bit_operation(a, b, foo, bar);
}

big_integer operator|(big_integer const &a, big_integer const &b) {
    bit_function foo = [](uint32_t x, uint32_t y) { return x | y; };
    sign_function bar = [](bool x, bool y) { return (x || y) ? true : false; };
    return bit_operation(a, b, foo, bar);
}

big_integer operator^(big_integer const &a, big_integer const &b) {
    bit_function foo = [](uint32_t x, uint32_t y) { return x ^ y; };
    sign_function bar = [](bool x, bool y) { return (x ^ y) ? true : false; };
    return bit_operation(a, b, foo, bar);
}

big_integer operator<<(big_integer const &a, int b) {
    if (b < 0) {
        return a >> abs(b);
    } else if (b == 0) {
        return big_integer(a);
    }
    auto shift = static_cast<uint32_t>(b);
    auto cell_shift = shift % a.BITS_IN_CELL;
    shift /= a.BITS_IN_CELL;
    big_integer result;
    result.value.pop_back();
    for (uint32_t i = 0; i < shift; i++) {
        result.value.push_back(0);
    }
    for (auto i : a.value) {
        result.value.push_back(i);
    }
    uint32_t remainder = 0;
    for (size_t i = shift; i < result.value.size(); ++i) {
        uint32_t temp = (result.value[i] << cell_shift);
        temp += remainder;
        remainder = (result.value[i] >> (a.BITS_IN_CELL - cell_shift));
        result.value[i] = temp;
    }
    result.value.push_back(remainder);
    result.trim();
    result.sign = a.sign;
    return result;
}

big_integer operator>>(big_integer const &a, int b) {
    if (b < 0) {
        return a << abs(b);
    } else if (b == 0) {
        return big_integer(a);
    }
    big_integer num = a;
    auto shift = static_cast<uint32_t>(b);
    auto cell_shift = shift % a.BITS_IN_CELL;
    shift /= a.BITS_IN_CELL;
    if (num.sign) {
        num = num.binary();
        for (int32_t i = 0; i < int32_t(shift) - 1; ++i) {
            num.value.push_back(UINT32_MAX);
        }
    }
    big_integer result;
    if (shift >= num.value.size()) {
        return result;
    }
    result.value.pop_back();
    for (size_t i = shift; i < num.value.size(); i++) {
        result.value.push_back(num.value[i]);
    }
    for (size_t i = 0; i < result.value.size() - 1; i++) {
        result.value[i] >>= cell_shift;
        uint32_t moving_part = result.value[i + 1] << (a.BITS_IN_CELL - cell_shift);
        result.value[i] += moving_part;
    }
    result.value[result.value.size() - 1] >>= cell_shift;
    result.trim();
    result.sign = num.sign;
    if (num.sign) {
        result.value.pop_back();
    }
    return result.complementation();
}

bool operator==(big_integer const &a, big_integer const &b) {
    if (a.is_zero() && b.is_zero()) {
        return true;
    }
    if (a.value.size() != b.value.size() || a.sign != b.sign) {
        return false;
    }
    for (size_t i = 0; i < a.value.size(); ++i) {
        if (a.value[i] != b.value[i]) {
            return false;
        }
    }
    return true;
}

bool operator!=(big_integer const &a, big_integer const &b) {
    return !(a == b);
}

bool operator<(big_integer const &a, big_integer const &b) {
    if (a.is_zero() && b.is_zero()) {
        return false;
    }
    if (a.sign && !b.sign) {
        return true;
    }
    if (!a.sign && b.sign) {
        return false;
    }
    bool reverse = a.sign;
    for (auto i = static_cast<int32_t>(a.value.size()) - 1; i >= 0; --i) {
        auto index = static_cast<size_t>(i);
        if (a.value[index] < b.value[index]) {
            return true^reverse;
        }
        if (a.value[index] > b.value[index]) {
            return false^reverse;
        }
    }
    return false;

}

bool operator>(big_integer const &a, big_integer const &b) {
    return b < a && a != b;
}

bool operator<=(big_integer const &a, big_integer const &b) {
    return a < b || a == b;
}

bool operator>=(big_integer const &a, big_integer const &b) {
    return a > b || a == b;
}

std::string to_string(big_integer const &a) {
    std::string result;
    big_integer current = a;
    bool sign = current.sign;
    if (current.is_zero()) {
        return "0";
    }
    while (!current.is_zero()) {
        auto next = current.simple_division(10);
        current = next.first;
        result+=('0' + char(next.second));
    }
    if (sign) {
        result+='-';
    }
    std::reverse(result.begin(), result.end());
    return result;
}

std::ostream &operator<<(std::ostream &s, big_integer const &a) {
    s << to_string(a);
    return s;
}
