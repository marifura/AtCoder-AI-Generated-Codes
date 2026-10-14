#pragma once
// 出典: https://github.com/marifura/AtCoder-AI-Generated-Codes/blob/main/exported/algorithm/implicit_treap.hpp

#include <cstddef>
#include <cstdint>
#include <limits>
#include <optional>
#include <stdexcept>
#include <utility>
#include <vector>

// 遅延評価付き Implicit Treap
//
// S:                 要素および区間集約値の型
// op(a, b):          a の列の後ろに b の列を連結したときの集約値
// e():               op の単位元
// F:                 遅延作用の型
// mapping(f, x, n):  長さ n の区間集約値 x に作用 f を適用
// composition(f, g): g を適用した後に f を適用する合成作用
// id():              作用の恒等写像
//
// op は結合的である必要があります。可換である必要はありません。
// mapping は次を満たす必要があります。
//   mapping(f, op(a, b), na + nb)
//     == op(mapping(f, a, na), mapping(f, b, nb))
//
// 添字は 0-indexed、区間は半開区間 [l, r) です。
// 各操作の期待計算量:
//   at/set/insert/erase/prod/apply/reverse: O(log N)
//   rotate/move:                            O(log N)
//   bispp:                                  O(log N)
//   insert_range (K elements):               O(K log K + log N)
//   erase_range (K elements):                O(K + log N)
//   to_vector:                              O(N)
template <
    class S,
    auto op,
    auto e,
    class F,
    auto mapping,
    auto composition,
    auto id
>
class ImplicitTreap {
private:
    struct Node {
        S value;
        S product;
        S reverse_product;
        F lazy;
        std::uint64_t priority;
        std::size_t subtree_size;
        bool has_lazy;
        bool reversed;
        Node* left;
        Node* right;

        Node(const S& value_, std::uint64_t priority_)
            : value(value_),
              product(value_),
              reverse_product(value_),
              lazy(id()),
              priority(priority_),
              subtree_size(1),
              has_lazy(false),
              reversed(false),
              left(nullptr),
              right(nullptr) {}
    };

    Node* root_ = nullptr;
    std::uint64_t random_state_ = 0x9e3779b97f4a7c15ULL;

    static std::size_t node_size(const Node* node) {
        return node == nullptr ? 0 : node->subtree_size;
    }

    static S node_product(const Node* node) {
        return node == nullptr ? e() : node->product;
    }

    static S node_reverse_product(const Node* node) {
        return node == nullptr ? e() : node->reverse_product;
    }

    // splitmix64。優先度生成をオブジェクト内で完結させる。
    std::uint64_t next_priority() {
        std::uint64_t z = (random_state_ += 0x9e3779b97f4a7c15ULL);
        z = (z ^ (z >> 30)) * 0xbf58476d1ce4e5b9ULL;
        z = (z ^ (z >> 27)) * 0x94d049bb133111ebULL;
        return z ^ (z >> 31);
    }

    static void pull(Node* node) {
        if (node == nullptr) return;

        node->subtree_size =
            node_size(node->left) + 1 + node_size(node->right);
        node->product = op(
            node_product(node->left),
            op(node->value, node_product(node->right))
        );
        node->reverse_product = op(
            node_reverse_product(node->right),
            op(node->value, node_reverse_product(node->left))
        );
    }

    static void apply_to_node(Node* node, const F& action) {
        if (node == nullptr) return;

        node->value = mapping(action, node->value, std::size_t{1});
        node->product =
            mapping(action, node->product, node->subtree_size);
        node->reverse_product =
            mapping(action, node->reverse_product, node->subtree_size);

        if (node->has_lazy) {
            node->lazy = composition(action, node->lazy);
        } else {
            node->lazy = action;
            node->has_lazy = true;
        }
    }

    static void reverse_node(Node* node) {
        if (node == nullptr) return;
        std::swap(node->left, node->right);
        std::swap(node->product, node->reverse_product);
        node->reversed = !node->reversed;
    }

    static void push(Node* node) {
        if (node == nullptr) return;

        if (node->has_lazy) {
            apply_to_node(node->left, node->lazy);
            apply_to_node(node->right, node->lazy);
            node->lazy = id();
            node->has_lazy = false;
        }
        if (node->reversed) {
            reverse_node(node->left);
            reverse_node(node->right);
            node->reversed = false;
        }
    }

    // 先頭 k 要素と、それ以降に分割する。
    static std::pair<Node*, Node*> split(Node* node, std::size_t k) {
        if (node == nullptr) return {nullptr, nullptr};

        push(node);
        if (k <= node_size(node->left)) {
            auto [left_tree, middle_tree] = split(node->left, k);
            node->left = middle_tree;
            pull(node);
            return {left_tree, node};
        }

        auto [middle_tree, right_tree] = split(
            node->right, k - node_size(node->left) - 1
        );
        node->right = middle_tree;
        pull(node);
        return {node, right_tree};
    }

    // left_tree の全要素が right_tree より前にあるものとして結合する。
    static Node* merge(Node* left_tree, Node* right_tree) {
        if (left_tree == nullptr) return right_tree;
        if (right_tree == nullptr) return left_tree;

        if (left_tree->priority > right_tree->priority) {
            push(left_tree);
            left_tree->right = merge(left_tree->right, right_tree);
            pull(left_tree);
            return left_tree;
        }

        push(right_tree);
        right_tree->left = merge(left_tree, right_tree->left);
        pull(right_tree);
        return right_tree;
    }

    static void destroy(Node* node) {
        if (node == nullptr) return;
        destroy(node->left);
        destroy(node->right);
        delete node;
    }

    static S get_at(Node* node, std::size_t index) {
        push(node);
        const std::size_t left_size = node_size(node->left);
        if (index < left_size) return get_at(node->left, index);
        if (index == left_size) return node->value;
        return get_at(node->right, index - left_size - 1);
    }

    static void set_at(Node* node, std::size_t index, const S& value) {
        push(node);
        const std::size_t left_size = node_size(node->left);
        if (index < left_size) {
            set_at(node->left, index, value);
        } else if (index == left_size) {
            node->value = value;
        } else {
            set_at(node->right, index - left_size - 1, value);
        }
        pull(node);
    }

    static void append_to_vector(Node* node, std::vector<S>& result) {
        if (node == nullptr) return;
        push(node);
        append_to_vector(node->left, result);
        result.push_back(node->value);
        append_to_vector(node->right, result);
    }

    void check_index(std::size_t index) const {
        if (index >= size()) {
            throw std::out_of_range("ImplicitTreap: index out of range");
        }
    }

    void check_insert_index(std::size_t index) const {
        if (index > size()) {
            throw std::out_of_range(
                "ImplicitTreap: insertion index out of range"
            );
        }
    }

    void check_range(std::size_t left, std::size_t right) const {
        if (left > right || right > size()) {
            throw std::out_of_range("ImplicitTreap: range out of range");
        }
    }

public:
    ImplicitTreap() = default;

    // count 個の要素を op の単位元 e() で初期化する。
    explicit ImplicitTreap(std::size_t count)
        : ImplicitTreap(count, e()) {}

    // count 個の要素を value で初期化する。
    ImplicitTreap(std::size_t count, const S& value) {
        Node* built_tree = nullptr;
        try {
            for (std::size_t i = 0; i < count; ++i) {
                Node* new_node = new Node(value, next_priority());
                built_tree = merge(built_tree, new_node);
            }
        } catch (...) {
            destroy(built_tree);
            throw;
        }
        root_ = built_tree;
    }

    explicit ImplicitTreap(const std::vector<S>& values) {
        insert_range(0, values);
    }

    ImplicitTreap(const ImplicitTreap&) = delete;
    ImplicitTreap& operator=(const ImplicitTreap&) = delete;

    ImplicitTreap(ImplicitTreap&& other) noexcept
        : root_(std::exchange(other.root_, nullptr)),
          random_state_(other.random_state_) {}

    ImplicitTreap& operator=(ImplicitTreap&& other) noexcept {
        if (this == &other) return *this;
        clear();
        root_ = std::exchange(other.root_, nullptr);
        random_state_ = other.random_state_;
        return *this;
    }

    ~ImplicitTreap() {
        destroy(root_);
    }

    std::size_t size() const {
        return node_size(root_);
    }

    bool empty() const {
        return root_ == nullptr;
    }

    void clear() {
        destroy(root_);
        root_ = nullptr;
    }

    // index の直前に value を挿入する。index == size() なら末尾に挿入。
    void insert(std::size_t index, const S& value) {
        check_insert_index(index);
        auto [left_tree, right_tree] = split(root_, index);
        Node* new_node = new Node(value, next_priority());
        root_ = merge(merge(left_tree, new_node), right_tree);
    }

    // index の直前に values をまとめて挿入する。
    void insert_range(std::size_t index, const std::vector<S>& values) {
        check_insert_index(index);
        if (values.empty()) return;

        Node* middle_tree = nullptr;
        try {
            for (const S& value : values) {
                Node* new_node = new Node(value, next_priority());
                middle_tree = merge(middle_tree, new_node);
            }
        } catch (...) {
            destroy(middle_tree);
            throw;
        }

        auto [left_tree, right_tree] = split(root_, index);
        root_ = merge(merge(left_tree, middle_tree), right_tree);
    }

    void push_front(const S& value) {
        insert(0, value);
    }

    void push_back(const S& value) {
        insert(size(), value);
    }

    // index の要素を削除し、その値を返す。
    S erase(std::size_t index) {
        check_index(index);
        auto [left_tree, rest] = split(root_, index);
        auto [target, right_tree] = split(rest, 1);
        push(target);
        S erased_value = target->value;
        delete target;
        root_ = merge(left_tree, right_tree);
        return erased_value;
    }

    // [left, right) をまとめて削除する。
    void erase_range(std::size_t left, std::size_t right) {
        check_range(left, right);
        if (left == right) return;

        auto [left_tree, rest] = split(root_, left);
        auto [middle_tree, right_tree] = split(rest, right - left);
        destroy(middle_tree);
        root_ = merge(left_tree, right_tree);
    }

    S pop_front() {
        return erase(0);
    }

    S pop_back() {
        if (empty()) {
            throw std::out_of_range("ImplicitTreap: pop from empty treap");
        }
        return erase(size() - 1);
    }

    S at(std::size_t index) {
        check_index(index);
        return get_at(root_, index);
    }

    S operator[](std::size_t index) {
        return at(index);
    }

    // predicate(value) が true となる側の境界添字を返す。
    // ok/ng は [-1, size()] 内の異なる添字で、true/false 側を表す。
    // 両端は判定せず、-1 と size() は仮想的な番兵として使用できる。
    // 両端の間で条件が単調であること。predicate 内で列を変更しないこと。
    // predicate には添字や区間集約値ではなく、各要素の値を渡す。
    template <class Predicate>
    long long bispp(long long ok, long long ng, Predicate predicate) {
        if (std::cmp_greater(size(), std::numeric_limits<long long>::max())) {
            throw std::length_error("ImplicitTreap: size exceeds index limit");
        }
        const long long count = static_cast<long long>(size());
        if (ok < -1 || ok > count || ng < -1 || ng > count) {
            throw std::out_of_range("ImplicitTreap: search index out of range");
        }
        if (ok == ng) {
            throw std::invalid_argument("ImplicitTreap: search endpoints must differ");
        }

        const bool true_on_left = ok < ng;
        long long lower = true_on_left ? ok : ng;
        long long upper = true_on_left ? ng : ok;
        Node* node = root_;
        std::size_t offset = 0;
        while (node != nullptr && lower + 1 < upper) {
            push(node);
            const std::size_t index = offset + node_size(node->left);
            const long long position = static_cast<long long>(index);
            if (position <= lower) {
                offset = index + 1;
                node = node->right;
            } else if (position >= upper) {
                node = node->left;
            } else {
                const bool accepted = predicate(std::as_const(node->value));
                if (accepted) ok = position;
                if (accepted == true_on_left) {
                    lower = position;
                    offset = index + 1;
                    node = node->right;
                } else {
                    upper = position;
                    node = node->left;
                }
            }
        }
        return ok;
    }

    void set(std::size_t index, const S& value) {
        check_index(index);
        set_at(root_, index, value);
    }

    // [left, right) の集約値を返す。
    S prod(std::size_t left, std::size_t right) {
        check_range(left, right);
        if (left == right) return e();

        auto [left_tree, rest] = split(root_, left);
        auto [middle_tree, right_tree] = split(rest, right - left);
        S result = node_product(middle_tree);
        root_ = merge(left_tree, merge(middle_tree, right_tree));
        return result;
    }

    S all_prod() const {
        return node_product(root_);
    }

    // [left, right) の全要素に action を遅延適用する。
    void apply(std::size_t left, std::size_t right, const F& action) {
        check_range(left, right);
        if (left == right) return;

        auto [left_tree, rest] = split(root_, left);
        auto [middle_tree, right_tree] = split(rest, right - left);
        apply_to_node(middle_tree, action);
        root_ = merge(left_tree, merge(middle_tree, right_tree));
    }

    // [left, right) の要素順を反転する。
    void reverse(std::size_t left, std::size_t right) {
        check_range(left, right);
        if (left == right) return;

        auto [left_tree, rest] = split(root_, left);
        auto [middle_tree, right_tree] = split(rest, right - left);
        reverse_node(middle_tree);
        root_ = merge(left_tree, merge(middle_tree, right_tree));
    }

    // std::rotate と同様に、[left, middle) と [middle, right) を入れ替える。
    void rotate(
        std::size_t left,
        std::size_t middle,
        std::size_t right
    ) {
        check_range(left, right);
        if (middle < left || middle > right) {
            throw std::out_of_range(
                "ImplicitTreap: rotation middle out of range"
            );
        }
        if (left == middle || middle == right) return;

        auto [left_tree, rest] = split(root_, left);
        auto [first_tree, suffix] = split(rest, middle - left);
        auto [second_tree, right_tree] = split(suffix, right - middle);
        root_ = merge(
            left_tree,
            merge(second_tree, merge(first_tree, right_tree))
        );
    }

    // [left, right) を切り取り、残った列の position の直前へ移動する。
    void move(
        std::size_t left,
        std::size_t right,
        std::size_t position
    ) {
        check_range(left, right);
        const std::size_t remaining_size = size() - (right - left);
        if (position > remaining_size) {
            throw std::out_of_range(
                "ImplicitTreap: move destination out of range"
            );
        }
        if (left == right) return;

        auto [left_tree, rest] = split(root_, left);
        auto [middle_tree, right_tree] = split(rest, right - left);
        Node* remaining_tree = merge(left_tree, right_tree);

        auto [destination_left, destination_right] =
            split(remaining_tree, position);
        root_ = merge(
            destination_left,
            merge(middle_tree, destination_right)
        );
    }

    std::vector<S> to_vector() {
        std::vector<S> result;
        result.reserve(size());
        append_to_vector(root_, result);
        return result;
    }
};

// よく使うモノイド・遅延作用のプリセット。
//
// 使用例:
//   using Treap =
//       implicit_treap_presets::RangeAddRangeSumTreap<long long>;
//   Treap treap(std::vector<long long>{1, 2, 3});
//   treap.apply(0, 2, 10);  // {11, 12, 3}
//   auto sum = treap.prod(0, 3);
//
// 区間代入では T をそのまま apply に渡せます。
//   using Treap =
//       implicit_treap_presets::RangeAssignRangeMinTreap<long long>;
//   treap.apply(left, right, value);
//
// 区間アフィン変換 x <- a*x+b:
//   using Treap =
//       implicit_treap_presets::RangeAffineRangeSumTreap<long long>;
//   treap.apply(left, right, {a, b});
namespace implicit_treap_presets {

namespace detail {

template<class T>
struct RangeAddRangeSumSpec {
    static T op(const T& left, const T& right) { return left + right; }
    static T e() { return T{}; }
    static T mapping(const T& add, const T& sum, std::size_t length) {
        return sum + add * static_cast<T>(length);
    }
    static T composition(const T& newer, const T& older) {
        return newer + older;
    }
    static T id() { return T{}; }
};

template<class T>
struct RangeAddRangeMinSpec {
    static T op(const T& left, const T& right) {
        return left < right ? left : right;
    }
    static T e() { return std::numeric_limits<T>::max(); }
    static T mapping(const T& add, const T& minimum, std::size_t) {
        return minimum + add;
    }
    static T composition(const T& newer, const T& older) {
        return newer + older;
    }
    static T id() { return T{}; }
};

template<class T>
struct RangeAddRangeMaxSpec {
    static T op(const T& left, const T& right) {
        return left < right ? right : left;
    }
    static T e() { return std::numeric_limits<T>::lowest(); }
    static T mapping(const T& add, const T& maximum, std::size_t) {
        return maximum + add;
    }
    static T composition(const T& newer, const T& older) {
        return newer + older;
    }
    static T id() { return T{}; }
};

template<class T>
using AssignAction = std::optional<T>;

template<class T>
struct RangeAssignRangeSumSpec {
    using F = AssignAction<T>;
    static T op(const T& left, const T& right) { return left + right; }
    static T e() { return T{}; }
    static T mapping(const F& assignment, const T& sum, std::size_t length) {
        if (!assignment.has_value()) return sum;
        return *assignment * static_cast<T>(length);
    }
    static F composition(const F& newer, const F& older) {
        return newer.has_value() ? newer : older;
    }
    static F id() { return std::nullopt; }
};

template<class T>
struct RangeAssignRangeMinSpec {
    using F = AssignAction<T>;
    static T op(const T& left, const T& right) {
        return left < right ? left : right;
    }
    static T e() { return std::numeric_limits<T>::max(); }
    static T mapping(const F& assignment, const T& minimum, std::size_t) {
        return assignment.has_value() ? *assignment : minimum;
    }
    static F composition(const F& newer, const F& older) {
        return newer.has_value() ? newer : older;
    }
    static F id() { return std::nullopt; }
};

template<class T>
struct RangeAssignRangeMaxSpec {
    using F = AssignAction<T>;
    static T op(const T& left, const T& right) {
        return left < right ? right : left;
    }
    static T e() { return std::numeric_limits<T>::lowest(); }
    static T mapping(const F& assignment, const T& maximum, std::size_t) {
        return assignment.has_value() ? *assignment : maximum;
    }
    static F composition(const F& newer, const F& older) {
        return newer.has_value() ? newer : older;
    }
    static F id() { return std::nullopt; }
};

} // namespace detail

template<class T>
struct AffineAction {
    T multiplier = T{1};
    T additive = T{};
};

namespace detail {

template<class T>
struct RangeAffineRangeSumSpec {
    using F = AffineAction<T>;
    static T op(const T& left, const T& right) { return left + right; }
    static T e() { return T{}; }
    static T mapping(const F& affine, const T& sum, std::size_t length) {
        return affine.multiplier * sum
             + affine.additive * static_cast<T>(length);
    }
    // older の後に newer を適用する。
    static F composition(const F& newer, const F& older) {
        return {
            newer.multiplier * older.multiplier,
            newer.multiplier * older.additive + newer.additive
        };
    }
    static F id() { return {}; }
};

} // namespace detail

template<class T>
using RangeAddRangeSumTreap = ImplicitTreap<
    T,
    &detail::RangeAddRangeSumSpec<T>::op,
    &detail::RangeAddRangeSumSpec<T>::e,
    T,
    &detail::RangeAddRangeSumSpec<T>::mapping,
    &detail::RangeAddRangeSumSpec<T>::composition,
    &detail::RangeAddRangeSumSpec<T>::id
>;

template<class T>
using RangeAddRangeMinTreap = ImplicitTreap<
    T,
    &detail::RangeAddRangeMinSpec<T>::op,
    &detail::RangeAddRangeMinSpec<T>::e,
    T,
    &detail::RangeAddRangeMinSpec<T>::mapping,
    &detail::RangeAddRangeMinSpec<T>::composition,
    &detail::RangeAddRangeMinSpec<T>::id
>;

template<class T>
using RangeAddRangeMaxTreap = ImplicitTreap<
    T,
    &detail::RangeAddRangeMaxSpec<T>::op,
    &detail::RangeAddRangeMaxSpec<T>::e,
    T,
    &detail::RangeAddRangeMaxSpec<T>::mapping,
    &detail::RangeAddRangeMaxSpec<T>::composition,
    &detail::RangeAddRangeMaxSpec<T>::id
>;

template<class T>
using RangeAssignRangeSumTreap = ImplicitTreap<
    T,
    &detail::RangeAssignRangeSumSpec<T>::op,
    &detail::RangeAssignRangeSumSpec<T>::e,
    detail::AssignAction<T>,
    &detail::RangeAssignRangeSumSpec<T>::mapping,
    &detail::RangeAssignRangeSumSpec<T>::composition,
    &detail::RangeAssignRangeSumSpec<T>::id
>;

template<class T>
using RangeAssignRangeMinTreap = ImplicitTreap<
    T,
    &detail::RangeAssignRangeMinSpec<T>::op,
    &detail::RangeAssignRangeMinSpec<T>::e,
    detail::AssignAction<T>,
    &detail::RangeAssignRangeMinSpec<T>::mapping,
    &detail::RangeAssignRangeMinSpec<T>::composition,
    &detail::RangeAssignRangeMinSpec<T>::id
>;

template<class T>
using RangeAssignRangeMaxTreap = ImplicitTreap<
    T,
    &detail::RangeAssignRangeMaxSpec<T>::op,
    &detail::RangeAssignRangeMaxSpec<T>::e,
    detail::AssignAction<T>,
    &detail::RangeAssignRangeMaxSpec<T>::mapping,
    &detail::RangeAssignRangeMaxSpec<T>::composition,
    &detail::RangeAssignRangeMaxSpec<T>::id
>;

template<class T>
using RangeAffineRangeSumTreap = ImplicitTreap<
    T,
    &detail::RangeAffineRangeSumSpec<T>::op,
    &detail::RangeAffineRangeSumSpec<T>::e,
    AffineAction<T>,
    &detail::RangeAffineRangeSumSpec<T>::mapping,
    &detail::RangeAffineRangeSumSpec<T>::composition,
    &detail::RangeAffineRangeSumSpec<T>::id
>;

} // namespace implicit_treap_presets
