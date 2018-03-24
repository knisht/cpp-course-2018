#ifndef BIG_INTEGER_H
#define BIG_INTEGER_H

#include <cstddef>
#include <iosfwd>
#include <vector>
#include <climits>
#include <cstdint>

using ui = unsigned int;
using ull = uintmax_t;
const ull base = (ull)UINT_MAX+1;
const ui logic_base = UINT_MAX;
constexpr ui bits_in_base = sizeof(ui)*8;

struct big_integer
{
    big_integer();
    big_integer(big_integer const& other);
    big_integer(int a);
    explicit big_integer(std::string const& str);
    ~big_integer();

    big_integer& operator=(big_integer const& other);

    big_integer& operator+=(big_integer const& rhs);
    big_integer& operator-=(big_integer const& rhs);
    big_integer& operator*=(big_integer const& rhs);
    big_integer& operator/=(big_integer const& rhs);
    big_integer& operator%=(big_integer const& rhs);

    big_integer& operator&=(big_integer const& rhs);
    big_integer& operator|=(big_integer const& rhs);
    big_integer& operator^=(big_integer const& rhs);

    big_integer& operator<<=(int rhs);
    big_integer& operator>>=(int rhs);

    big_integer operator+() const;
    big_integer operator-() const;
    big_integer operator~() const;

    big_integer& operator++();
    big_integer operator++(int);

    big_integer& operator--();
    big_integer operator--(int);

    friend bool operator==(big_integer const& a, big_integer const& b);
    friend bool operator!=(big_integer const& a, big_integer const& b);
    friend bool operator<(big_integer const& a, big_integer const& b);
    friend bool operator>(big_integer const& a, big_integer const& b);
    friend bool operator<=(big_integer const& a, big_integer const& b);
    friend bool operator>=(big_integer const& a, big_integer const& b);

    friend std::string to_string(big_integer const& a);
    friend std::ostream& operator<<(std::ostream& s, big_integer const& a);
    friend void abs(big_integer& a);
    friend int sgn(big_integer const& a);

private:
    std::vector<ui> number;
    int sign;

    template<typename T>
    big_integer logic_operation(big_integer a, big_integer b, T&& lambda) const;

    void add_absolute(big_integer& a, big_integer const& b);
    void sub_absolute(big_integer &a, big_integer const&b);
    void sub_different(big_integer &a, big_integer const& rhs);
    void multiply_absolute(big_integer& a, big_integer const& b);
    big_integer multiply_digit(const big_integer &num, ui x);

    bool cmp_prefix(big_integer const& r, big_integer const& d, ui k, ui m);
    void difference(big_integer& r, big_integer& d, size_t k, size_t m);
    ui trial(big_integer const& r, big_integer const& d, ui k, ui m);
    ui digit_rem(big_integer const& num, ui x);
    big_integer digit_quot(big_integer const& num, ui x);
    std::pair<big_integer, big_integer> divide(const big_integer &x, const big_integer &y);
    std::pair<big_integer, big_integer> division(big_integer const& x, big_integer const& y);

    bool zero(const big_integer &a) const;
    void change_sign(big_integer& a);
    void normalize(big_integer& a);
};

big_integer operator+(big_integer a, big_integer const& b);
big_integer operator-(big_integer a, big_integer const& b);
big_integer operator*(big_integer a, big_integer const& b);
big_integer operator/(big_integer a, big_integer const& b);
big_integer operator%(big_integer a, big_integer const& b);

big_integer operator&(big_integer a, big_integer const& b);
big_integer operator|(big_integer a, big_integer const& b);
big_integer operator^(big_integer a, big_integer const& b);

big_integer operator<<(big_integer a, int b);
big_integer operator>>(big_integer a, int b);

bool operator==(big_integer const& a, big_integer const& b);
bool operator!=(big_integer const& a, big_integer const& b);
bool operator<(big_integer const& a, big_integer const& b);
bool operator>(big_integer const& a, big_integer const& b);
bool operator<=(big_integer const& a, big_integer const& b);
bool operator>=(big_integer const& a, big_integer const& b);

std::string to_string(big_integer const& a);
std::ostream& operator<<(std::ostream& s, big_integer const& a);
void abs(big_integer& a);
int sgn(big_integer const& a);

#endif  //BIG_INTEGER_H
