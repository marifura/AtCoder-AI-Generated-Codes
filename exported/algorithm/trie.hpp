#pragma once
// 出典: https://github.com/marifura/AtCoder-AI-Generated-Codes/blob/main/exported/algorithm/trie.hpp

#include <algorithm>
#include <array>
#include <cassert>
#include <cstddef>
#include <limits>
#include <optional>
#include <stdexcept>
#include <string>
#include <string_view>
#include <utility>
#include <vector>

// 重複と個数指定、辞書順の順位検索に対応する文字列の多重集合。
// 文字は unsigned char として margin から連続する X 種類。既定は英小文字。
// 空文字列は辞書順最小。順位は 0-indexed で、重複も数える。
// library-master/string/trie.hpp の Trie とは別実装のため同時にインクルードしない。
//
// 個数は long long。負の個数・対象外の文字・逆転した区間は invalid_argument、
// 総数の上限超過は overflow_error、ノード数の上限超過は length_error。
// 個数 0 の更新でも文字は検査する。引数エラーや挿入時の確保失敗では内容を保持する。
// 空集合の最小・最大、範囲外の順位、存在しない境界値検索は nullopt。
// コピー可能。ムーブ元は空になり再利用可能。取得した文字列は独立したコピー。
//
// 長さ L の挿入・削除・個数検索・接頭辞個数・最長接頭辞一致は O(L)。
// 挿入は配列拡張について償却、文字種類数 X を変数とすると O(X * L)。
// 辞書順の個数・順位・最小最大・境界値検索は O(X * (L + R)) 以下。
// R は返す文字列の長さ。区間個数は O(X * (|l| + |r|))。
// 保持領域は同時に必要だったノード数の最大値 P に対して O(X * P)。
// 削除ノードは再利用し、clear は確保済み領域を保持する。すべて非再帰。
//
// 使用例:
// Trie<> trie;
// trie.insert("banana", 100000);
// trie.insert("apple", 2);
// trie.count_less("banana");          // 2
// trie.kth(2).value();                // "banana"
// trie.erase("banana", 30000);        // true
// trie.count_prefix("ban");           // 70000
// Trie<10, '0'> digits;               // 数字用
template <std::size_t X = 26, char margin = 'a'>
class Trie {
    static constexpr std::size_t alphabet_begin = static_cast<unsigned char>(margin);
    static constexpr std::size_t alphabet_size =
        static_cast<std::size_t>(std::numeric_limits<unsigned char>::max()) + 1;
    static_assert(X > 0 && X <= alphabet_size, "Trie: invalid alphabet size");
    static_assert(alphabet_begin <= alphabet_size - X, "Trie: alphabet exceeds byte range");
    static constexpr std::size_t none = std::numeric_limits<std::size_t>::max();

public:
    using count_type = long long;

private:
    struct Node {
        std::array<std::size_t, X> next;
        std::size_t parent;
        std::size_t next_free = none;
        count_type terminal = 0;
        count_type total = 0;

        explicit Node(std::size_t p = none) noexcept : parent(p) {
            next.fill(none);
        }
    };

    std::vector<Node> nodes_;
    std::size_t free_head_ = none;
    std::size_t free_count_ = 0;
    count_type distinct_count_ = 0;

    static std::size_t letter(char c) noexcept {
        return static_cast<unsigned char>(c) - alphabet_begin;
    }

    static void validate(std::string_view s) {
        for (char c : s) {
            const auto value = static_cast<std::size_t>(static_cast<unsigned char>(c));
            if (value < alphabet_begin || value - alphabet_begin >= X) {
                throw std::invalid_argument("Trie: character outside alphabet");
            }
        }
    }

    static void validate_count(count_type count) {
        if (count < 0) throw std::invalid_argument("Trie: negative count");
    }

    // 文字の検査後に呼ぶ。空の木には根もまだ確保されていない。
    std::size_t find_node(std::string_view s) const noexcept {
        if (nodes_.empty()) return none;
        std::size_t pos = 0;
        for (char c : s) {
            pos = nodes_[pos].next[letter(c)];
            if (pos == none) return none;
        }
        return pos;
    }

    // 更新開始前に全領域を確保し、以後のノード追加で例外を発生させない。
    void reserve_additional(std::size_t additional) {
        const auto limit = nodes_.max_size();
        if (additional > limit - nodes_.size()) {
            throw std::length_error("Trie: too many nodes");
        }
        const auto required = nodes_.size() + additional;
        if (required <= nodes_.capacity()) return;
        const auto capacity = nodes_.capacity();
        const auto grown = capacity > limit / 2 ? limit : std::max<std::size_t>(1, capacity * 2);
        nodes_.reserve(std::max(required, grown));
    }

    std::size_t make_node(std::size_t parent) {
        if (free_head_ != none) {
            const auto pos = free_head_;
            free_head_ = nodes_[pos].next_free;
            --free_count_;
            nodes_[pos] = Node(parent);
            return pos;
        }
        const auto pos = nodes_.size();
        nodes_.emplace_back(parent);
        return pos;
    }

    // 末尾から根へ個数を減らす。不要な経路は葉から順に回収する。
    void erase_at(std::string_view s, std::size_t pos, count_type count) noexcept {
        nodes_[pos].terminal -= count;
        if (nodes_[pos].terminal == 0) --distinct_count_;
        std::size_t depth = s.size();
        while (true) {
            auto& node = nodes_[pos];
            node.total -= count;
            if (pos == 0) break;
            const auto parent = node.parent;
            if (node.total == 0) {
                nodes_[parent].next[letter(s[depth - 1])] = none;
                node.next_free = free_head_;
                free_head_ = pos;
                ++free_count_;
            }
            pos = parent;
            --depth;
        }
    }

    // 有効な部分木と順位を受け取り、接頭辞に続く文字を復元する。
    std::string select(std::size_t pos, count_type k, std::string result) const {
        while (true) {
            const auto& node = nodes_[pos];
            if (k < node.terminal) return result;
            k -= node.terminal;
            std::size_t child = none;
            for (std::size_t c = 0; c < X; ++c) {
                const auto next = node.next[c];
                if (next == none) continue;
                if (k < nodes_[next].total) {
                    child = next;
                    result.push_back(static_cast<char>(alphabet_begin + c));
                    break;
                }
                k -= nodes_[next].total;
            }
            assert(child != none);
            pos = child;
        }
    }

public:
    Trie() = default;
    Trie(const Trie&) = default;
    Trie& operator=(const Trie&) = default;

    Trie(Trie&& other) noexcept
        : nodes_(std::move(other.nodes_)),
          free_head_(std::exchange(other.free_head_, none)),
          free_count_(std::exchange(other.free_count_, 0)),
          distinct_count_(std::exchange(other.distinct_count_, 0)) {
        other.nodes_.clear();
    }

    Trie& operator=(Trie&& other) noexcept {
        if (this != &other) {
            nodes_ = std::move(other.nodes_);
            free_head_ = std::exchange(other.free_head_, none);
            free_count_ = std::exchange(other.free_count_, 0);
            distinct_count_ = std::exchange(other.distinct_count_, 0);
            other.nodes_.clear();
        }
        return *this;
    }

    // 総数は重複込み。種類数には空文字列も含める。
    count_type size() const noexcept { return nodes_.empty() ? 0 : nodes_[0].total; }
    count_type distinct_size() const noexcept { return distinct_count_; }
    bool empty() const noexcept { return size() == 0; }

    // ノード数に比例する時間以下。確保済み領域は以後の挿入で再利用する。
    void clear() noexcept {
        nodes_.clear();
        free_head_ = none;
        free_count_ = 0;
        distinct_count_ = 0;
    }

    void insert(std::string_view s, count_type count = 1) {
        validate(s);
        validate_count(count);
        if (count == 0) return;
        if (count > std::numeric_limits<count_type>::max() - size()) {
            throw std::overflow_error("Trie: count overflow");
        }

        std::size_t pos = 0, matched = 0;
        if (!nodes_.empty()) {
            while (matched < s.size()) {
                const auto child = nodes_[pos].next[letter(s[matched])];
                if (child == none) break;
                pos = child;
                ++matched;
            }
        }
        std::size_t needed = s.size() - matched;
        if (nodes_.empty()) {
            if (needed == none) throw std::length_error("Trie: too many nodes");
            ++needed;
        }
        reserve_additional(needed > free_count_ ? needed - free_count_ : 0);
        if (nodes_.empty()) nodes_.emplace_back();
        for (std::size_t i = matched; i < s.size(); ++i) {
            const auto child = make_node(pos);
            nodes_[pos].next[letter(s[i])] = child;
            pos = child;
        }
        if (nodes_[pos].terminal == 0) ++distinct_count_;
        nodes_[pos].terminal += count;
        for (auto p = pos; p != none; p = nodes_[p].parent) nodes_[p].total += count;
    }

    // 指定個数を削除。不足なら変更せず false。個数 0 なら未登録でも true。
    bool erase(std::string_view s, count_type count = 1) {
        validate(s);
        validate_count(count);
        if (count == 0) return true;
        const auto pos = find_node(s);
        if (pos == none || nodes_[pos].terminal < count) return false;
        erase_at(s, pos, count);
        return true;
    }

    // 同じ単語を全個削除し、削除個数を返す。未登録なら 0。
    count_type erase_all(std::string_view s) {
        validate(s);
        const auto pos = find_node(s);
        if (pos == none || nodes_[pos].terminal == 0) return 0;
        const auto removed = nodes_[pos].terminal;
        erase_at(s, pos, removed);
        return removed;
    }

    count_type count(std::string_view s) const {
        validate(s);
        const auto pos = find_node(s);
        return pos == none ? 0 : nodes_[pos].terminal;
    }

    bool contains(std::string_view s) const { return count(s) != 0; }

    // 空接頭辞なら全単語の総数。
    count_type count_prefix(std::string_view prefix) const {
        validate(prefix);
        const auto pos = find_node(prefix);
        return pos == none ? 0 : nodes_[pos].total;
    }

    // s 自身を含めず、辞書順で厳密に小さい単語を数える。
    count_type count_less(std::string_view s) const {
        validate(s);
        if (nodes_.empty()) return 0;
        count_type result = 0;
        std::size_t pos = 0;
        for (char c : s) {
            const auto& node = nodes_[pos];
            result += node.terminal;
            const auto k = letter(c);
            for (std::size_t i = 0; i < k; ++i) {
                const auto child = node.next[i];
                if (child != none) result += nodes_[child].total;
            }
            pos = node.next[k];
            if (pos == none) break;
        }
        return result;
    }

    // 半開区間 [l, r) の個数。l > r は invalid_argument。
    count_type count_range(std::string_view l, std::string_view r) const {
        validate(l);
        validate(r);
        if (l > r) throw std::invalid_argument("Trie: reversed range");
        return count_less(r) - count_less(l);
    }

    std::optional<std::string> kth(count_type k) const {
        if (k < 0 || k >= size()) return std::nullopt;
        return select(0, k, {});
    }

    std::optional<std::string> min() const { return kth(0); }
    std::optional<std::string> max() const { return kth(size() - 1); }

    // 最小または最大の単語を 1 個削除し、その単語を返す。
    std::optional<std::string> pop_min() {
        auto result = min();
        if (result) erase(*result);
        return result;
    }

    std::optional<std::string> pop_max() {
        auto result = max();
        if (result) erase(*result);
        return result;
    }

    // それぞれ s 以上の最小、s より大きい最小、s より小さい最大。
    std::optional<std::string> lower_bound(std::string_view s) const {
        return kth(count_less(s));
    }

    std::optional<std::string> upper_bound(std::string_view s) const {
        return kth(count_less(s) + count(s));
    }

    std::optional<std::string> predecessor(std::string_view s) const {
        return kth(count_less(s) - 1);
    }

    // 指定接頭辞を持つ単語の中での順位。戻り値は接頭辞を含む単語全体。
    std::optional<std::string> kth_prefix(std::string_view prefix, count_type k) const {
        validate(prefix);
        const auto pos = find_node(prefix);
        if (pos == none || k < 0 || k >= nodes_[pos].total) return std::nullopt;
        return select(pos, k, std::string(prefix));
    }

    std::optional<std::string> min_prefix(std::string_view prefix) const {
        return kth_prefix(prefix, 0);
    }

    std::optional<std::string> max_prefix(std::string_view prefix) const {
        return kth_prefix(prefix, count_prefix(prefix) - 1);
    }

    // 登録単語のいずれかと共有する接頭辞の最大長。空集合なら 0。
    // 登録単語自体が s の接頭辞である必要はない。
    std::size_t longest_prefix_match(std::string_view s) const {
        validate(s);
        if (empty()) return 0;
        std::size_t pos = 0, length = 0;
        for (char c : s) {
            pos = nodes_[pos].next[letter(c)];
            if (pos == none) break;
            ++length;
        }
        return length;
    }
};
