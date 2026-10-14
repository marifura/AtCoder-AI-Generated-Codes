#pragma once
// 出典: https://github.com/marifura/AtCoder-AI-Generated-Codes/blob/main/exported/algorithm/tuple_hash.hpp

#include <cstddef>
#include <cstdint>
#include <functional>
#include <tuple>
#include <type_traits>
#include <utility>

// std::tuple の各要素を std::hash でハッシュ化して結合する。
//
// 使用例:
//   std::unordered_map<std::tuple<int, int>, int, TupleHash> map;
//
// std::hash<std::tuple<...>> を namespace std に追加することは標準規格上
// 認められていないため、unordered_map の第 3 テンプレート引数に指定する。
struct TupleHash {
private:
    // splitmix64 の最終化処理。整数の連番など、std::hash の出力に偏りが
    // あっても上位・下位ビットへ均等に拡散させる。
    static constexpr std::uint64_t mix(std::uint64_t x) noexcept {
        x ^= x >> 30;
        x *= 0xbf58476d1ce4e5b9ULL;
        x ^= x >> 27;
        x *= 0x94d049bb133111ebULL;
        return x ^ (x >> 31);
    }

    template <class Tuple, std::size_t... Indices>
    static constexpr std::size_t hash_impl(
        const Tuple& value, std::index_sequence<Indices...>)
        noexcept((noexcept(std::hash<std::remove_cvref_t<
                      std::tuple_element_t<Indices, Tuple>>>{}(
                      std::get<Indices>(value))) && ...)) {
        // 要素数を初期値へ含め、空 tuple にも定義された値を与える。
        std::uint64_t state = mix(
            0x9e3779b97f4a7c15ULL ^ std::tuple_size_v<Tuple>);

        // state を毎回 avalanche させることで、要素の順序も区別する。
        ((state = mix(state ^
                      (static_cast<std::uint64_t>(
                           std::hash<std::remove_cvref_t<
                               std::tuple_element_t<Indices, Tuple>>>{}(
                               std::get<Indices>(value))) +
                       0x9e3779b97f4a7c15ULL))),
         ...);
        return static_cast<std::size_t>(state);
    }

public:
    template <class... Types>
    constexpr std::size_t operator()(
        const std::tuple<Types...>& value) const
        noexcept(noexcept(hash_impl(value, std::index_sequence_for<Types...>{}))) {
        return hash_impl(value, std::index_sequence_for<Types...>{});
    }
};
