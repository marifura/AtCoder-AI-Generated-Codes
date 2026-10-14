#pragma once

#include <cstddef>
#include <cstdint>
#include <functional>
#include <limits>
#include <stdexcept>
#include <type_traits>
#include <utility>
#include <vector>

// 重複と順位検索に対応するランダム化二分探索木。
// Compare は狭義弱順序を定めること。比較上同値の値は同じ頂点にまとめる。
// T はコピー構築可能であること。同値の値では最初に挿入した値を保持する。
// size は重複込み、kth は Compare 順の 0-indexed。既定では昇順。
// 比較を O(1) として単一要素の検索・更新は期待 O(log(D+1))、領域は O(D)。
// 区間削除は期待 O(log(D+1)+d)（d は解放する頂点数）。
// to_vector は重複込みの要素数 N に対して O(N)。
// D は異なる値の数。木の高さの最悪値は O(D)。
template <class T, class Compare = std::less<T>>
class OrderedMultiSet {
    struct Node {
        T value;
        std::uint64_t priority;
        std::size_t count = 1, size = 1;
        Node* left = nullptr;
        Node* right = nullptr;
        Node(const T& x, std::uint64_t p) : value(x), priority(p) {}
    };

    Compare compare_;
    Node* root_ = nullptr;
    std::uint64_t random_state_ = 0x9e3779b97f4a7c15ULL;

    static std::size_t size_of(const Node* p) noexcept {
        return p ? p->size : 0;
    }
    static void pull(Node* p) noexcept {
        p->size = size_of(p->left) + p->count + size_of(p->right);
    }
    std::uint64_t next_priority() noexcept {
        std::uint64_t z = (random_state_ += 0x9e3779b97f4a7c15ULL);
        z = (z ^ (z >> 30)) * 0xbf58476d1ce4e5b9ULL;
        z = (z ^ (z >> 27)) * 0x94d049bb133111ebULL;
        return z ^ (z >> 31);
    }
    static void rotate_right(Node*& p) noexcept {
        Node* q = p->left;
        p->left = q->right;
        q->right = p;
        pull(p);
        pull(q);
        p = q;
    }
    static void rotate_left(Node*& p) noexcept {
        Node* q = p->right;
        p->right = q->left;
        q->left = p;
        pull(p);
        pull(q);
        p = q;
    }
    void insert_at(Node*& p, const T& x) {
        if (!p) {
            p = new Node(x, next_priority());
            return;
        }
        if (compare_(x, p->value)) {
            insert_at(p->left, x);
            if (p->left->priority > p->priority) rotate_right(p);
        } else if (compare_(p->value, x)) {
            insert_at(p->right, x);
            if (p->right->priority > p->priority) rotate_left(p);
        } else {
            ++p->count;
        }
        pull(p);
    }
    static Node* merge(Node* a, Node* b) noexcept {
        if (!a) return b;
        if (!b) return a;
        if (a->priority > b->priority) {
            a->right = merge(a->right, b);
            pull(a);
            return a;
        }
        b->left = merge(a, b->left);
        pull(b);
        return b;
    }
    std::size_t erase_at(Node*& p, const T& x, bool all) {
        if (!p) return 0;
        std::size_t removed;
        if (compare_(x, p->value)) {
            removed = erase_at(p->left, x, all);
        } else if (compare_(p->value, x)) {
            removed = erase_at(p->right, x, all);
        } else if (!all && p->count > 1) {
            --p->count;
            removed = 1;
        } else {
            removed = p->count;
            Node* old = p;
            p = merge(p->left, p->right);
            delete old;
            return removed;
        }
        pull(p);
        return removed;
    }

    // 再帰や追加領域を使わずに部分木を破棄する。
    static void destroy(Node* p) noexcept {
        while (p) {
            if (p->left) {
                Node* q = p->left;
                p->left = q->right;
                q->right = p;
                p = q;
            } else {
                Node* next = p->right;
                delete p;
                p = next;
            }
        }
    }
    // 呼び出し時点の部分木の添字 [l, r) を削除する。
    static void erase_indices(Node*& p, std::size_t l, std::size_t r) noexcept {
        if (l == r) return;
        if (l == 0 && r == size_of(p)) {
            destroy(p);
            p = nullptr;
            return;
        }
        const auto left = size_of(p->left);
        const auto end = left + p->count;
        if (r > end) erase_indices(p->right, l > end ? l - end : 0, r - end);
        if (l < left) erase_indices(p->left, l, r < left ? r : left);
        const auto begin_removed = l > left ? l : left;
        const auto end_removed = r < end ? r : end;
        if (begin_removed < end_removed) p->count -= end_removed - begin_removed;
        if (p->count == 0) {
            Node* old = p;
            p = merge(p->left, p->right);
            delete old;
        } else pull(p);
    }

public:
    OrderedMultiSet() : compare_() {}
    explicit OrderedMultiSet(Compare compare) : compare_(std::move(compare)) {}
    OrderedMultiSet(const OrderedMultiSet&) = delete;
    OrderedMultiSet& operator=(const OrderedMultiSet&) = delete;

    // 比較器の移動が例外を投げない場合にムーブ可能。
    OrderedMultiSet(OrderedMultiSet&& other) noexcept
        requires std::is_nothrow_move_constructible_v<Compare>
        : compare_(std::move(other.compare_)),
          root_(std::exchange(other.root_, nullptr)),
          random_state_(other.random_state_) {}

    OrderedMultiSet& operator=(OrderedMultiSet&& other) noexcept
        requires std::is_nothrow_move_assignable_v<Compare> {
        if (this != &other) {
            clear();
            compare_ = std::move(other.compare_);
            root_ = std::exchange(other.root_, nullptr);
            random_state_ = other.random_state_;
        }
        return *this;
    }
    ~OrderedMultiSet() { clear(); }

    std::size_t size() const noexcept { return size_of(root_); }
    bool empty() const noexcept { return !root_; }

    // 再帰や追加領域を使わず、異なる値の数に比例する時間で破棄する。
    void clear() noexcept {
        destroy(root_);
        root_ = nullptr;
    }
    void insert(const T& x) {
        if (size() == std::numeric_limits<std::size_t>::max()) {
            throw std::length_error("OrderedMultiSet: size overflow");
        }
        insert_at(root_, x);
    }
    bool erase_one(const T& x) { return erase_at(root_, x, false) != 0; }
    std::size_t erase_all(const T& x) { return erase_at(root_, x, true); }

    // 添字の半開区間を削除し、削除数を返す。重複の途中でも指定可能。
    std::size_t erase_index_range(std::size_t l, std::size_t r) {
        if (l > r || r > size()) {
            throw std::out_of_range("OrderedMultiSet: invalid index range");
        }
        erase_indices(root_, l, r);
        return r - l;
    }
    // Compare 順で値の半開区間 [l, r) を削除する。
    std::size_t erase_range(const T& l, const T& r) {
        if (compare_(r, l)) throw std::invalid_argument("OrderedMultiSet: reversed value range");
        const auto begin = order_of_key(l);
        const auto end = order_of_key(r);
        return erase_index_range(begin, end);
    }
    void erase_kth(std::size_t k) {
        if (k >= size()) throw std::out_of_range("OrderedMultiSet: erase_kth out of range");
        erase_indices(root_, k, k + 1);
    }
    // 先頭・末尾を1個削除する。空なら false を返す。
    bool pop_front() {
        if (empty()) return false;
        erase_indices(root_, 0, 1);
        return true;
    }
    bool pop_back() {
        if (empty()) return false;
        erase_indices(root_, size() - 1, size());
        return true;
    }

    std::size_t count(const T& x) const {
        const Node* p = root_;
        while (p) {
            if (compare_(x, p->value)) p = p->left;
            else if (compare_(p->value, x)) p = p->right;
            else return p->count;
        }
        return 0;
    }
    // x より前にある要素の数。同値の要素は含めない。
    std::size_t order_of_key(const T& x) const {
        std::size_t result = 0;
        const Node* p = root_;
        while (p) {
            if (compare_(p->value, x)) {
                result += size_of(p->left) + p->count;
                p = p->right;
            } else p = p->left;
        }
        return result;
    }
    // x より前、または同値である要素の数。既定の比較器では x 以下。
    std::size_t count_leq(const T& x) const {
        std::size_t result = 0;
        const Node* p = root_;
        while (p) {
            if (!compare_(x, p->value)) {
                result += size_of(p->left) + p->count;
                p = p->right;
            } else p = p->left;
        }
        return result;
    }
    // 大小はすべて Compare 順。既定の比較器では通常の数値の大小。
    std::size_t count_less(const T& x) const { return order_of_key(x); }
    std::size_t count_geq(const T& x) const { return size() - order_of_key(x); }
    std::size_t count_greater(const T& x) const { return size() - count_leq(x); }
    std::size_t count_range(const T& l, const T& r) const {
        if (compare_(r, l)) throw std::invalid_argument("OrderedMultiSet: reversed value range");
        return order_of_key(r) - order_of_key(l);
    }
    // 添字 k 以降の要素数。k == size() は許可する。
    std::size_t count_from_index(std::size_t k) const {
        if (k > size()) throw std::out_of_range("OrderedMultiSet: index out of range");
        return size() - k;
    }
    // 返した参照は、その値の最後の要素の削除または clear まで有効。
    const T& kth(std::size_t k) const {
        if (k >= size()) throw std::out_of_range("OrderedMultiSet: kth out of range");
        const Node* p = root_;
        while (p) {
            const auto left = size_of(p->left);
            if (k < left) p = p->left;
            else if (k - left < p->count) return p->value;
            else {
                k -= left + p->count;
                p = p->right;
            }
        }
        throw std::logic_error("OrderedMultiSet: invalid subtree size");
    }
    // 大小と最小・最大は Compare 順。該当値がなければ out_of_range。
    // 返した参照の有効期間は kth と同じ。
    // x 以上の最小値。
    const T& min_geq(const T& x) const { return kth(order_of_key(x)); }
    // x より大きい最小値。
    const T& min_greater(const T& x) const { return kth(count_leq(x)); }
    // x 以下の最大値。
    const T& max_leq(const T& x) const {
        const auto end = count_leq(x);
        if (end == 0) throw std::out_of_range("OrderedMultiSet: max_leq has no matching value");
        return kth(end - 1);
    }
    // x 未満の最大値。
    const T& max_less(const T& x) const {
        const auto end = order_of_key(x);
        if (end == 0) throw std::out_of_range("OrderedMultiSet: max_less has no matching value");
        return kth(end - 1);
    }
    const T& front() const { return kth(0); }
    const T& back() const {
        if (empty()) throw std::out_of_range("OrderedMultiSet: back on empty set");
        return kth(size() - 1);
    }
    // Compare 順に列挙する。重複もすべて含め、値のコピーは合計 N 回。
    std::vector<T> to_vector() const {
        std::vector<T> result;
        result.reserve(size());
        std::vector<const Node*> stack;
        const Node* p = root_;
        while (p || !stack.empty()) {
            while (p) {
                stack.push_back(p);
                p = p->left;
            }
            p = stack.back();
            stack.pop_back();
            for (std::size_t i = 0; i < p->count; ++i) result.push_back(p->value);
            p = p->right;
        }
        return result;
    }
};
