#include "big_integer.h"

#include <algorithm>
#include <iostream>
#include <stdexcept>
#include <string>

namespace  {
    using lim = std::numeric_limits<uint32_t>;
    using signed_lim = std::numeric_limits<int32_t>;
    static constexpr uint32_t BITS_IN_CELL = 32;
    static constexpr uint64_t BASE = static_cast<uint64_t>(lim::max()) + 1;
}

big_integer::big_integer() : sign(false) {
    value.push_back(0);
}

big_integer::big_integer(int a) {
    sign = (a < 0) ? true : false;
    if (a == signed_lim::min()) {
        value.push_back(static_cast<uint32_t>(signed_lim::max()) + 1);
    } else {
        value.push_back(static_cast<uint32_t>(abs(a)));
    }
}

big_integer::big_integer(std::string const &str) : big_integer() {
    size_t index = 0;
    bool temp_sign = false;
    if (str[0] == '-') {
        temp_sign = true;
        index++;
    }
    for (; index < str.size(); ++index) {
        int next = str[index] - '0';
        if (next < 0 || next > 9) {
            throw std::runtime_error("Parsing error");
        }
        *this *= 10;
        *this += next;
    }
    sign = temp_sign;
}

bool big_integer::is_simple() const{
    return value.size() == 1;
}

void big_integer::trim() {
    while (value.size() > 1 && value.back() == 0) {
        value.pop_back();
    }
}

big_integer big_integer::binary() const {
    if (!sign) {
        return *this;
    }
    big_integer res = complementation();
    res.value.resize(value.size());
    res.value.push_back(lim::max());
    return res;
}

big_integer big_integer::complementation() const {
    if (!sign) {
        return *this;
    }
    big_integer res;
    res.value.resize(value.size());
    for (size_t i = 0; i < value.size(); ++i) {
        res.value[i] = ~value[i];
    }
    res++;
    res.trim();
    res.sign = (is_zero()) ? false :true;
    return res;
}

bool big_integer::is_zero() const {
    return (is_simple() && value[0] == 0);
}

big_integer &big_integer::operator=(big_integer const &other) = default;

big_integer &big_integer::operator+=(big_integer const &rhs) {
    if (sign ^ rhs.sign) {
        if (rhs.sign) {
            sign = true;
            *this -= rhs;
            sign = !sign;
        } else {
            sign = !sign;
            *this -= rhs;
            sign = (is_zero()) ? false : !sign;
        }
        return *this;
    }
    uint64_t carry = 0;
    uint64_t next = 0;
    size_t maxlen = std::max(value.size(), rhs.value.size());
    value.resize(maxlen + 1);
    for (size_t i = 0; i < maxlen || carry; ++i) {
        next = value[i] + carry + rhs.get_or_default(i);
        carry = next >> BITS_IN_CELL;
        next ^= BASE;
        value[i] = static_cast<uint32_t>(next);
    }
    trim();
    return *this;
}

big_integer &big_integer::operator-=(big_integer const &rhs) {
    if (sign ^ rhs.sign) {
        if (rhs.sign) {
            sign = true;
            *this += rhs;
            sign = false;
        } else {
            sign = !sign;
            *this += rhs;
            sign = (is_zero())? false : !sign;
        }
        return *this;
    }
    int32_t reverse = 1;
    if (absolute_compare(rhs)) {
        reverse = -1;
        value.resize(rhs.value.size());
    }
    int64_t carry = 0;
    int64_t next = 0;
    for (size_t i = 0; i < rhs.value.size() || carry; ++i) {
        next = (static_cast<int64_t>(value[i]) - static_cast<int64_t>(rhs.get_or_default(i))) * reverse - carry;
        carry = 0;
        if (next < 0) {
            carry = 1;
            next += BASE;
        }
        value[i] = static_cast<uint32_t>(next);
    }
    trim();
    sign = sign ^ (reverse == -1);
    return *this;
}

big_integer &big_integer::operator*=(big_integer const &rhs) {
    big_integer old = *this;
    uint64_t carry = 0;
    uint64_t next = 0;
    value = std::vector<uint32_t>(value.size() + rhs.value.size());
    for (size_t i = 0; i < old.value.size(); ++i) {
        for (size_t j = 0; j < rhs.value.size() || carry; j++) {
            next = value[i + j] + old.value[i] * 1ull * rhs.get_or_default(j) + carry;
            value[i + j] = static_cast<uint32_t>(next & lim::max());
            carry = next >> BITS_IN_CELL;
        }
    }
    trim();
    sign ^= rhs.sign;
    return *this;
}

big_integer &big_integer::operator/=(big_integer const &rhs) {
    if (rhs.is_zero()) {
        throw std::runtime_error("Division by zero");
    } else if (value.size() < rhs.value.size()) {
        return *this = big_integer();
    } else if (rhs.is_simple()) {
        uint32_t divisor = rhs.value[0];
        sign ^= rhs.sign;
        return *this = simple_division(divisor).first;
    }
    size_t n = value.size();
    size_t m = rhs.value.size();
    auto norm = normalise(rhs, m);
    value.resize(n - m + 1);
    norm.first.value.push_back(0);
    const uint64_t d2 = (static_cast<uint64_t>(norm.second.value[m - 1]) << BITS_IN_CELL) +
            static_cast<uint64_t>(norm.second.value[m - 2]);
    int32_t c = static_cast<int32_t>(n - m);
    uint64_t k = static_cast<uint64_t>(n - m);
    for (; c >= 0; --c, --k) {
        auto qt = norm.first.trial(k , m, static_cast<uint128_t>(d2));
        big_integer qt_as_big;
        qt_as_big.value[0] = qt;
        big_integer dq = qt_as_big * norm.second;
        dq.value.resize(m + 1);
        while (norm.first.smaller(dq, k, m)) {
            qt--;
            dq = norm.second * static_cast<int>(qt);
        }
        value[static_cast<size_t>(k)] = qt;
        norm.first.difference(dq, k, m);
    }
    trim();
    sign ^= rhs.sign;
    return *this;
}

big_integer &big_integer::operator%=(big_integer const &rhs) {
    return *this -= (*this / rhs) * rhs;
}

big_integer &big_integer::operator&=(big_integer const &rhs) {
    bit_function foo = [](uint32_t x, uint32_t y) { return x & y; };
    sign_function bar = [](bool x, bool y) { return x && y;};
    return *this = bit_operation(rhs, foo, bar);
}

big_integer &big_integer::operator|=(big_integer const &rhs) {
    bit_function foo = [](uint32_t x, uint32_t y) { return x | y; };
    sign_function bar = [](bool x, bool y) { return x || y;};
    return *this = bit_operation(rhs, foo, bar);
}

big_integer &big_integer::operator^=(big_integer const &rhs) {
    bit_function foo = [](uint32_t x, uint32_t y) { return x ^ y; };
    sign_function bar = [](bool x, bool y) { return x ^ y;};
    return *this = bit_operation(rhs, foo, bar);
}

big_integer &big_integer::operator<<=(int rhs) {
    if (rhs < 0) {
        return *this >>= -rhs;
    } else if (rhs == 0) {
        return *this;
    }
    auto shift = static_cast<uint32_t>(rhs);
    auto cell_shift = shift % BITS_IN_CELL;
    shift /= BITS_IN_CELL;
    big_integer old = *this;
    value = std::vector<uint32_t>(shift);
    for (size_t i = 0; i < old.value.size(); ++i) {
        value.push_back(old.value[i]);
    }
    uint32_t remainder = 0;
    uint32_t temp = 0;
    for (size_t i = shift; i < value.size(); ++i) {
        temp = (value[i] << cell_shift) + remainder;
        remainder = value[i] >> (BITS_IN_CELL - cell_shift);
        value[i] = temp;
    }
    value.push_back(remainder);
    trim();
    return *this;
}

big_integer &big_integer::operator>>=(int rhs) {
    if (rhs < 0) {
        return *this <<= -rhs;
    } else if (rhs == 0) {
        return *this;
    }
    auto shift = static_cast<uint32_t>(rhs);
    auto cell_shift = shift % BITS_IN_CELL;
    shift /= BITS_IN_CELL;
    big_integer num = binary();
    if (sign && shift > 1) {
        for (size_t i = 1; i < shift; ++i) {
            num.value.push_back(lim::max());
        }
    }
    if (shift >= num.value.size()) {
        return *this = big_integer();
    }
    big_integer old = *this;
    value.clear();
    for (size_t i = shift; i < num.value.size(); i++) {
        value.push_back(num.value[i]);
    }
    for (size_t i = 0; i < value.size() - 1; ++i) {
        value[i] >>= cell_shift;
        uint32_t moving_part = value[i + 1] << (BITS_IN_CELL - cell_shift);
        value[i] += moving_part;
    }
    value.back() >>= cell_shift;
    trim();
    if (num.sign) {
        value.pop_back();
    }
    return *this = complementation();
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
    big_integer result = *this;
    result.sign = !result.sign;
    return --result;
}

big_integer &big_integer::operator++() {
    *this += 1;
    return *this;
}

big_integer big_integer::operator++(int) {
    big_integer result = *this;
    *this += 1;
    return result;
}

big_integer &big_integer::operator--() {
    *this -= 1;
    return *this;
}

big_integer big_integer::operator--(int) {
    big_integer result = *this;
    *this -= 1;
    return result;
}

big_integer operator+(big_integer const &a, big_integer const &b) {
    big_integer result = a;
    return result += b;
}

big_integer operator-(big_integer const &a, big_integer const &b) {
    big_integer result = a;
    return result -= b;
}

uint32_t big_integer::get_or_default(size_t index) const {
    return (index < value.size()) ? value[index] : 0;
}

std::pair<big_integer, uint32_t> big_integer::simple_division(uint32_t divisor) const {
    if (divisor == 0) {
        throw std::runtime_error("Division by zero");
    }
    uint64_t carry = 0;
    big_integer result;
    result.value.resize(value.size());
    size_t index = value.size() - 1;
    for (auto it = value.rbegin(); it != value.rend(); ++it, --index) {
        uint64_t next = *it + (carry << BITS_IN_CELL);
        result.value[index] = static_cast<uint32_t>(next / divisor);
        carry = next % divisor;
    }
    result.trim();
    result.sign = sign;
    return {result, carry};
}

big_integer operator*(big_integer const &a, big_integer const &b) {
    big_integer result = a;
    return result *= b;
}

bool big_integer::smaller(big_integer const &dq, uint64_t k, uint64_t m) const {
    uint64_t i = m;
    uint64_t j = 0;
    while (i != j) {
        (value[i + k] != dq.value[i]) ? j = i : i--;
    }
    return value[i + k] < dq.value[i];
}

uint32_t big_integer::trial(uint64_t k, uint64_t m, uint128_t const d2) const {
    uint64_t km = k + m;
    uint128_t r3 = (static_cast<uint128_t>(value[km]) << (2 * BITS_IN_CELL)) +
            (static_cast<uint128_t>(value[km - 1]) << BITS_IN_CELL) +
            static_cast<uint128_t>(value[km - 2]);
    return static_cast<uint32_t>(std::min(r3 / d2, static_cast<uint128_t>(lim::max())));
}

void big_integer::difference(big_integer const &dq, uint64_t k, uint64_t m) {
    int64_t borrow = 0;
    int64_t diff = 0;
    const int64_t b = static_cast<int64_t>(BASE);
    for (uint64_t i = 0; i <= m; ++i) {
        diff = static_cast<int64_t>(value[i + k]) - static_cast<int64_t>(dq.value[i]) - borrow + b;
        value[i + k] = static_cast<uint32_t>(diff % b);
        borrow = 1 - (diff >> BITS_IN_CELL);
    }
}

std::pair<big_integer, big_integer> big_integer::normalise(big_integer const &b, size_t m) {
    auto f = BASE / (static_cast<uint64_t>(b.value[m - 1]) + 1);
    auto normalised = std::make_pair(*this * static_cast<int>(f), b * static_cast<int>(f));
    normalised.first.sign = false;
    normalised.second.sign = false;
    return normalised;
}

big_integer operator/(big_integer const &a, big_integer const &b) {
    big_integer result = a;
    return result /= b;
}

big_integer operator%(big_integer const &a, big_integer const &b) {
    big_integer result = a;
    return result %= b;
}

big_integer big_integer::bit_operation(big_integer const &b,
                          bit_function foo, sign_function bar) {
    big_integer first = binary();
    big_integer second = b.binary();
    if (first.value.size() < second.value.size()) {
        std::swap(first, second);
    }
    second.value.resize(first.value.size());
    for (size_t i = 0; i < first.value.size(); ++i) {
        first.value[i] = foo(first.value[i], second.value[i]);
    }
    first.trim();
    first.sign = (first.is_zero())? false : bar(sign, b.sign);
    return first.complementation();
}

big_integer operator&(big_integer const &a, big_integer const &b) {
    big_integer result = a;
    return result &= b;
}

big_integer operator|(big_integer const &a, big_integer const &b) {
    big_integer result = a;
    return result |= b;
}

big_integer operator^(big_integer const &a, big_integer const &b) {
    big_integer result = a;
    return result ^= b;
}

big_integer operator<<(big_integer const &a, int b) {
    big_integer result = a;
    return result <<= b;
}

big_integer operator>>(big_integer const &a, int b) {
    big_integer result = a;
    return result >>= b;
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
    if (a.value.size()< b.value.size()) {
        return true^reverse;
    }
    if (b.value.size() < a.value.size()) {
        return false^reverse;
    }
    auto it_a = a.value.rbegin();
    auto it_b = b.value.rbegin();
    for (; it_a != a.value.rend(); ++it_a, ++it_b) {
        if (*it_a < *it_b) {
            return true^reverse;
        }
        if (*it_a > *it_b) {
            return false^reverse;
        }
    }
    return false;
}

bool big_integer::absolute_compare(big_integer const &other) {
    bool reverse = false;
    bool old_sign = sign;
    if (other.sign) {
        sign = true;
        reverse = true;
    } else {
        sign = false;
        reverse = false;
    }
    bool compare = *this < other;
    sign = old_sign;
    return compare ^ reverse;
}

bool operator>(big_integer const &a, big_integer const &b) {
    return !(b >= a);
}

bool operator<=(big_integer const &a, big_integer const &b) {
    return !(a > b);
}

bool operator>=(big_integer const &a, big_integer const &b) {
    return !(a < b);
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
        result += ('0' + static_cast<char>(next.second));
    }
    if (sign) {
        result += '-';
    }
    std::reverse(result.begin(), result.end());
    return result;
}

std::ostream &operator<<(std::ostream &s, big_integer const &a) {
    s << to_string(a);
    return s;
}
