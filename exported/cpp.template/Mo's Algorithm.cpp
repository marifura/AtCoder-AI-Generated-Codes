// Query は ll l, r を持つものとする。
// 元のクエリ番号が必要なら Query に id を持たせておく。

#include <bits/stdc++.h>

#define ll long long
using namespace std;

struct Query {
    ll l;
    ll r;
    auto operator<=>(const Query&) const = default;
    bool operator==(const Query&) const = default;
};
using vq = vector<Query>;

void Mo(vq& qs) {
    const int Q = qs.size();
    if (Q == 0) return;

    ll min_coordinate = min(qs[0].l, qs[0].r);
    ll max_coordinate = max(qs[0].l, qs[0].r);
    for (const Query& q : qs) {
        min_coordinate = min(min_coordinate, min(q.l, q.r));
        max_coordinate = max(max_coordinate, max(q.l, q.r));
    }

    using i128 = __int128_t;
    const i128 N = static_cast<i128>(max_coordinate) - min_coordinate;
    const ll block_count = max<ll>(1, sqrt(static_cast<double>(Q)));
    const i128 block_size = max<i128>(1, N / block_count);

    sort(qs.begin(), qs.end(), [&](const Query& a, const Query& b) {
        const i128 block_a = (static_cast<i128>(a.l) - min_coordinate) / block_size;
        const i128 block_b = (static_cast<i128>(b.l) - min_coordinate) / block_size;
        if (block_a != block_b) return block_a < block_b;
        if (block_a % 2 == 0) return a.r < b.r;
        return a.r > b.r;
    });
}

void FastMo(vq& qs) {
    const size_t Q = qs.size();
    if (Q == 0) return;

    ll min_coordinate = min(qs[0].l, qs[0].r);
    ll max_coordinate = max(qs[0].l, qs[0].r);
    for (const Query& q : qs) {
        min_coordinate = min(min_coordinate, min(q.l, q.r));
        max_coordinate = max(max_coordinate, max(q.l, q.r));
    }

    using i128 = __int128_t;
    const i128 N = static_cast<i128>(max_coordinate) - min_coordinate;
    const i128 block_size = max<i128>(
        1, static_cast<i128>(static_cast<long double>(N) / sqrtl(Q)));

    // 通常の帯と、境界を block_size / 2 ずらした帯の両方を試す。
    // 2 倍した座標で分類することで、block_size が奇数でも半整数の
    // ずらしを切り捨てずに扱う。
    auto make_order = [&](int phase) {
        vq order = qs;
        sort(order.begin(), order.end(), [&](const Query& a, const Query& b) {
            const auto block = [&](ll l) -> i128 {
                const i128 x = static_cast<i128>(l) - min_coordinate;
                return (2 * x + phase * block_size) / (2 * block_size);
            };
            const i128 block_a = block(a.l);
            const i128 block_b = block(b.l);
            if (block_a != block_b) return block_a < block_b;
            if ((block_a & 1) == 0) return a.r < b.r;
            return a.r > b.r;
        });
        return order;
    };

    const auto path_length = [](const vq& order) {
        i128 length = 0;
        i128 l = 0, r = 0;
        for (const Query& q : order) {
            const i128 next_l = q.l, next_r = q.r;
            const i128 dl = next_l - l, dr = next_r - r;
            length += (dl < 0 ? -dl : dl) + (dr < 0 ? -dr : dr);
            l = next_l;
            r = next_r;
        }
        return length;
    };

    vq normal = make_order(0);
    vq shifted = make_order(1);
    qs = path_length(normal) <= path_length(shifted) ? move(normal)
                                                     : move(shifted);
}
