#pragma once

#include <limits>
#include <stdexcept>
#include <string>
#include <tuple>
#include <vector>

// 連続する同じ文字を、文字の配列と各連続区間の長さに分ける。計算量は O(N)。
// 空文字列には空の配列を返す。区間長が int の範囲を超える場合は例外を送出する。
inline std::tuple<std::vector<char>, std::vector<int>> runlength(std::string s) {
    std::vector<char> characters;
    std::vector<int> lengths;
    for (char value : s) {
        if (characters.empty() || characters.back() != value) {
            characters.emplace_back(value);
            lengths.emplace_back(1);
        } else {
            if (lengths.back() == std::numeric_limits<int>::max()) {
                throw std::overflow_error("runlength: 連続区間の長さが int の範囲を超えています");
            }
            ++lengths.back();
        }
    }
    return {characters, lengths};
}
