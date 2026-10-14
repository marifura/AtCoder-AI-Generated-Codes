#pragma once
// 出典: https://github.com/marifura/AtCoder-AI-Generated-Codes/blob/main/exported/algorithm/template.hpp

#include <atcoder/all>
#include <bits/stdc++.h>

#include "prefixsum.hpp"
#include "runlength.hpp"
#include "coordinate_compression.hpp"
#include "bis++.hpp"

#define OVERLOAD_REP(_1, _2, _3, name, ...) name
#define REP1(i, n) for (auto i = std::decay_t<decltype(n)>{}; (i) < (n); ++(i))
#define REP2(i, l, r) for (auto i = (l); (i) < (r); ++(i))
#define rep(...) OVERLOAD_REP(__VA_ARGS__, REP2, REP1)(__VA_ARGS__)
#define RREP1(i, n) for (auto i = (n); (i)-- > decltype(i){};)
#define RREP2(i, l, r) for (auto i = (r); (i)-- > (l);)
#define rrep(...) OVERLOAD_REP(__VA_ARGS__, RREP2, RREP1)(__VA_ARGS__)
#define all(...) std::begin(__VA_ARGS__), std::end(__VA_ARGS__)
#define rall(...) std::rbegin(__VA_ARGS__), std::rend(__VA_ARGS__)
#define YES std::cout << "Yes\n"
#define NO std::cout << "No\n"
#define YN { YES; } else { NO; }

// 変数を宣言して入力する。ifやforの本体で使う場合は波括弧で囲む。
#define LL(...) ll __VA_ARGS__; ::cp_template_input(__VA_ARGS__)
#define STRING(...) string __VA_ARGS__; ::cp_template_input(__VA_ARGS__)

// nは非負とし、要素型は入力可能な型（boolを除く）を指定する。
#define VEC(T, name, n) \
    vector<T> name(n); \
    for (auto& input_value_ : name) cin >> input_value_

#define VL(name, n) VEC(ll, name, n)

using namespace std;
using namespace atcoder;

template <class T>
ostream& operator<<(ostream& os, const vector<T>& values);

template <class T>
ostream& operator<<(ostream& os, const vector<vector<T>>& values);

template <class T1, class T2>
istream& operator>>(istream& is, pair<T1, T2>& value) {
    return is >> value.first >> value.second;
}

template <class T>
istream& operator>>(istream& is, vector<T>& values) {
    for (auto& value : values) is >> value;
    return is;
}

template <class T>
ostream& operator<<(ostream& os, const vector<T>& values) {
    for (const auto& value : values) os << value << ' ';
    return os;
}

template <class T>
ostream& operator<<(ostream& os, const vector<vector<T>>& values) {
    for (const auto& row : values) os << row << '\n';
    return os;
}

template <int m>
ostream& operator<<(ostream& os, const static_modint<m>& value) {
    return os << value.val();
}

inline ostream& operator<<(ostream& os, const modint& value) {
    return os << value.val();
}

template <int m>
istream& operator>>(istream& is, static_modint<m>& value) {
    long long input;
    is >> input;
    value = input;
    return is;
}

inline istream& operator>>(istream& is, modint& value) {
    long long input;
    is >> input;
    value = input;
    return is;
}

template <class... Ts>
void cp_template_input(Ts&... xs) {
    (cin >> ... >> xs);
}

template <typename T>
inline bool chmax(T& a, T b) {
    return a < b ? (a = b, true) : false;
}

template <typename T>
inline bool chmin(T& a, T b) {
    return a > b ? (a = b, true) : false;
}

template <class T, size_t n, size_t idx = 0>
auto make_vec(const size_t (&dimensions)[n], const T& initial_value) noexcept {
    if constexpr (idx < n) {
        return vector(dimensions[idx], make_vec<T, n, idx + 1>(dimensions, initial_value));
    } else {
        return initial_value;
    }
}

template <class T, size_t n>
auto make_vec(const size_t (&dimensions)[n]) noexcept {
    return make_vec(dimensions, T{});
}

struct md_max_fn {
    template <class T>
        requires(!ranges::range<T>)
    constexpr remove_cvref_t<T> operator()(T&& value) const {
        return forward<T>(value);
    }

    template <ranges::range R>
    constexpr auto operator()(R&& values) const {
        auto it = ranges::begin(values);
        auto result = (*this)(*it);
        for (++it; it != ranges::end(values); ++it) result = max(result, (*this)(*it));
        return result;
    }

    template <class T>
    constexpr auto operator()(initializer_list<T> values) const {
        auto it = values.begin();
        auto result = (*this)(*it);
        for (++it; it != values.end(); ++it) result = max(result, (*this)(*it));
        return result;
    }
};

struct md_min_fn {
    template <class T>
        requires(!ranges::range<T>)
    constexpr remove_cvref_t<T> operator()(T&& value) const {
        return forward<T>(value);
    }

    template <ranges::range R>
    constexpr auto operator()(R&& values) const {
        auto it = ranges::begin(values);
        auto result = (*this)(*it);
        for (++it; it != ranges::end(values); ++it) result = min(result, (*this)(*it));
        return result;
    }

    template <class T>
    constexpr auto operator()(initializer_list<T> values) const {
        auto it = values.begin();
        auto result = (*this)(*it);
        for (++it; it != values.end(); ++it) result = min(result, (*this)(*it));
        return result;
    }
};

struct md_sum_fn {
    template <class T>
        requires(!ranges::range<T>)
    constexpr remove_cvref_t<T> operator()(T&& value) const {
        return forward<T>(value);
    }

    template <ranges::range R>
    constexpr auto operator()(R&& values) const {
        using result_type = remove_cvref_t<decltype((*this)(*ranges::begin(values)))>;
        result_type result{};
        for (auto&& value : values) result += (*this)(value);
        return result;
    }

    template <class T>
    constexpr auto operator()(initializer_list<T> values) const {
        using result_type = remove_cvref_t<decltype((*this)(*values.begin()))>;
        result_type result{};
        for (const auto& value : values) result += (*this)(value);
        return result;
    }
};

template<class T>
struct SortResult {
    vector<T> sorted;

    // order[ソート後の添字] = ソート前の添字
    vector<int> order;

    // rank[ソート前の添字] = ソート後の添字
    vector<int> rank;
};

template<class T, class Compare = less<T>>
SortResult<T> sort_with_index(
    const vector<T>& values,
    Compare comp = Compare{}
) {
    const int n = values.size();

    vector<int> order(n);
    iota(order.begin(), order.end(), 0);

    // 同値要素については元の順序を維持する。
    stable_sort(order.begin(), order.end(),
                [&](int lhs, int rhs) {
                    return comp(values[lhs], values[rhs]);
                });

    vector<T> sorted;
    sorted.reserve(n);

    vector<int> rank(n);

    for (int sorted_index = 0; sorted_index < n; ++sorted_index) {
        const int original_index = order[sorted_index];

        sorted.push_back(values[original_index]);
        rank[original_index] = sorted_index;
    }

    return {std::move(sorted), std::move(order), std::move(rank)};
}

inline constexpr md_max_fn md_max;
inline constexpr md_min_fn md_min;
inline constexpr md_sum_fn md_sum;

using ll = long long;
constexpr ll INF = numeric_limits<ll>::max() / 4;
constexpr ll MAX = numeric_limits<ll>::max() / 4;
using vl = vector<ll>;
using vvl = vector<vector<ll>>;
using vvvl = vector<vector<vector<ll>>>;
using vp = vector<pair<ll, ll>>;
using vvp = vector<vector<pair<ll, ll>>>;
template<class T>
using priority_queue_min = priority_queue<T, vector<T>, greater<T>>;
