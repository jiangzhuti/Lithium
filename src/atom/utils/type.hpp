/*
 * type.hpp
 *
 * Copyright (C) 2023 Max Qian <lightapt.com>
 *
 * This program is free software: you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation, either version 3 of the License, or
 * (at your option) any later version.
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 * You should have received a copy of the GNU General Public License
 * along with this program.  If not, see <https://www.gnu.org/licenses/>.
 */

/*************************************************

Copyright: 2023 Max Qian. All rights reserved

Author: Max Qian

E-mail: astro_air@126.com

Date: 2023-12-13

Description: A simple type wrapper

**************************************************/

#include <cstddef>
#include <tuple>
#include <iostream>
#include <type_traits>

template <typename T>
class MyType
{
public:
    explicit MyType(const T &value) : m_value(value) {}

    // 加法运算符重载
    template <typename U>
    auto operator+(const MyType<U> &other) const
    {
        using ResultType = decltype(m_value + other.m_value);
        return MyType<ResultType>(m_value + other.m_value);
    }

    // 减法运算符重载
    template <typename U>
    auto operator-(const MyType<U> &other) const
    {
        using ResultType = decltype(m_value - other.m_value);
        return MyType<ResultType>(m_value - other.m_value);
    }

    // 乘法运算符重载
    template <typename U>
    auto operator*(const MyType<U> &other) const
    {
        using ResultType = decltype(m_value * other.m_value);
        return MyType<ResultType>(m_value * other.m_value);
    }

    // 除法运算符重载
    template <typename U>
    auto operator/(const MyType<U> &other) const
    {
        using ResultType = decltype(m_value / other.m_value);
        return MyType<ResultType>(m_value / other.m_value);
    }

    // 取模运算符重载
    template <typename U>
    auto operator%(const MyType<U> &other) const
    {
        using ResultType = decltype(m_value % other.m_value);
        return MyType<ResultType>(m_value % other.m_value);
    }

    // 等于运算符重载
    template <typename U>
    auto operator==(const MyType<U> &other) const
    {
        return m_value == other.m_value;
    }

    // 不等于运算符重载
    template <typename U>
    auto operator!=(const MyType<U> &other) const
    {
        return m_value != other.m_value;
    }

    // 小于运算符重载
    template <typename U>
    auto operator<(const MyType<U> &other) const
    {
        return m_value < other.m_value;
    }

    // 小于等于运算符重载
    template <typename U>
    auto operator<=(const MyType<U> &other) const
    {
        return m_value <= other.m_value;
    }

    // 大于运算符重载
    template <typename U>
    auto operator>(const MyType<U> &other) const
    {
        return m_value > other.m_value;
    }

    // 大于等于运算符重载
    template <typename U>
    auto operator>=(const MyType<U> &other) const
    {
        return m_value >= other.m_value;
    }

    T m_value; // 成员变量
};

template <typename Tuple, std::size_t N = std::tuple_size_v<Tuple>, typename T = void>
struct TuplePrinter
{
    static void print(const Tuple &t)
    {
        TuplePrinter<Tuple, N - 1, T>::print(t);
        std::cout << ", " << std::get<N - 1>(t);
    }
};

template <typename Tuple, std::size_t N>
struct TuplePrinter<Tuple, N, typename std::enable_if<N == 1>::type>
{
    static void print(const Tuple &t)
    {
        std::cout << std::get<0>(t);
    }
};

/*
int main()
{
    MyType<int> a(2);
    MyType<double> b(3.5);

    auto c = a + b; // MyType<double>(5.5)
    auto d = a - b; // MyType<double>(-1.5)
    auto e = a * b; // MyType<double>(7.0)
    auto f = a / b; // MyType<double>(0.571429)

    std::cout << std::boolalpha << (a == b) << '\n'; // false
    std::cout << std::boolalpha << (a != b) << '\n'; // true
    std::cout << std::boolalpha << (a < b) << '\n';  // true
    std::cout << std::boolalpha << (a <= b) << '\n'; // true
    std::cout << std::boolalpha << (a > b) << '\n';  // false
    std::cout << std::boolalpha << (a >= b) << '\n'; // false

    auto t = std::make_tuple(1, 2.5, "Hello");
    TuplePrinter<decltype(t)>::print(t); // 1, 2.5, Hello

    return 0;
}
*/