#pragma once
// 出典: https://github.com/marifura/AtCoder-AI-Generated-Codes/blob/main/exported/algorithm/rolling_hash.hpp

#include <algorithm>
#include <cassert>
#include <chrono>
#include <cstdint>
#include <limits>
#include <random>
#include <string>
#include <vector>

// 文字列の部分文字列を O(1) で比較するためのローリングハッシュ。
// get(l, r) は s[l, r) のハッシュ値と長さを返す。
class RollingHash {
public:
    using ull = unsigned long long;

    struct HashValue {
        ull value = 0;
        int length = 0;

        bool operator==(const HashValue &) const = default;
    };

private:
    // 2^61 - 1 は素数。積の計算には 128 bit 整数を使う。
    static constexpr ull MOD = (1ULL << 61) - 1;
    std::vector<ull> hash_value;
    std::vector<ull> reverse_hash_value;
    mutable std::vector<ull> power;

    static ull base() {
        // 実行ごとに BASE を変え、固定 BASE を狙った衝突ケースを防ぐ。
        static const ull value = [] {
            ull seed = std::chrono::steady_clock::now()
                           .time_since_epoch()
                           .count();
            seed ^= reinterpret_cast<std::uintptr_t>(&seed);
            std::mt19937_64 rng(seed);
            std::uniform_int_distribution<ull> dist(256, MOD - 2);
            return dist(rng);
        }();
        return value;
    }

    static ull mod_add(ull a, ull b) {
        ull res = a + b;
        if (res >= MOD) res -= MOD;
        return res;
    }

    static ull mod_sub(ull a, ull b) {
        return a >= b ? a - b : a + MOD - b;
    }

    static ull mod_mul(ull a, ull b) {
        return static_cast<ull>((__uint128_t)a * b % MOD);
    }

    void ensure_power(int length) const {
        assert(0 <= length);
        if (power.empty()) power.push_back(1);
        while (static_cast<int>(power.size()) <= length) {
            power.push_back(mod_mul(power.back(), base()));
        }
    }

public:
    ull get_hash(int l, int r) const {
        assert(0 <= l && l <= r && r <= size());
        return mod_sub(hash_value[r],
                       mod_mul(hash_value[l], power[r - l]));
    }

    ull get_reverse_hash(int l, int r) const {
        assert(0 <= l && l <= r && r <= size());
        int reverse_l = size() - r;
        int reverse_r = size() - l;
        return mod_sub(reverse_hash_value[reverse_r],
                       mod_mul(reverse_hash_value[reverse_l],
                               power[reverse_r - reverse_l]));
    }

    RollingHash() {
        build("");
    }

    explicit RollingHash(const std::string &s) {
        build(s);
    }

    void build(const std::string &s) {
        const int n = static_cast<int>(s.size());
        hash_value.assign(n + 1, 0);
        reverse_hash_value.assign(n + 1, 0);
        power.assign(n + 1, 1);

        for (int i = 0; i < n; ++i) {
            // unsigned char に変換し、0 と区別するため 1 を足す。
            ull c = static_cast<unsigned char>(s[i]) + 1;
            ull reverse_c = static_cast<unsigned char>(s[n - 1 - i]) + 1;
            hash_value[i + 1] =
                mod_add(mod_mul(hash_value[i], base()), c);
            reverse_hash_value[i + 1] =
                mod_add(mod_mul(reverse_hash_value[i], base()), reverse_c);
            power[i + 1] = mod_mul(power[i], base());
        }
    }

    int size() const {
        return static_cast<int>(hash_value.size()) - 1;
    }

    // s[l, r) のハッシュ値と長さを返す。
    HashValue get(int l, int r) const {
        return {get_hash(l, r), r - l};
    }

    // reverse(s[l, r)) のハッシュ値と長さを返す。
    HashValue get_reverse(int l, int r) const {
        return {get_reverse_hash(l, r), r - l};
    }

    // s[l, r) が回文か判定する。
    bool is_palindrome(int l, int r) const {
        return get(l, r) == get_reverse(l, r);
    }

    // s[l, r) を無視したとき、左右が鏡写しになる最大半径を返す。
    // 戻り値を k とすると、
    // reverse(s[l - k, l)) == s[r, r + k) が成立する。
    int mirrored_radius(int l, int r) const {
        assert(0 <= l && l <= r && r <= size());

        int ok = 0;
        int ng = std::min(l, size() - r) + 1;
        while (ng - ok > 1) {
            int mid = (ok + ng) / 2;
            if (get_reverse(l - mid, l) == get(r, r + mid)) {
                ok = mid;
            } else {
                ng = mid;
            }
        }
        return ok;
    }

    // 2つのハッシュを left + right の順に連結する。
    ull connect(ull left_hash, ull right_hash, int right_length) const {
        ensure_power(right_length);
        return mod_add(mod_mul(left_hash, power[right_length]), right_hash);
    }

    // 2つのHashValueを left + right の順に連結する。
    HashValue connect(HashValue left, HashValue right) const {
        assert(0 <= left.length && 0 <= right.length);
        assert(left.length <= std::numeric_limits<int>::max() - right.length);
        return {
            connect(left.value, right.value, right.length),
            left.length + right.length
        };
    }
    // s[l1, r1) と s[l2, r2) が一致するか判定する。
    bool equal(int l1, int r1, int l2, int r2) const {
        if (r1 - l1 != r2 - l2) return false;
        return get(l1, r1) == get(l2, r2);
    }

    // s[i, ...) と s[j, ...) の最長共通接頭辞の長さを返す。
    // 上限を limit に制限できる。計算量は O(log N)。
    int lcp(int i, int j, int limit) const {
        assert(0 <= i && i <= size());
        assert(0 <= j && j <= size());
        limit = std::min({limit, size() - i, size() - j});

        int ok = 0;
        int ng = limit + 1;
        while (ng - ok > 1) {
            int mid = (ok + ng) / 2;
            if (get(i, i + mid) == get(j, j + mid)) {
                ok = mid;
            } else {
                ng = mid;
            }
        }
        return ok;
    }

    int lcp(int i, int j) const {
        return lcp(i, j, std::min(size() - i, size() - j));
    }

    // この文字列の s[i, ...) と other の文字列の t[j, ...) の
    // 最長共通接頭辞を返す。計算量は O(log N)。
    int lcp(int i, const RollingHash &other, int j, int limit) const {
        assert(0 <= i && i <= size());
        assert(0 <= j && j <= other.size());
        assert(0 <= limit);
        limit = std::min({limit, size() - i, other.size() - j});

        int ok = 0;
        int ng = limit + 1;
        while (ng - ok > 1) {
            int mid = (ok + ng) / 2;
            if (get(i, i + mid) == other.get(j, j + mid)) {
                ok = mid;
            } else {
                ng = mid;
            }
        }
        return ok;
    }

    int lcp(int i, const RollingHash &other, int j) const {
        return lcp(i, other, j, std::min(size() - i, other.size() - j));
    }
};

/*
使用例:

int main() {
    string s = "abcabc";
    RollingHash rh(s);

    cout << rh.equal(0, 3, 3, 6) << '\n'; // 1 ("abc" == "abc")
    cout << rh.get(1, 4).value << '\n';   // s[1, 4) = "bca"
    cout << rh.lcp(0, 3) << '\n';         // 3
}

注意:
ローリングハッシュには、ごく低い確率で異なる文字列のハッシュ値が
一致する「衝突」がある。
*/
