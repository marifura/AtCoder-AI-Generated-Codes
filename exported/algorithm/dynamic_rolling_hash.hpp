#pragma once
// 出典: https://github.com/marifura/AtCoder-AI-Generated-Codes/blob/main/exported/algorithm/dynamic_rolling_hash.hpp

#include "rolling_hash.hpp"

#include <stdexcept>
#include <string_view>
#include <type_traits>
#include <utility>

// 挿入・削除・区間反転に対応するローリングハッシュ。
// 添字は0-indexed、区間は半開区間 [l, r)。任意のバイト列を扱える。
// RollingHashと法・BASEを共有するため、同じ実行内のハッシュは相互比較可能。
// 編集・区間取得は期待 O(log N)、LCP・対称半径は期待 O(log^2 N)。
// K文字の挿入・削除は期待 O(K + log N)。構築は O(N)。
// ハッシュによる一致判定には衝突の可能性がある。
class DynamicRollingHash {
public:
    using ull = RollingHash::ull;
    using HashValue = RollingHash::HashValue;

private:
    static constexpr ull MOD = RollingHash::MOD;
    static constexpr int MAX_SIZE = std::numeric_limits<int>::max();

    struct Node {
        ull hash = 0;
        ull reverse_hash = 0;
        std::uint64_t priority = 0;
        int left = 0;
        int right = 0;
        int length = 0;
        std::uint16_t value = 0;
        bool reversed = false;
    };

    struct Product {
        ull hash = 0;
        ull reverse_hash = 0;
        int length = 0;
    };

    // 添字0は空部分木。削除ノードはleftを次ポインタとして再利用する。
    std::vector<Node> nodes_;
    mutable std::vector<ull> power_;
    int root_ = 0;
    int free_head_ = 0;
    std::size_t free_count_ = 0;
    std::uint64_t random_state_ = RollingHash::base() ^
        static_cast<std::uint64_t>(
            std::chrono::steady_clock::now().time_since_epoch().count());

    static ull add(ull a, ull b) {
        ull result = a + b;
        return result >= MOD ? result - MOD : result;
    }

    static ull mul(ull a, ull b) {
        // 2^61 == 1 (mod MOD)を使い、積を上下61 bitで折り畳む。
        const __uint128_t product = static_cast<__uint128_t>(a) * b;
        const ull result = static_cast<ull>(product >> 61) +
                           (static_cast<ull>(product) & MOD);
        return result >= MOD ? result - MOD : result;
    }

    std::uint64_t next_priority() {
        // splitmix64。固定優先度を狙った入力を避けるため種は実行時に決める。
        std::uint64_t z = (random_state_ += 0x9e3779b97f4a7c15ULL);
        z = (z ^ (z >> 30)) * 0xbf58476d1ce4e5b9ULL;
        z = (z ^ (z >> 27)) * 0x94d049bb133111ebULL;
        return z ^ (z >> 31);
    }

    int length(int node) const {
        return node == 0 ? 0 : nodes_[node].length;
    }

    void ensure_power(int n) const {
        if (power_.empty()) power_.push_back(1);
        const auto required = static_cast<std::size_t>(n) + 1;
        if (power_.size() >= required) return;
        if (power_.capacity() < required) {
            power_.reserve(std::max(required, power_.capacity() * 2));
        }
        const ull base = RollingHash::base();
        while (power_.size() < required) {
            power_.push_back(mul(power_.back(), base));
        }
    }

    void prepare_nodes(std::size_t count) {
        const std::size_t extra = count > free_count_ ? count - free_count_ : 0;
        const std::size_t required = std::max(nodes_.size(), std::size_t{1}) + extra;
        if (nodes_.capacity() < required) {
            const auto limit = static_cast<std::size_t>(MAX_SIZE) + 1;
            nodes_.reserve(std::max(required, std::min(limit, nodes_.capacity() * 2)));
        }
        if (nodes_.empty()) nodes_.emplace_back();
    }

    int new_node(unsigned char c) {
        int index;
        if (free_head_ != 0) {
            index = free_head_;
            free_head_ = nodes_[index].left;
            --free_count_;
        } else {
            index = static_cast<int>(nodes_.size());
            nodes_.emplace_back();
        }
        nodes_[index] = Node{};
        Node& node = nodes_[index];
        node.value = static_cast<std::uint16_t>(c) + 1;
        node.hash = node.reverse_hash = node.value;
        node.priority = next_priority();
        node.length = 1;
        return index;
    }

    void pull(int index) {
        Node& node = nodes_[index];
        const Node& left = nodes_[node.left];
        const Node& right = nodes_[node.right];
        const ull base = RollingHash::base();
        node.length = left.length + 1 + right.length;
        node.hash = add(mul(add(mul(left.hash, base), node.value),
                            power_[right.length]), right.hash);
        node.reverse_hash = add(
            mul(add(mul(right.reverse_hash, base), node.value),
                power_[left.length]), left.reverse_hash);
    }

    void reverse_node(int index) {
        if (index == 0) return;
        Node& node = nodes_[index];
        std::swap(node.left, node.right);
        std::swap(node.hash, node.reverse_hash);
        node.reversed = !node.reversed;
    }

    void push(int index) {
        Node& node = nodes_[index];
        if (!node.reversed) return;
        reverse_node(node.left);
        reverse_node(node.right);
        node.reversed = false;
    }

    std::pair<int, int> split(int index, int k) {
        if (k == 0) return {0, index};
        if (k == length(index)) return {index, 0};
        push(index);
        Node& node = nodes_[index];
        const int left_length = length(node.left);
        if (k <= left_length) {
            auto [left, middle] = split(node.left, k);
            node.left = middle;
            pull(index);
            return {left, index};
        }
        auto [middle, right] = split(node.right, k - left_length - 1);
        node.right = middle;
        pull(index);
        return {index, right};
    }

    int merge(int left, int right) {
        if (left == 0) return right;
        if (right == 0) return left;
        if (nodes_[left].priority > nodes_[right].priority) {
            push(left);
            nodes_[left].right = merge(nodes_[left].right, right);
            pull(left);
            return left;
        }
        push(right);
        nodes_[right].left = merge(left, nodes_[right].left);
        pull(right);
        return right;
    }

    int build_tree(std::string_view s, std::vector<int>& stack) {
        // 単調スタックでCartesian treeを線形時間で構築する。
        for (unsigned char c : s) {
            const int index = new_node(c);
            int last = 0;
            while (!stack.empty() && nodes_[stack.back()].priority < nodes_[index].priority) {
                last = stack.back();
                stack.pop_back();
                pull(last);
            }
            nodes_[index].left = last;
            if (!stack.empty()) nodes_[stack.back()].right = index;
            stack.push_back(index);
        }
        for (auto it = stack.rbegin(); it != stack.rend(); ++it) pull(*it);
        return stack.front();
    }

    void recycle(int index) {
        if (index == 0) return;
        recycle(nodes_[index].left);
        recycle(nodes_[index].right);
        nodes_[index].left = free_head_;
        free_head_ = index;
        ++free_count_;
    }

    void set_at(int index, int pos, unsigned char c) {
        push(index);
        Node& node = nodes_[index];
        const int left_length = length(node.left);
        if (pos < left_length) set_at(node.left, pos, c);
        else if (pos > left_length) set_at(node.right, pos - left_length - 1, c);
        else node.value = static_cast<std::uint16_t>(c) + 1;
        pull(index);
    }

    template <bool Both>
    Product append(Product left, Product right) const {
        if (left.length == 0) return right;
        if (right.length == 0) return left;
        if constexpr (Both) {
            left.reverse_hash = add(mul(right.reverse_hash, power_[left.length]),
                                    left.reverse_hash);
        }
        left.hash = add(mul(left.hash, power_[right.length]), right.hash);
        left.length += right.length;
        return left;
    }

    template <bool Both>
    Product query(int index, int l, int r, bool flipped) const {
        if (l == r) return {};
        const Node& node = nodes_[index];
        if (l == 0 && r == node.length) {
            return flipped ? Product{node.reverse_hash, node.hash, node.length}
                           : Product{node.hash, node.reverse_hash, node.length};
        }
        const int left = flipped ? node.right : node.left;
        const int right = flipped ? node.left : node.right;
        const int left_length = length(left);
        const bool child_flipped = flipped != node.reversed;
        if (r <= left_length) return query<Both>(left, l, r, child_flipped);
        if (l > left_length) {
            return query<Both>(right, l - left_length - 1, r - left_length - 1,
                               child_flipped);
        }
        Product result;
        if (l < left_length) result = query<Both>(left, l, left_length, child_flipped);
        result = append<Both>(result, {node.value, node.value, 1});
        if (r > left_length + 1) {
            result = append<Both>(result, query<Both>(right, 0, r - left_length - 1,
                                                     child_flipped));
        }
        return result;
    }

    void append_string(int index, bool flipped, std::string& result) const {
        if (index == 0) return;
        const Node& node = nodes_[index];
        append_string(flipped ? node.right : node.left, flipped != node.reversed, result);
        result.push_back(static_cast<char>(node.value - 1));
        append_string(flipped ? node.left : node.right, flipped != node.reversed, result);
    }

    ull prefix_hash(int count, bool flipped = false) const {
        ull result = 0;
        int index = root_;
        const ull base = RollingHash::base();
        // 既に通過した部分を残り長で重み付けし、片側の経路だけをたどる。
        while (count > 0) {
            const Node& node = nodes_[index];
            if (count == node.length) {
                return add(result, flipped ? node.reverse_hash : node.hash);
            }
            const int left = flipped ? node.right : node.left;
            const int right = flipped ? node.left : node.right;
            const int left_length = length(left);
            const bool child_flipped = flipped != node.reversed;
            if (count > left_length) {
                count -= left_length + 1;
                const ull left_hash = child_flipped ? nodes_[left].reverse_hash : nodes_[left].hash;
                result = add(result, mul(add(mul(left_hash, base), node.value), power_[count]));
                index = right;
            } else {
                index = left;
            }
            flipped = child_flipped;
        }
        return result;
    }

    void check_position(int pos, bool allow_end = false) const {
        if (pos < 0 || pos > size() || (!allow_end && pos == size())) {
            throw std::out_of_range("DynamicRollingHash: index out of range");
        }
    }

    void check_range(int l, int r) const {
        if (l < 0 || l > r || r > size()) {
            throw std::out_of_range("DynamicRollingHash: range out of range");
        }
    }

    template <class Other>
    int lcp_with(int i, const Other& other, int j, int limit) const {
        check_position(i, true);
        if (j < 0 || j > other.size()) {
            throw std::out_of_range("DynamicRollingHash: LCP index out of range");
        }
        if (limit < 0) throw std::invalid_argument("DynamicRollingHash: negative LCP limit");
        int low = 0;
        int high = std::min({limit, size() - i, other.size() - j});
        if (high == 0) return 0;
        if constexpr (std::is_same_v<Other, DynamicRollingHash>) {
            if (this == &other && i == j) return high;
        }
        if (static_cast<unsigned char>(at(i)) + 1ULL != other.get_hash(j, j + 1)) return 0;
        if (high == 1) return 1;
        low = 1;
        // 短い不一致は小区間だけで探し、長い完全一致は一度で確定する。
        const int probe = std::min(high, 8);
        if (get(i, i + probe) != other.get(j, j + probe)) {
            high = probe - 1;
        } else {
            if (probe == high || get(i, i + high) == other.get(j, j + high)) return high;
            low = probe;
            --high;
        }
        // 開始位置の接頭辞は固定なので、二分探索の各判定で再取得しない。
        const ull difference = RollingHash::mod_sub(prefix_hash(i), other.get_hash(0, j));
        while (low < high) {
            const int mid = low + (high - low) / 2 + 1;
            if (prefix_hash(i + mid) == add(other.get_hash(0, j + mid),
                                            mul(difference, power_[mid]))) low = mid;
            else high = mid - 1;
        }
        return low;
    }

public:
    DynamicRollingHash() = default;

    explicit DynamicRollingHash(std::string_view s) {
        insert(0, s);
    }

    DynamicRollingHash(const DynamicRollingHash&) = default;

    DynamicRollingHash(DynamicRollingHash&& other) noexcept
        : nodes_(std::move(other.nodes_)), power_(std::move(other.power_)),
          root_(std::exchange(other.root_, 0)),
          free_head_(std::exchange(other.free_head_, 0)),
          free_count_(std::exchange(other.free_count_, 0)),
          random_state_(other.random_state_) {}

    DynamicRollingHash& operator=(DynamicRollingHash other) noexcept {
        swap(other);
        return *this;
    }

    void swap(DynamicRollingHash& other) noexcept {
        nodes_.swap(other.nodes_);
        power_.swap(other.power_);
        std::swap(root_, other.root_);
        std::swap(free_head_, other.free_head_);
        std::swap(free_count_, other.free_count_);
        std::swap(random_state_, other.random_state_);
    }

    void build(std::string_view s) {
        DynamicRollingHash next(s);
        swap(next);
    }

    int size() const { return length(root_); }
    bool empty() const { return root_ == 0; }

    // 確保済み領域と累乗表は保持し、以後の挿入に再利用する。
    void clear() {
        nodes_.clear();
        root_ = free_head_ = 0;
        free_count_ = 0;
    }

    // posの直前へ挿入する。pos == size()なら末尾に追加する。
    void insert(int pos, std::string_view s) {
        check_position(pos, true);
        if (s.size() > static_cast<std::size_t>(MAX_SIZE - size())) {
            throw std::length_error("DynamicRollingHash: size exceeds INT_MAX");
        }
        if (s.empty()) return;
        // 例外を投げうる確保は既存の木を変更する前に完了させる。
        std::vector<int> stack;
        if (s.size() > 1) stack.reserve(s.size());
        ensure_power(size() + static_cast<int>(s.size()));
        prepare_nodes(s.size());
        const int middle = s.size() == 1 ? new_node(static_cast<unsigned char>(s[0]))
                                         : build_tree(s, stack);
        auto [left, right] = split(root_, pos);
        root_ = merge(merge(left, middle), right);
    }

    void insert(int pos, char c) { insert(pos, std::string_view(&c, 1)); }
    void push_front(char c) { insert(0, c); }
    void push_back(char c) { insert(size(), c); }

    void erase(int l, int r) {
        check_range(l, r);
        if (l == r) return;
        auto [left, rest] = split(root_, l);
        auto [middle, right] = split(rest, r - l);
        recycle(middle);
        root_ = merge(left, right);
    }

    char erase(int pos) {
        const char c = at(pos);
        erase(pos, pos + 1);
        return c;
    }

    void erase_range(int l, int r) { erase(l, r); }
    char pop_front() { return erase(0); }
    char pop_back() { return erase(size() - 1); }

    void reverse(int l, int r) {
        check_range(l, r);
        if (r - l <= 1) return;
        auto [left, rest] = split(root_, l);
        auto [middle, right] = split(rest, r - l);
        reverse_node(middle);
        root_ = merge(merge(left, middle), right);
    }

    void set(int pos, char c) {
        check_position(pos);
        set_at(root_, pos, static_cast<unsigned char>(c));
    }

    char at(int pos) const {
        check_position(pos);
        int index = root_;
        bool flipped = false;
        while (true) {
            const Node& node = nodes_[index];
            const int left = flipped ? node.right : node.left;
            const int left_length = length(left);
            if (pos == left_length) return static_cast<char>(node.value - 1);
            const int right = flipped ? node.left : node.right;
            flipped = flipped != node.reversed;
            if (pos < left_length) index = left;
            else {
                pos -= left_length + 1;
                index = right;
            }
        }
    }

    char operator[](int pos) const { return at(pos); }

    std::string to_string() const {
        std::string result;
        result.reserve(size());
        append_string(root_, false, result);
        return result;
    }

    // 取得では木の分割・結合や遅延反転の伝播を行わない。
    HashValue get(int l, int r) const {
        check_range(l, r);
        if (l == 0) return {prefix_hash(r), r};
        return {query<false>(root_, l, r, false).hash, r - l};
    }

    HashValue get_reverse(int l, int r) const {
        check_range(l, r);
        if (r == size()) return {prefix_hash(size() - l, true), r - l};
        return {query<false>(root_, size() - r, size() - l, true).hash, r - l};
    }

    ull get_hash(int l, int r) const { return get(l, r).value; }
    ull get_reverse_hash(int l, int r) const { return get_reverse(l, r).value; }

    bool equal(int l1, int r1, int l2, int r2) const {
        check_range(l1, r1);
        check_range(l2, r2);
        return r1 - l1 == r2 - l2 && get(l1, r1) == get(l2, r2);
    }

    bool is_palindrome(int l, int r) const {
        check_range(l, r);
        const Product result = query<true>(root_, l, r, false);
        return result.hash == result.reverse_hash;
    }

    int mirrored_radius(int l, int r) const {
        check_range(l, r);
        int high = std::min(l, size() - r);
        if (high == 0 || at(l - 1) != at(r)) return 0;
        if (get_reverse(l - high, l) == get(r, r + high)) return high;
        int low = 1;
        --high;
        const ull difference = RollingHash::mod_sub(prefix_hash(size() - l, true), prefix_hash(r));
        while (low < high) {
            const int mid = low + (high - low) / 2 + 1;
            if (prefix_hash(size() - l + mid, true) ==
                add(prefix_hash(r + mid), mul(difference, power_[mid]))) low = mid;
            else high = mid - 1;
        }
        return low;
    }

    // ハッシュ値は [0, 2^61 - 1)、長さは非負でなければならない。
    ull connect(ull left, ull right, int right_length) const {
        if (left >= MOD || right >= MOD || right_length < 0) {
            throw std::invalid_argument("DynamicRollingHash: invalid hash or length");
        }
        ensure_power(right_length);
        return add(mul(left, power_[right_length]), right);
    }

    HashValue connect(HashValue left, HashValue right) const {
        if (left.length < 0 || right.length < 0) {
            throw std::invalid_argument("DynamicRollingHash: negative hash length");
        }
        if (left.length > MAX_SIZE - right.length) {
            throw std::length_error("DynamicRollingHash: connected length exceeds INT_MAX");
        }
        return {connect(left.value, right.value, right.length), left.length + right.length};
    }

    int lcp(int i, int j, int limit) const { return lcp_with(i, *this, j, limit); }
    int lcp(int i, int j) const { return lcp(i, j, MAX_SIZE); }
    int lcp(int i, const DynamicRollingHash& other, int j, int limit) const {
        return lcp_with(i, other, j, limit);
    }
    int lcp(int i, const DynamicRollingHash& other, int j) const {
        return lcp_with(i, other, j, MAX_SIZE);
    }
    int lcp(int i, const RollingHash& other, int j, int limit) const {
        return lcp_with(i, other, j, limit);
    }
    int lcp(int i, const RollingHash& other, int j) const {
        return lcp_with(i, other, j, MAX_SIZE);
    }
};
