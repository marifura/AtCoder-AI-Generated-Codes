#pragma once
// 出典: https://github.com/marifura/AtCoder-AI-Generated-Codes/blob/main/exported/algorithm/coordinate_compression.hpp

#include <algorithm>
#include <cstddef>
#include <limits>
#include <stdexcept>
#include <vector>

// 各要素を、昇順に並べたときの 0-indexed の順位へ変換する。
// 同じ値には同じ順位を割り当てる。計算量は O(N log N)。
template <class T>
std::vector<int> coordinate_compress(const std::vector<T>& values) {
    std::vector<T> sorted_values = values;
    std::sort(sorted_values.begin(), sorted_values.end());
    sorted_values.erase(
        std::unique(sorted_values.begin(), sorted_values.end()),
        sorted_values.end());

    if (sorted_values.size()
        > static_cast<std::size_t>(std::numeric_limits<int>::max())) {
        throw std::overflow_error(
            "coordinate_compress: too many distinct values for int indices");
    }

    std::vector<int> result;
    result.reserve(values.size());
    for (const T& value : values) {
        result.push_back(static_cast<int>(
            std::lower_bound(sorted_values.begin(), sorted_values.end(), value)
            - sorted_values.begin()));
    }
    return result;
}

// 提出用断片と同じ名前・戻り値の型で座標圧縮を行う。
inline std::vector<long long> ccprsn(const std::vector<long long>& values) {
    std::vector<long long> sorted_values = values;
    std::sort(sorted_values.begin(), sorted_values.end());
    sorted_values.erase(
        std::unique(sorted_values.begin(), sorted_values.end()),
        sorted_values.end());

    std::vector<long long> result;
    result.reserve(values.size());
    for (long long value : values) {
        result.push_back(static_cast<long long>(
            std::lower_bound(sorted_values.begin(), sorted_values.end(), value)
            - sorted_values.begin()));
    }
    return result;
}

inline std::vector<int> ccprsn_int(const std::vector<int>& values) {
    return coordinate_compress(values);
}
