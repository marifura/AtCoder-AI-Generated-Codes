#pragma once
// 出典: https://github.com/marifura/AtCoder-AI-Generated-Codes/blob/main/exported/algorithm/prime.hpp

#include <algorithm>
#include <cstddef>
#include <cstdint>
#include <limits>
#include <stdexcept>
#include <string>
#include <utility>
#include <vector>

// 比較的小さい上限（10^7 程度）に対する素数・素因数分解ライブラリ。
// 構築時に線形篩で最小素因数 (SPF) を前計算する。
//
// 範囲外の n が渡された場合、n 以上の最小の 2 の冪まで自動拡張する。
// 構築・拡張:           O(L) 時間、O(L) メモリ（L は新しい上限）
// is_prime(n):         O(1)（拡張が発生しない場合）
// primes(n):           O(n)
// smallest_factor(n):  O(1)（拡張が発生しない場合）
// prime_factors(n):    O(log n)（拡張が発生しない場合）
// factorize(n):        O(log n)（拡張が発生しない場合）
// divisors(n):         O(d(n) log d(n))（拡張が発生しない場合）
namespace prime {

class Sieve {
public:
    explicit Sieve(int limit) {
        rebuild(limit);
    }

    int limit() const noexcept {
        return static_cast<int>(smallest_factor_.size()) - 1;
    }

    // 0, 1, 負数は false。上限を超える値に対しては篩を自動拡張する。
    bool is_prime(int n) const {
        if (n < 2) return false;
        ensure_capacity(n);
        return smallest_factor_[n] == n;
    }

    // n >= 2 の最小素因数を返す。
    int smallest_factor(int n) const {
        if (n < 2) {
            throw std::invalid_argument("smallest_factor: n must be at least 2");
        }
        ensure_capacity(n);
        return smallest_factor_[n];
    }

    // 素数を昇順で返す。自動拡張が発生すると、以前の参照は無効になる。
    const std::vector<int>& primes() const noexcept {
        return primes_;
    }

    // result[x] が x の素数判定を表す、長さ n + 1 の新しい配列を返す。
    std::vector<bool> primes(int n) const {
        if (n < 0) {
            throw std::invalid_argument("primes: n must be non-negative");
        }
        ensure_capacity(n);

        std::vector<bool> result(static_cast<std::size_t>(n) + 1, false);
        for (int prime_number : primes_) {
            if (prime_number > n) break;
            result[prime_number] = true;
        }
        return result;
    }

    // 素因数を重複込みの昇順で返す。1 に対しては空配列を返す。
    std::vector<int> prime_factors(int n) const {
        check_factorization_input(n, "prime_factors");
        std::vector<int> result;
        while (n > 1) {
            const int factor = smallest_factor_[n];
            result.push_back(factor);
            n /= factor;
        }
        return result;
    }

    // (素因数, 指数) を素因数の昇順で返す。
    std::vector<std::pair<int, int>> factorize(int n) const {
        check_factorization_input(n, "factorize");
        std::vector<std::pair<int, int>> result;
        while (n > 1) {
            const int factor = smallest_factor_[n];
            int exponent = 0;
            do {
                n /= factor;
                ++exponent;
            } while (n > 1 && smallest_factor_[n] == factor);
            result.emplace_back(factor, exponent);
        }
        return result;
    }

    // 正の約数をすべて昇順で返す。1 に対しては {1} を返す。
    std::vector<int> divisors(int n) const {
        check_factorization_input(n, "divisors");
        const auto factors = factorize(n);

        std::size_t divisor_count = 1;
        for (const auto& [factor, exponent] : factors) {
            (void)factor;
            divisor_count *= static_cast<std::size_t>(exponent + 1);
        }

        std::vector<int> result{1};
        result.reserve(divisor_count);
        for (const auto& [factor, exponent] : factors) {
            const std::size_t previous_size = result.size();
            std::int64_t power = 1;
            for (int current_exponent = 1;
                 current_exponent <= exponent;
                 ++current_exponent) {
                power *= factor;
                for (std::size_t i = 0; i < previous_size; ++i) {
                    result.push_back(static_cast<int>(result[i] * power));
                }
            }
        }
        std::sort(result.begin(), result.end());
        return result;
    }

private:
    mutable std::vector<int> smallest_factor_;
    mutable std::vector<int> primes_;

    static std::size_t checked_size(int limit) {
        if (limit < 0) {
            throw std::invalid_argument("Sieve: limit must be non-negative");
        }
        return static_cast<std::size_t>(limit) + 1;
    }

    static int next_power_of_two(int n) {
        std::uint64_t result = 1;
        while (result < static_cast<std::uint64_t>(n)) result <<= 1;
        if (result > static_cast<std::uint64_t>(
                         std::numeric_limits<int>::max())) {
            throw std::length_error(
                "Sieve: the next power of two does not fit in int"
            );
        }
        return static_cast<int>(result);
    }

    void rebuild(int new_limit) const {
        std::vector<int> new_smallest_factor(checked_size(new_limit), 0);
        std::vector<int> new_primes;

        for (std::int64_t number = 2; number <= new_limit; ++number) {
            const auto index = static_cast<std::size_t>(number);
            if (new_smallest_factor[index] == 0) {
                new_smallest_factor[index] = static_cast<int>(number);
                new_primes.push_back(static_cast<int>(number));
            }

            for (int prime_number : new_primes) {
                if (prime_number > new_smallest_factor[index]) break;
                const std::int64_t multiple = number * prime_number;
                if (multiple > new_limit) break;
                new_smallest_factor[static_cast<std::size_t>(multiple)] =
                    prime_number;
            }
        }

        smallest_factor_.swap(new_smallest_factor);
        primes_.swap(new_primes);
    }

    void ensure_capacity(int n) const {
        if (n > limit()) rebuild(next_power_of_two(n));
    }

    void check_factorization_input(int n, const char* function_name) const {
        if (n <= 0) {
            throw std::invalid_argument(
                std::string(function_name) + ": n must be positive"
            );
        }
        if (n == 1) return;
        ensure_capacity(n);
    }
};

}  // namespace prime
