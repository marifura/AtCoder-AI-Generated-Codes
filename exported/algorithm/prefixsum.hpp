#pragma once
// 出典: https://github.com/marifura/AtCoder-AI-Generated-Codes/blob/main/exported/algorithm/prefixsum.hpp

#include <cstddef>
#include <stdexcept>
#include <vector>

// v[0], ..., v[i - 1] の累積和を result[i] に格納する。
// T は T{} が加法単位元で、加算が定義されている型を想定する。
template <class T>
std::vector<T> prefix_sum(const std::vector<T>& v) {
    std::vector<T> result(v.size() + 1, T{});
    for (std::size_t i = 0; i < v.size(); ++i) {
        result[i + 1] = result[i] + v[i];
    }
    return result;
}

// v[i], ..., v[n - 1] の累積和を result[i] に格納する。
template <class T>
std::vector<T> suffix_sum(const std::vector<T>& v) {
    std::vector<T> result(v.size() + 1, T{});
    for (std::size_t i = v.size(); i > 0; --i) {
        result[i - 1] = result[i] + v[i - 1];
    }
    return result;
}

template <class T>
std::vector<T> prefix_mul(const std::vector<T>& v) {
    std::vector<T> result(v.size() + 1, T{1});
    for (std::size_t i = 0; i < v.size(); ++i) {
        result[i + 1] = result[i] * v[i];
    }
    return result;
}

template <class T>
std::vector<T> suffix_mul(const std::vector<T>& v) {
    std::vector<T> result(v.size() + 1, T{1});
    for (std::size_t i = v.size(); i > 0; --i) {
        result[i - 1] = result[i] * v[i - 1];
    }
    return result;
}

// result[i][j] は v の半開矩形 [0, i) x [0, j) の総和。
template <class T>
std::vector<std::vector<T>> prefix_sum_2d(
    const std::vector<std::vector<T>>& v) {
    const std::size_t width = v.empty() ? 0 : v.front().size();
    for (const auto& row : v) {
        if (row.size() != width) {
            throw std::invalid_argument("prefix_sum_2d: matrix must be rectangular");
        }
    }

    std::vector<std::vector<T>> result(
        v.size() + 1, std::vector<T>(width + 1, T{}));
    for (std::size_t i = 0; i < v.size(); ++i) {
        for (std::size_t j = 0; j < width; ++j) {
            result[i + 1][j + 1] = result[i][j + 1] + result[i + 1][j]
                                      - result[i][j] + v[i][j];
        }
    }
    return result;
}

// result[i][j] は v の半開矩形 [0, i) x [0, j) の総積。
template <class T>
std::vector<std::vector<T>> prefix_mul_2d(
    const std::vector<std::vector<T>>& v) {
    const std::size_t width = v.empty() ? 0 : v.front().size();
    for (const auto& row : v) {
        if (row.size() != width) {
            throw std::invalid_argument("prefix_mul_2d: matrix must be rectangular");
        }
    }

    std::vector<std::vector<T>> result(
        v.size() + 1, std::vector<T>(width + 1, T{1}));
    for (std::size_t i = 0; i < v.size(); ++i) {
        T row_product{1};
        for (std::size_t j = 0; j < width; ++j) {
            row_product = row_product * v[i][j];
            result[i + 1][j + 1] = result[i][j + 1] * row_product;
        }
    }
    return result;
}
