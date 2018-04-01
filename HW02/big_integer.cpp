#include "big_integer.h"

#include <algorithm>
#include <climits>
#include <cstring>
#include <stdexcept>

using ui = unsigned int;
using ull = uintmax_t;

big_integer::big_integer()
{
    number.push_back(0);
    sign = 0;
}

big_integer::big_integer(big_integer const &rhs)
{
    number = rhs.number;
    sign = rhs.sign;
}

big_integer::big_integer(int a)
{
    if (a < 0)
        sign = -1;
    if (a == 0)
        sign = 0;
    if (a > 0)
        sign = 1;
    this->number.push_back(ui_cast(std::abs(a)));
}

big_integer::big_integer(std::string const &str)
{
    big_integer multiplier = 1;
    sign = 1;
    for (size_t i = str.size() - 1; i < str.size(); i--) {
        if ((i == 0) && (str[i] == '-')) {
            sign = -1;
            break;
        }
        *this += multiply_digit(multiplier, ui_cast(str[i] - '0'));
        multiplier = multiply_digit(multiplier, 10);
    }
    normalize(*this);
}

big_integer::~big_integer() = default;

big_integer &big_integer::operator=(big_integer const &other)
{
    number = other.number;
    sign = other.sign;
    return *this;
}

void big_integer::add_absolute(big_integer &a, big_integer const &rhs)
{
    ull carry = 0;
    a.sign = 1;
    for (size_t i = 0; i < std::min(a.number.size(), rhs.number.size()); i++) {
        ull test = ull_cast(a.number[i]) + ull_cast(rhs.number[i]) + carry;
        a.number[i] += rhs.number[i] + carry;
        (test > logic_base) ? carry = 1 : carry = 0;
    }
    for (size_t i = std::min(a.number.size(), rhs.number.size());
         i < std::max(a.number.size(), rhs.number.size()); i++) {
        if (a.number.size() < rhs.number.size())
            a.number.push_back(rhs.number[i]);
        ull test = ull_cast(a.number[i]) + ull_cast(carry);
        a.number[i] += carry;
        (test > logic_base) ? carry = 1 : carry = 0;
    }
    if (carry) {
        a.number.push_back(ui_cast(carry));
    }
    normalize(a);
}

void big_integer::sub_different(big_integer &a, big_integer const &rhs)
{
    if (a >= rhs) {
        a.sign = 1;
    } else {
        a.sign = -1;
    }
    sub_absolute(a, rhs);
    if (zero(a))
        a.sign = 0;
}

void big_integer::sub_absolute(big_integer &a, big_integer const &rhs)
{
    ui carry = 0;

    if (a > rhs)
        a.sign = 1;
    else
        a.sign = -1;
    size_t rhslen = std::min(rhs.number.size(), a.number.size());
    for (size_t i = 0; i < rhslen; i++) {
        ui carrytmp = carry;
        ui digit, other;
        if (a.sign == 1) {
            digit = a.number[i];
            other = rhs.number[i];
        } else {
            digit = rhs.number[i];
            other = a.number[i];
        }
        (other + carry > digit) ? carry = 1 : carry = 0;
        a.number[i] = digit - other - carrytmp;
    }

    while (carry) {
        if (a.number.size() < rhslen)
            a.number.push_back(0);
        ui digit;
        a.sign == 1 ? digit = a.number[rhslen] : digit = rhs.number[rhslen];
        ui carrytmp = (digit == 0) ? 1 : 0;
        (digit != 0) ? digit -= carry : digit = logic_base;
        a.number[rhslen] = digit;
        carry = carrytmp;
        rhslen++;
    }
    normalize(a);
    if (zero(a))
        a.sign = 0;
}

big_integer &big_integer::operator+=(big_integer const &rhs)
{
    if ((sgn(*this) >= 0) && (sgn(rhs) >= 0)) {
        add_absolute(*this, rhs);
    } else if ((sgn(*this) == -1) && (sgn(rhs) == -1)) {
        change_sign(*this);
        add_absolute(*this, -rhs);
        change_sign(*this);
    } else if ((sgn(*this) == -1) && (sgn(rhs) >= 0)) {
        change_sign(*this);
        sub_absolute(*this, rhs);
        change_sign(*this);
    } else {
        sub_absolute(*this, -rhs);
    }
    return *this;
}

big_integer operator+(big_integer a, big_integer const &b)
{
    a += b;
    return a;
}

big_integer &big_integer::operator-=(big_integer const &rhs)
{
    if ((sgn(*this) >= 0) && (sgn(rhs) >= 0)) {
        sub_absolute(*this, rhs);
    } else if ((sgn(*this) == -1) && (sgn(rhs) == -1)) {
        change_sign(*this);
        sub_absolute(*this, -rhs);
        change_sign(*this);
    } else if ((sgn(*this) == -1) && (sgn(rhs) >= 0)) {
        change_sign(*this);
        add_absolute(*this, rhs);
        change_sign(*this);
    } else {
        add_absolute(*this, -rhs);
    }
    return *this;
}

big_integer operator-(big_integer a, big_integer const &b)
{
    a -= b;
    return a;
}

big_integer big_integer::operator-() const
{
    big_integer tmp = *this;
    tmp.sign *= (-1);
    return tmp;
}

big_integer big_integer::operator+() const
{
    big_integer tmp = *this;
    if (tmp.sign == -1)
        tmp.sign = 1;
    return tmp;
}

big_integer &big_integer::operator++()
{
    *this += 1;
    return *this;
}

big_integer big_integer::operator++(int)
{
    big_integer tmp = *this;
    ++(*this);
    return tmp;
}

big_integer &big_integer::operator--()
{
    *this -= 1;
    return *this;
}

big_integer big_integer::operator--(int)
{
    big_integer tmp = *this;
    --(*this);
    return tmp;
}

void big_integer::change_sign(big_integer &a) { a.sign *= -1; }

template <typename Lambda>
big_integer big_integer::logic_operation(big_integer a, big_integer b,
                                         Lambda &&lambda) const
{
    if (a.number.size() < b.number.size()) {
        size_t old_size = a.number.size();
        a.number.resize(b.number.size());
        for (size_t i = old_size; i < a.number.size(); i++) {
            a.number[i] = 0;
        }
    }
    if (a.number.size() > b.number.size()) {
        size_t old_size = b.number.size();
        b.number.resize(a.number.size());
        for (size_t i = old_size; i < b.number.size(); i++) {
            b.number[i] = 0;
        }
    }
    if (a.sign == -1) {
        for (size_t i = 0; i < a.number.size(); i++)
            a.number[i] = ~a.number[i];
        a.sign = 1;
        a += 1;
    }
    if (b.sign == -1) {
        for (size_t i = 0; i < b.number.size(); i++)
            b.number[i] = ~b.number[i];
        b.sign = 1;
        b += 1;
    }
    for (size_t i = 0; i < a.number.size(); i++) {
        a.number[i] = lambda(a.number[i], b.number[i]);
    }
    if (zero(a))
        a.sign = 0;
    if (a.number[a.number.size() - 1] > (logic_base >> 1)) {
        for (size_t i = 0; i < a.number.size(); i++)
            a.number[i] = ~a.number[i];
        a += 1;
        a.sign = -1;
    }
    return a;
}

big_integer &big_integer::operator&=(big_integer const &rhs)
{
    *this =
        this->logic_operation(*this, rhs, [&](ui a, ui b) { return a & b; });
    return *this;
}

big_integer &big_integer::operator|=(big_integer const &rhs)
{
    *this =
        this->logic_operation(*this, rhs, [&](ui a, ui b) { return a | b; });
    return *this;
}

big_integer &big_integer::operator^=(big_integer const &rhs)
{
    *this =
        this->logic_operation(*this, rhs, [&](ui a, ui b) { return a ^ b; });
    return *this;
}

big_integer &big_integer::operator<<=(int rhs)
{
    ui delta = 0;
    std::vector<ui> result(number.size() + ui_cast((rhs + 31) / 32));
    size_t j = 0;
    while (rhs >= int(bits_in_base)) {
        rhs -= (bits_in_base);
        result[j++] = 0;
    }
    if (!rhs) {
        for (size_t i = 0; i < number.size(); i++) {
            result[j++] = number[i];
        }
        number = result;
        normalize(*this);
        return *this;
    }
    for (size_t i = 0; i < number.size(); i++) {
        ui newdigit = delta | (number[i] << rhs);
        result[j++] = newdigit;
        delta = ((number[i] >> (bits_in_base - ui_cast(rhs))));
    }
    if (delta)
        result[j++] = delta;
    number = result;
    normalize(*this);
    return *this;
}

big_integer &big_integer::operator>>=(int rhs)
{
    ui delta = 0;
    ui true_rhs = ui_cast(rhs);
    if (sgn(*this) == -1)
        *this -= 1;
    std::vector<ui> result;
    size_t start = 0;
    while (rhs >= int(bits_in_base)) {
        start = true_rhs / (bits_in_base);
        true_rhs %= bits_in_base;
    }
    for (size_t i = number.size() - 1; (i >= start) && (i < number.size());
         i--) {
        ui newdigit =
            (number[i] >> true_rhs) & ((1 << (bits_in_base - true_rhs)) - 1);
        newdigit |= delta;
        delta = (number[i] & ((1 << true_rhs) - 1))
                << (bits_in_base - true_rhs);
        result.push_back(newdigit);
    }
    std::reverse(result.begin(), result.end());
    this->number = result;
    if (this->sign < 0)
        *this -= 1;
    normalize(*this);
    return *this;
}

big_integer big_integer::operator~() const
{
    big_integer tmp = -(*this + 1);
    return tmp;
}

big_integer operator&(big_integer a, big_integer const &b)
{
    a &= b;
    return a;
}

big_integer operator|(big_integer a, big_integer const &b)
{
    a |= b;
    return a;
}

big_integer operator^(big_integer a, big_integer const &b)
{
    a ^= b;
    return a;
}

big_integer operator>>(big_integer a, int b) { return a >>= b; }

big_integer operator<<(big_integer a, int b)
{
    a <<= b;
    return a;
}

bool operator<(big_integer const &a, big_integer const &b)
{
    if (sgn(a) != sgn(b))
        return sgn(a) < sgn(b);
    if (a.number.size() > b.number.size())
        return false;
    if (b.number.size() > a.number.size())
        return true;
    size_t minlen = std::min(a.number.size(), b.number.size());
    for (size_t i = minlen - 1; i < minlen; i--) {
        if (a.number[i] < b.number[i])
            return true;
        if (a.number[i] > b.number[i])
            return false;
    }
    return false;
}

bool operator<=(big_integer const &a, big_integer const &b) { return !(a > b); }

bool operator==(big_integer const &a, big_integer const &b)
{
    if (sgn(a) != sgn(b))
        return false;
    if (a.number.size() != b.number.size())
        return false;
    for (size_t i = a.number.size() - 1; i < a.number.size(); i--) {
        if (a.number[i] != b.number[i])
            return false;
    }
    return true;
}

bool operator!=(big_integer const &a, big_integer const &b)
{
    return !(a == b);
}

bool operator>(big_integer const &a, big_integer const &b)
{
    return !((a < b) || (a == b));
}

bool operator>=(big_integer const &a, big_integer const &b) { return !(a < b); }

std::string to_string(big_integer const &a)
{
    std::string str = "";
    big_integer tmp = a;
    if (tmp.zero(tmp))
        tmp.sign = 0;
    int sign = tmp.sign;
    abs(tmp);
    while (tmp > 0) {
        str += std::to_string(tmp.digit_rem(tmp, 10));
        tmp = tmp.digit_quot(tmp, 10);
    }
    if (sign == -1)
        str.push_back('-');
    if (a == 0)
        str = "0";
    std::reverse(str.begin(), str.end());
    return str;
}

std::ostream &operator<<(std::ostream &s, big_integer const &a)
{
    return s << to_string(a);
}

void big_integer::multiply_absolute(big_integer &a, big_integer const &b)
{
    big_integer result;
    result.sign = 1;
    result.number.resize(a.number.size() + b.number.size() + 2, 0);
    for (size_t i = 0; i < a.number.size(); i++) {
        ull storage = 0, borrow = 0, tmp = 0;
        for (size_t j = 0; j < b.number.size(); j++) {
            storage = ull_cast(a.number[i]) * ull_cast(b.number[j]);
            tmp = (storage & logic_base) + result.number[i + j] + borrow;
            result.number[i + j] = ui_cast(tmp & logic_base);
            borrow = (storage >> (bits_in_base)) + (tmp >> (bits_in_base));
        }
        result.number[i + b.number.size()] += ui_cast(borrow);
    }
    normalize(result);
    std::swap(a, result);
}

big_integer big_integer::multiply_digit(big_integer const &num, ui x)
{
    big_integer tmp(num);
    ull remainder = 0;
    for (size_t i = 0; i < tmp.number.size(); i++) {
        ull product = tmp.number[i] * ull_cast(x) + remainder;
        tmp.number[i] = ui_cast(product & (logic_base));
        remainder = product >> (bits_in_base);
    }
    if (remainder)
        tmp.number.push_back(ui_cast(remainder));
    tmp.sign = 1;
    normalize(tmp);
    return tmp;
}

big_integer &big_integer::operator*=(big_integer const &rhs)
{
    int startsign = this->sign;
    multiply_absolute(*this, rhs);
    this->sign = rhs.sign * startsign;
    return *this;
}

big_integer operator*(big_integer a, big_integer const &rhs)
{
    a *= rhs;
    return a;
}

big_integer big_integer::digit_quot(big_integer const &num, ui x)
{
    ull tmp = 0;
    big_integer result;
    for (size_t i = num.number.size() - 1; i < num.number.size(); i--) {
        tmp = (tmp << (bits_in_base)) + num.number[i];
        result.number.push_back(ui_cast(tmp / x));
        tmp %= x;
    }
    std::reverse(result.number.begin(), result.number.end());
    normalize(result);
    return result;
}

ui big_integer::digit_rem(big_integer const &num, ui x)
{
    ull tmp = 0;
    for (size_t i = num.number.size() - 1; i < num.number.size(); i--) {
        tmp = ((tmp << (bits_in_base)) + num.number[i]) % x;
    }
    return ui_cast(tmp);
}

ui big_integer::trial(big_integer const &r, big_integer const &d, ui k, ui m)
{
    ui km = k + m;
    if (r == 0)
        return 0;
    ull r1 = (ull_cast(r.number[km]) << (bits_in_base)) +
             (ull_cast(r.number[km - 1]));
    ull d1 = ull_cast(d.number[m - 1]);
    ull left = r1 / d1;
    return std::min(ui_cast(left), logic_base);
}

bool big_integer::cmp_prefix(big_integer const &r, big_integer const &d, ui k,
                             ui m)
{
    size_t i = m, j = 0;
    while (i != j) {
        if (r.number[i + k] != d.number[i])
            j = i;
        else
            --i;
    }
    return r.number[i + k] < d.number[i];
}

std::pair<big_integer, big_integer> big_integer::divide(big_integer const &x,
                                                        big_integer const &y)
{
    big_integer r, q;
    q.sign = 1;
    size_t n = x.number.size(), m = y.number.size();
    ull e = base;
    ui f = ui_cast(e / (y.number[y.number.size() - 1] + 1));
    if (f != 0)
        r = multiply_digit(x, f);
    else
        r = x << (static_cast<int>(bits_in_base));
    big_integer d = multiply_digit(y, f);
    if (f == 0)
        d = y << (static_cast<int>(bits_in_base));
    q.number.resize(n - m + 2, 0);
    r.number.push_back(0);
    for (size_t k = n - m; k < n - m + 1; k--) {
        ui qt = trial(r, d, k, m);
        if (qt == 0)
            continue;
        big_integer dq = multiply_digit(d, qt);
        dq.number.push_back(0);
        while (cmp_prefix(r, dq, k, m)) {
            --qt;
            dq -= d;
        }
        q.number[k] = qt;
        r -= dq << static_cast<int>(((bits_in_base)*k));
    }
    if (f != 0)
        r = digit_quot(r, f);
    else
        r = r >> static_cast<int>(bits_in_base);
    normalize(q);
    std::pair<big_integer, big_integer> ans{q, r};
    return ans;
}

std::pair<big_integer, big_integer> big_integer::division(big_integer const &a,
                                                          big_integer const &b)
{
    if (zero(b)) {
        throw(std::runtime_error("Division by zero!"));
    }
    if (a < b)
        return std::pair<big_integer, big_integer>{0, a};
    if (a == b)
        return std::pair<big_integer, big_integer>{1, 0};
    if (b.number.size() == 1) {
        return std::pair<big_integer, big_integer>{digit_quot(a, b.number[0]),
                                                   digit_rem(a, b.number[0])};
    }
    return divide(a, b);
}

big_integer &big_integer::operator/=(big_integer const &rhs)
{
    bool sign = (sgn(*this) == sgn(rhs));
    abs(*this);
    if (rhs.sign < 0) {
        big_integer tmp = rhs;
        abs(tmp);
        *this = division(*this, tmp).first;
    } else {
        *this = division(*this, rhs).first;
    }
    if (sign)
        this->sign = 1;
    else
        this->sign = -1;
    normalize(*this);
    return *this;
}

big_integer &big_integer::operator%=(big_integer const &rhs)
{
    bool sign = ((sgn(*this) <= 0) && (sgn(rhs) > 0));
    abs(*this);
    if (rhs.sign < 0) {
        big_integer tmp = rhs;
        abs(tmp);
        *this = division(*this, tmp).second;
    } else {
        *this = division(*this, rhs).second;
    }
    if (sign)
        this->sign = -1;
    else
        this->sign = 1;
    normalize(*this);
    return *this;
}

big_integer operator/(big_integer a, big_integer const &b) { return a /= b; }

big_integer operator%(big_integer a, big_integer const &b) { return a %= b; }

int sgn(big_integer const &a) { return a.sign; }

void big_integer::normalize(big_integer &a)
{
    while ((a.number.back() == 0) && (a.number.size() > 1))
        a.number.pop_back();
    if (a.number.size() == 1 && a.number[0] == 0)
        a.sign = 0;
}

void abs(big_integer &a)
{
    if (a.sign == -1)
        a.sign = 1;
}

bool big_integer::zero(big_integer const &a) const
{
    if ((a.number.size() == 1) && (a.number[0]) == 0)
        return true;
    else
        return false;
}

template <typename T>
inline ui big_integer::ui_cast(T const &x)
{
    return static_cast<ui>(x);
}

template <typename T>
inline ull big_integer::ull_cast(T const &x)
{
    return static_cast<ull>(x);
}
