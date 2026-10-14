#include <algorithm>
#include <cstddef>
#include <utility>
#include <vector>

// 座標圧縮された数列の狭義単調増加部分列を求め、最大長と列の値を返す。
// 同じ値は続けて採用しない。最長の列が複数ある場合は、そのうち一つを返す。
// 空入力には {0, 空の列} を返す。時間 O(N log N)、空間 O(N)。
template <class T>
std::pair<std::size_t, std::vector<T>> lis(const std::vector<T>& values) {
    const std::size_t n = values.size();
    std::vector<T> tails;
    std::vector<std::size_t> tail_indices;
    std::vector<std::size_t> previous(n, n);

    for (std::size_t i = 0; i < n; ++i) {
        const std::size_t position =
            std::lower_bound(tails.begin(), tails.end(), values[i]) - tails.begin();
        if (position > 0) previous[i] = tail_indices[position - 1];

        if (position == tails.size()) {
            tails.push_back(values[i]);
            tail_indices.push_back(i);
        } else {
            tails[position] = values[i];
            tail_indices[position] = i;
        }
    }

    std::vector<T> sequence;
    if (!tail_indices.empty()) {
        for (std::size_t i = tail_indices.back(); i != n; i = previous[i]) {
            sequence.push_back(values[i]);
        }
        std::reverse(sequence.begin(), sequence.end());
    }
    return {sequence.size(), std::move(sequence)};
}

// 使用例：
// std::vector<int> a{2, 0, 1, 1, 3};
// auto [length, sequence] = lis(a);
// length は 3、sequence は {0, 1, 3}。

// 座標圧縮された数列の広義単調増加部分列を求め、最大長と列の値を返す。
// 同じ値も続けて採用する。最長の列が複数ある場合は、そのうち一つを返す。
// 空入力には {0, 空の列} を返す。時間 O(N log N)、空間 O(N)。
template <class T>
std::pair<std::size_t, std::vector<T>> lnds(const std::vector<T>& values) {
    const std::size_t n = values.size();
    std::vector<T> tails;
    std::vector<std::size_t> tail_indices;
    std::vector<std::size_t> previous(n, n);

    for (std::size_t i = 0; i < n; ++i) {
        const std::size_t position =
            std::upper_bound(tails.begin(), tails.end(), values[i]) - tails.begin();
        if (position > 0) previous[i] = tail_indices[position - 1];

        if (position == tails.size()) {
            tails.push_back(values[i]);
            tail_indices.push_back(i);
        } else {
            tails[position] = values[i];
            tail_indices[position] = i;
        }
    }

    std::vector<T> sequence;
    if (!tail_indices.empty()) {
        for (std::size_t i = tail_indices.back(); i != n; i = previous[i]) {
            sequence.push_back(values[i]);
        }
        std::reverse(sequence.begin(), sequence.end());
    }
    return {sequence.size(), std::move(sequence)};
}

// 使用例：
// std::vector<int> a{2, 0, 1, 1, 3};
// auto [length, sequence] = lnds(a);
// length は 4、sequence は {0, 1, 1, 3}。
