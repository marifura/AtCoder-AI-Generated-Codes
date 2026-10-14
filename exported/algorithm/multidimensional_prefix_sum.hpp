#pragma once

#include <array>
#include <cstddef>
#include <concepts>
#include <initializer_list>
#include <iterator>
#include <limits>
#include <ranges>
#include <stdexcept>
#include <type_traits>
#include <utility>
#include <vector>

// D 次元累積和。
//
// 添字は 0-indexed、sum の区間は各次元について半開区間 [lower, upper)。
// add/set で値を設定した後、sum を呼ぶ前に build() を一度呼ぶ。
//
// 構築: O(D * Π(size[d] + 1))
// 区間和: O(2^D)
// メモリ: O(Π(size[d] + 1))
template <class T, std::size_t D>
class MultiDimensionalPrefixSum {
    static_assert(D > 0, "the number of dimensions must be positive");

public:
    using Index = std::array<std::size_t, D>;

private:
    Index sizes_{};
    Index padded_sizes_{};
    Index strides_{};
    std::vector<T> data_;
    bool built_ = false;

    template <class R>
    static constexpr bool is_index_range = [] {
        if constexpr (!std::ranges::input_range<const R&>) {
            return false;
        } else {
            return std::integral<
                std::remove_cvref_t<std::ranges::range_reference_t<const R&>>
            >;
        }
    }();

    template <class R>
        requires is_index_range<R>
    static Index to_index(const R& coordinates) {
        Index result{};
        std::size_t dimension = 0;
        for (const auto coordinate : coordinates) {
            if (dimension == D) {
                throw std::invalid_argument(
                    "the number of coordinates does not match D"
                );
            }
            if (!std::in_range<std::size_t>(coordinate)) {
                throw std::out_of_range("coordinate is out of range");
            }
            result[dimension++] = static_cast<std::size_t>(coordinate);
        }
        if (dimension != D) {
            throw std::invalid_argument(
                "the number of coordinates does not match D"
            );
        }
        return result;
    }

    static std::size_t checked_multiply(std::size_t a, std::size_t b) {
        if (a != 0 && b > std::numeric_limits<std::size_t>::max() / a) {
            throw std::length_error("multidimensional prefix sum is too large");
        }
        return a * b;
    }

    void initialize_storage(const Index& sizes) {
        sizes_ = sizes;
        std::size_t total_size = 1;
        for (std::size_t d = D; d-- > 0;) {
            if (sizes_[d] == std::numeric_limits<std::size_t>::max()) {
                throw std::length_error("dimension size is too large");
            }
            padded_sizes_[d] = sizes_[d] + 1;
            strides_[d] = total_size;
            total_size = checked_multiply(total_size, padded_sizes_[d]);
        }
        data_.assign(total_size, T{});
    }

    template <std::size_t Depth, class U>
    static void infer_sizes(
        const U& values,
        Index& sizes,
        std::array<bool, D>& determined
    ) {
        if constexpr (Depth == D) {
            static_assert(
                std::is_convertible_v<const U&, T>,
                "array depth does not match D, or its element type "
                "is not convertible to T"
            );
        } else {
            using std::size;
            const std::size_t current_size = size(values);
            if (determined[Depth] && sizes[Depth] != current_size) {
                throw std::invalid_argument(
                    "multidimensional array must be rectangular"
                );
            }
            sizes[Depth] = current_size;
            determined[Depth] = true;
            for (const auto& child : values) {
                infer_sizes<Depth + 1>(child, sizes, determined);
            }
        }
    }

    template <std::size_t Depth, class U>
    void copy_values(const U& values, Index& index) {
        if constexpr (Depth == D) {
            Index padded_index{};
            for (std::size_t d = 0; d < D; ++d) {
                padded_index[d] = index[d] + 1;
            }
            data_[offset(padded_index)] = static_cast<T>(values);
        } else {
            std::size_t coordinate = 0;
            for (const auto& child : values) {
                index[Depth] = coordinate++;
                copy_values<Depth + 1>(child, index);
            }
        }
    }

    void check_index(const Index& index) const {
        for (std::size_t d = 0; d < D; ++d) {
            if (index[d] >= sizes_[d]) {
                throw std::out_of_range("index is out of range");
            }
        }
    }

    void check_bound(const Index& bound) const {
        for (std::size_t d = 0; d < D; ++d) {
            if (bound[d] > sizes_[d]) {
                throw std::out_of_range("bound is out of range");
            }
        }
    }

    std::size_t offset(const Index& index) const {
        std::size_t result = 0;
        for (std::size_t d = 0; d < D; ++d) {
            result += index[d] * strides_[d];
        }
        return result;
    }

    T corner_sum(
        std::size_t dimension,
        std::size_t current_offset,
        std::size_t lower_count,
        const Index& lower,
        const Index& upper
    ) const {
        if (dimension == D) {
            if (lower_count % 2 == 0) return data_[current_offset];
            return T{} - data_[current_offset];
        }

        return corner_sum(
                   dimension + 1,
                   current_offset + upper[dimension] * strides_[dimension],
                   lower_count,
                   lower,
                   upper
               )
             + corner_sum(
                   dimension + 1,
                   current_offset + lower[dimension] * strides_[dimension],
                   lower_count + 1,
                   lower,
                   upper
               );
    }

public:
    explicit MultiDimensionalPrefixSum(const Index& sizes)
    {
        initialize_storage(sizes);
    }

    // int などの変数を使った {height, width} も受け付ける。
    template <std::integral I>
    explicit MultiDimensionalPrefixSum(std::initializer_list<I> sizes) {
        initialize_storage(to_index(sizes));
    }

    template <std::integral I>
    explicit MultiDimensionalPrefixSum(const std::array<I, D>& sizes) {
        initialize_storage(to_index(sizes));
    }

    // D 重に入れ子になった vector、array、C 配列から構築する。
    // 配列は長方形である必要がある。コンストラクタ内で build() まで行う。
    template <class U>
        requires (!std::is_same_v<std::remove_cvref_t<U>, Index>)
    explicit MultiDimensionalPrefixSum(const U& values) {
        Index sizes{};
        std::array<bool, D> determined{};
        infer_sizes<0>(values, sizes, determined);
        initialize_storage(sizes);

        Index index{};
        copy_values<0>(values, index);
        build();
    }


    const Index& sizes() const noexcept {
        return sizes_;
    }

    bool built() const noexcept {
        return built_;
    }

    // build() 前に、index の値へ value を加える。
    void add(const Index& index, const T& value) {
        if (built_) {
            throw std::logic_error("cannot add after build");
        }
        check_index(index);

        Index padded_index{};
        for (std::size_t d = 0; d < D; ++d) {
            padded_index[d] = index[d] + 1;
        }
        data_[offset(padded_index)] += value;
    }

    template <class R>
        requires is_index_range<R>
    void add(const R& index, const T& value) {
        add(to_index(index), value);
    }

    template <std::integral I>
    void add(std::initializer_list<I> index, const T& value) {
        add(to_index(index), value);
    }

    // build() 前に、index の値を value にする。
    void set(const Index& index, const T& value) {
        if (built_) {
            throw std::logic_error("cannot set after build");
        }
        check_index(index);

        Index padded_index{};
        for (std::size_t d = 0; d < D; ++d) {
            padded_index[d] = index[d] + 1;
        }
        data_[offset(padded_index)] = value;
    }

    template <class R>
        requires is_index_range<R>
    void set(const R& index, const T& value) {
        set(to_index(index), value);
    }

    template <std::integral I>
    void set(std::initializer_list<I> index, const T& value) {
        set(to_index(index), value);
    }

    void build() {
        if (built_) {
            throw std::logic_error("build may only be called once");
        }

        for (std::size_t d = 0; d < D; ++d) {
            const std::size_t step = strides_[d];
            const std::size_t block = checked_multiply(step, padded_sizes_[d]);
            for (std::size_t block_begin = 0;
                 block_begin < data_.size();
                 block_begin += block) {
                for (std::size_t rest = 0; rest < step; ++rest) {
                    for (std::size_t i = 1; i < padded_sizes_[d]; ++i) {
                        const std::size_t position =
                            block_begin + i * step + rest;
                        data_[position] += data_[position - step];
                    }
                }
            }
        }
        built_ = true;
    }

    // 各次元について [lower[d], upper[d]) の範囲にある値の総和。
    T sum(const Index& lower, const Index& upper) const {
        if (!built_) {
            throw std::logic_error("build must be called before sum");
        }
        check_bound(lower);
        check_bound(upper);
        for (std::size_t d = 0; d < D; ++d) {
            if (lower[d] > upper[d]) {
                throw std::invalid_argument("lower bound exceeds upper bound");
            }
        }
        return corner_sum(0, 0, 0, lower, upper);
    }

    template <class Lower, class Upper>
        requires (is_index_range<Lower> && is_index_range<Upper>)
    T sum(const Lower& lower, const Upper& upper) const {
        return sum(to_index(lower), to_index(upper));
    }

    template <std::integral Lower, std::integral Upper>
    T sum(
        std::initializer_list<Lower> lower,
        std::initializer_list<Upper> upper
    ) const {
        return sum(to_index(lower), to_index(upper));
    }

    template <std::integral Lower, class Upper>
        requires is_index_range<Upper>
    T sum(std::initializer_list<Lower> lower, const Upper& upper) const {
        return sum(to_index(lower), to_index(upper));
    }

    template <class Lower, std::integral Upper>
        requires is_index_range<Lower>
    T sum(const Lower& lower, std::initializer_list<Upper> upper) const {
        return sum(to_index(lower), to_index(upper));
    }

    // 原点から upper 未満まで、すなわち [0, upper) の総和。
    T prefix_sum(const Index& upper) const {
        return sum(Index{}, upper);
    }

    template <class R>
        requires is_index_range<R>
    T prefix_sum(const R& upper) const {
        return prefix_sum(to_index(upper));
    }

    template <std::integral I>
    T prefix_sum(std::initializer_list<I> upper) const {
        return prefix_sum(to_index(upper));
    }
};
