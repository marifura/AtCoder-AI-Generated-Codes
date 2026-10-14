#pragma once
// 出典: https://github.com/marifura/AtCoder-AI-Generated-Codes/blob/main/exported/algorithm/prime_factorization.hpp

#include <algorithm>
#include <cstddef>
#include <cstdint>
#include <numeric>
#include <stdexcept>
#include <utility>
#include <vector>

// 64 bit 符号なし整数の素数判定・素因数分解。
// Miller--Rabin の底は uint64_t に対して決定的で、素因数分解には
// Pollard's Rho (Brent 法) を用いる。
//
// is_prime(n):       n が素数かを返す。
// prime_factors(n):  素因数を重複込みの昇順で返す。
// factorize(n):      (素因数, 指数) を素因数の昇順で返す。
// divisors(n):       正の約数を昇順で返す。
//
// 1 の素因数列は空とする。0 の素因数分解は定義せず、
// prime_factors(0), factorize(0), divisors(0) は
// std::invalid_argument を送出する。
namespace prime_factorization {

using u64 = std::uint64_t;
using u128 = __uint128_t;

namespace internal {

inline u64 multiply_mod(u64 a, u64 b, u64 mod) {
    return static_cast<u64>(static_cast<u128>(a) * b % mod);
}

inline u64 power_mod(u64 base, u64 exponent, u64 mod) {
    u64 result = 1;
    while (exponent > 0) {
        if (exponent & 1) result = multiply_mod(result, base, mod);
        base = multiply_mod(base, base, mod);
        exponent >>= 1;
    }
    return result;
}

inline u64 splitmix64(u64& state) {
    u64 z = (state += 0x9e3779b97f4a7c15ULL);
    z = (z ^ (z >> 30)) * 0xbf58476d1ce4e5b9ULL;
    z = (z ^ (z >> 27)) * 0x94d049bb133111ebULL;
    return z ^ (z >> 31);
}

}  // namespace internal

inline bool is_prime(u64 n) {
    if (n < 2) return false;
    for (u64 p : {2ULL, 3ULL, 5ULL, 7ULL, 11ULL, 13ULL, 17ULL,
                  19ULL, 23ULL, 29ULL, 31ULL, 37ULL}) {
        if (n % p == 0) return n == p;
    }

    u64 odd_part = n - 1;
    int power_of_two = 0;
    while ((odd_part & 1) == 0) {
        odd_part >>= 1;
        ++power_of_two;
    }

    // この7底で uint64_t 全域に対して決定的に判定できる。
    constexpr u64 bases[] = {
        2ULL, 325ULL, 9375ULL, 28178ULL,
        450775ULL, 9780504ULL, 1795265022ULL,
    };
    for (u64 base : bases) {
        if (base % n == 0) continue;
        u64 value = internal::power_mod(base % n, odd_part, n);
        if (value == 1 || value == n - 1) continue;

        bool probably_prime = false;
        for (int r = 1; r < power_of_two; ++r) {
            value = internal::multiply_mod(value, value, n);
            if (value == n - 1) {
                probably_prime = true;
                break;
            }
        }
        if (!probably_prime) return false;
    }
    return true;
}

namespace internal {

inline u64 pollard_rho(u64 n) {
    if (n % 2 == 0) return 2;
    if (n % 3 == 0) return 3;

    u64 random_state = n ^ 0x243f6a8885a308d3ULL;
    while (true) {
        const u64 initial = splitmix64(random_state) % (n - 1) + 1;
        const u64 constant = splitmix64(random_state) % (n - 1) + 1;
        auto next = [n, constant](u64 value) {
            return static_cast<u64>(
                (static_cast<u128>(multiply_mod(value, value, n)) + constant)
                % n
            );
        };

        u64 y = initial;
        u64 divisor = 1;
        u64 block_size = 1;
        u64 x = 0;
        u64 saved_y = 0;
        constexpr u64 batch_size = 128;

        while (divisor == 1) {
            x = y;
            for (u64 i = 0; i < block_size; ++i) y = next(y);

            u64 offset = 0;
            u64 product = 1;
            while (offset < block_size && divisor == 1) {
                saved_y = y;
                const u64 count = std::min(batch_size, block_size - offset);
                for (u64 i = 0; i < count; ++i) {
                    y = next(y);
                    const u64 difference = x > y ? x - y : y - x;
                    product = multiply_mod(product, difference, n);
                }
                divisor = std::gcd(product, n);
                offset += count;
            }
            block_size <<= 1;
        }

        if (divisor == n) {
            do {
                saved_y = next(saved_y);
                const u64 difference = x > saved_y ? x - saved_y : saved_y - x;
                divisor = std::gcd(difference, n);
            } while (divisor == 1);
        }
        if (divisor != n) return divisor;
    }
}

inline void collect_factors(u64 n, std::vector<u64>& factors) {
    if (n == 1) return;
    if (is_prime(n)) {
        factors.push_back(n);
        return;
    }
    const u64 divisor = pollard_rho(n);
    collect_factors(divisor, factors);
    collect_factors(n / divisor, factors);
}

}  // namespace internal

inline std::vector<u64> prime_factors(u64 n) {
    if (n == 0) {
        throw std::invalid_argument("prime_factors: zero has no prime factorization");
    }
    std::vector<u64> result;
    internal::collect_factors(n, result);
    std::sort(result.begin(), result.end());
    return result;
}

inline std::vector<std::pair<u64, int>> factorize(u64 n) {
    const std::vector<u64> factors = prime_factors(n);
    std::vector<std::pair<u64, int>> result;
    for (u64 prime : factors) {
        if (result.empty() || result.back().first != prime) {
            result.emplace_back(prime, 1);
        } else {
            ++result.back().second;
        }
    }
    return result;
}

// 正の約数をすべて昇順で返す。1 に対しては {1} を返す。
inline std::vector<u64> divisors(u64 n) {
    if (n == 0) {
        throw std::invalid_argument("divisors: zero has no positive divisors");
    }
    const auto factors = factorize(n);

    std::size_t divisor_count = 1;
    for (const auto& [prime, exponent] : factors) {
        (void)prime;
        divisor_count *= static_cast<std::size_t>(exponent + 1);
    }

    std::vector<u64> result{1};
    result.reserve(divisor_count);
    for (const auto& [prime, exponent] : factors) {
        const std::size_t previous_size = result.size();
        u64 power = 1;
        for (int current_exponent = 1;
             current_exponent <= exponent;
             ++current_exponent) {
            power *= prime;
            for (std::size_t i = 0; i < previous_size; ++i) {
                result.push_back(result[i] * power);
            }
        }
    }
    std::sort(result.begin(), result.end());
    return result;
}

}  // namespace prime_factorization
