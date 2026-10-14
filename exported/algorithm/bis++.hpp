#pragma once
// 出典: https://github.com/marifura/AtCoder-AI-Generated-Codes/blob/main/exported/algorithm/bis%2B%2B.hpp

#include <cmath>
#include <stdexcept>

// predicate(ok) == true, predicate(ng) == false を満たす境界を探索する。
template <class F>
long long bispp(long long ok, long long ng, F predicate) {
    auto distance = [](long long a, long long b) {
        using ull = unsigned long long;
        return a >= b ? static_cast<ull>(a) - static_cast<ull>(b)
                      : static_cast<ull>(b) - static_cast<ull>(a);
    };

    while (distance(ok, ng) > 1) {
        const auto half = distance(ok, ng) / 2;
        const long long mid = ok < ng
                                  ? ok + static_cast<long long>(half)
                                  : ok - static_cast<long long>(half);
        if (predicate(mid)) ok = mid;
        else ng = mid;
    }
    return ok;
}

template <class F>
double bisfpp(double ok, double ng, double absolute_error, F predicate) {
    if (!(absolute_error > 0)) {
        throw std::invalid_argument("bisfpp: absolute_error must be positive");
    }
    while (std::abs(ok - ng) > absolute_error) {
        const double mid = ok / 2 + ng / 2;
        if (mid == ok || mid == ng) break;
        if (predicate(mid)) ok = mid;
        else ng = mid;
    }
    return ok;
}

inline constexpr int ITERTIMES = 60;

// 単峰な目的関数について、閉区間 [left, right] 内の最適な整数を探索する。
// predicate(a, b) は a を b より優先するとき true を返す。
template <class F>
long long trspp(long long left, long long right, F predicate) {
    if (left > right) {
        throw std::invalid_argument("trspp: left must not exceed right");
    }

    using ull = unsigned long long;
    auto distance = [](long long a, long long b) {
        return static_cast<ull>(b) - static_cast<ull>(a);
    };

    while (distance(left, right) > 2) {
        const auto third = distance(left, right) / 3;
        const long long mid1 = left + static_cast<long long>(third);
        const long long mid2 = right - static_cast<long long>(third);
        if (predicate(mid1, mid2)) right = mid2;
        else left = mid1;
    }

    long long result = left;
    for (long long candidate = left; candidate != right;) {
        ++candidate;
        if (predicate(candidate, result)) result = candidate;
    }
    return result;
}

template <class F>
double trsfpp(double left, double right, F predicate) {
    for (int i = 0; i < ITERTIMES; ++i) {
        const double mid1 = (2 * left + right) / 3;
        const double mid2 = (left + 2 * right) / 3;
        if (predicate(mid1, mid2)) right = mid2;
        else left = mid1;
    }
    return left;
}
