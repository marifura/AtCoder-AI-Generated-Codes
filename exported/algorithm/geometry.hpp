#pragma once
// 出典: https://github.com/marifura/AtCoder-AI-Generated-Codes/blob/main/exported/algorithm/geometry.hpp

#include <bits/stdc++.h>

namespace geometry {

// Point<T>は点とベクトルの両方を表す。
// 整数型Tの判定は__int128_tで計算し、射影・交点・距離に関する演算で
// 座標を返す場合はlong doubleを使用する。
// 平行な直線と同一直線には一意な交点がないため、intersection_point()の前に
// intersection_type()を確認する。

using ld = long double;
using i128 = __int128_t;
using u128 = __uint128_t;
constexpr ld EPS = 1e-12L;

template<class T>
struct Point {
    static_assert(std::is_arithmetic_v<T>);
    T x{}, y{};

    Point() = default;
    Point(T x_, T y_) : x(x_), y(y_) {}

    Point operator+() const { return *this; }
    Point operator-() const { return {-x, -y}; }

    Point& operator+=(const Point& p) { x += p.x; y += p.y; return *this; }
    Point& operator-=(const Point& p) { x -= p.x; y -= p.y; return *this; }
    Point& operator*=(const T& k) { x *= k; y *= k; return *this; }
    Point& operator/=(const T& k) { x /= k; y /= k; return *this; }

    friend Point operator+(Point a, const Point& b) { return a += b; }
    friend Point operator-(Point a, const Point& b) { return a -= b; }
    friend Point operator*(Point a, const T& k) { return a *= k; }
    friend Point operator*(const T& k, Point a) { return a *= k; }
    friend Point operator/(Point a, const T& k) { return a /= k; }

    friend bool operator==(const Point&, const Point&) = default;
    friend bool operator<(const Point& a, const Point& b) {
        return a.x != b.x ? a.x < b.x : a.y < b.y;
    }
};

template<class T>
std::istream& operator>>(std::istream& is, Point<T>& p) {
    return is >> p.x >> p.y;
}

template<class T>
std::ostream& operator<<(std::ostream& os, const Point<T>& p) {
    return os << p.x << ' ' << p.y;
}

template<class T>
using Vector = Point<T>;

template<class T>
using Calc = std::conditional_t<std::is_integral_v<T>, i128, ld>;

template<class T>
Calc<T> calc(T x) {
    return static_cast<Calc<T>>(x);
}

template<class T>
int sign(Calc<T> x) {
    if constexpr (std::is_integral_v<T>) return (x > 0) - (x < 0);
    else return (x > EPS) - (x < -EPS);
}

inline bool equal(ld a, ld b) {
    return std::abs(a - b) <= EPS;
}

template<class T>
bool equal(const Point<T>& a, const Point<T>& b) {
    if constexpr (std::is_integral_v<T>) return a == b;
    else return equal(static_cast<ld>(a.x), static_cast<ld>(b.x))
             && equal(static_cast<ld>(a.y), static_cast<ld>(b.y));
}

template<class T>
Point<ld> to_ld(const Point<T>& p) {
    return {static_cast<ld>(p.x), static_cast<ld>(p.y)};
}

// 平面を回転・拡大し、元の座標でのマンハッタン距離が返された座標での
// チェビシェフ距離と等しくなるように変換する。
// 整数型の結果をTで表現できない場合は例外を送出する。
template<class T>
    requires (std::signed_integral<T> || std::floating_point<T>)
Point<T> manhattan_to_chebyshev(const Point<T>& p) {
    if constexpr (std::integral<T>) {
        const i128 x = p.x, y = p.y;
        const i128 u = x + y, v = x - y;
        const i128 lowest = std::numeric_limits<T>::lowest();
        const i128 highest = std::numeric_limits<T>::max();
        if (u < lowest || highest < u || v < lowest || highest < v) {
            throw std::overflow_error("coordinate transformation overflow");
        }
        return {static_cast<T>(u), static_cast<T>(v)};
    } else {
        return {p.x + p.y, p.x - p.y};
    }
}

// manhattan_to_chebyshev()の逆変換を行う。整数のチェビシェフ座標は
// 偶奇が一致しなければならず、一致しない場合の逆変換は半整数点になる。
template<class T>
    requires (std::signed_integral<T> || std::floating_point<T>)
Point<T> chebyshev_to_manhattan(const Point<T>& p) {
    if constexpr (std::integral<T>) {
        const i128 u = p.x, v = p.y;
        if ((u + v) % 2 != 0) {
            throw std::invalid_argument(
                "Chebyshev coordinates must have the same parity");
        }
        return {static_cast<T>((u + v) / 2),
                static_cast<T>((u - v) / 2)};
    } else {
        return {(p.x + p.y) / T{2}, (p.x - p.y) / T{2}};
    }
}

// 整数座標では、座標差と距離をCalc<T>で計算してオーバーフローを避ける。
template<class T>
Calc<T> manhattan_distance(const Point<T>& a, const Point<T>& b) {
    auto dx = calc(a.x) - calc(b.x);
    auto dy = calc(a.y) - calc(b.y);
    if (dx < 0) dx = -dx;
    if (dy < 0) dy = -dy;
    return dx + dy;
}

template<class T>
Calc<T> chebyshev_distance(const Point<T>& a, const Point<T>& b) {
    auto dx = calc(a.x) - calc(b.x);
    auto dy = calc(a.y) - calc(b.y);
    if (dx < 0) dx = -dx;
    if (dy < 0) dy = -dy;
    return std::max(dx, dy);
}

// 点集合のマンハッタン直径をO(N)で求める。空集合と1点集合の直径は0とする。
template<class T>
Calc<T> manhattan_diameter(const std::vector<Point<T>>& points) {
    if (points.size() <= 1) return 0;

    const auto first_x = calc(points.front().x);
    const auto first_y = calc(points.front().y);
    auto minimum_sum = first_x + first_y;
    auto maximum_sum = minimum_sum;
    auto minimum_difference = first_x - first_y;
    auto maximum_difference = minimum_difference;

    for (const auto& point : points) {
        const auto x = calc(point.x);
        const auto y = calc(point.y);
        const auto sum = x + y;
        const auto difference = x - y;
        minimum_sum = std::min(minimum_sum, sum);
        maximum_sum = std::max(maximum_sum, sum);
        minimum_difference = std::min(minimum_difference, difference);
        maximum_difference = std::max(maximum_difference, difference);
    }
    return std::max(maximum_sum - minimum_sum,
                    maximum_difference - minimum_difference);
}

// 点集合のチェビシェフ直径をO(N)で求める。空集合と1点集合の直径は0とする。
template<class T>
Calc<T> chebyshev_diameter(const std::vector<Point<T>>& points) {
    if (points.size() <= 1) return 0;

    auto minimum_x = calc(points.front().x);
    auto maximum_x = minimum_x;
    auto minimum_y = calc(points.front().y);
    auto maximum_y = minimum_y;

    for (const auto& point : points) {
        const auto x = calc(point.x);
        const auto y = calc(point.y);
        minimum_x = std::min(minimum_x, x);
        maximum_x = std::max(maximum_x, x);
        minimum_y = std::min(minimum_y, y);
        maximum_y = std::max(maximum_y, y);
    }
    return std::max(maximum_x - minimum_x, maximum_y - minimum_y);
}

template<class T>
Calc<T> dot(const Vector<T>& a, const Vector<T>& b) {
    return calc(a.x) * calc(b.x) + calc(a.y) * calc(b.y);
}

template<class T>
Calc<T> cross(const Vector<T>& a, const Vector<T>& b) {
    return calc(a.x) * calc(b.y) - calc(a.y) * calc(b.x);
}

template<class T>
Calc<T> cross(const Point<T>& a, const Point<T>& b, const Point<T>& c) {
    const auto abx = calc(b.x) - calc(a.x);
    const auto aby = calc(b.y) - calc(a.y);
    const auto acx = calc(c.x) - calc(a.x);
    const auto acy = calc(c.y) - calc(a.y);
    return abx * acy - aby * acx;
}

template<class T>
Calc<T> dot_from(const Point<T>& origin, const Point<T>& a, const Point<T>& b) {
    const auto ax = calc(a.x) - calc(origin.x);
    const auto ay = calc(a.y) - calc(origin.y);
    const auto bx = calc(b.x) - calc(origin.x);
    const auto by = calc(b.y) - calc(origin.y);
    return ax * bx + ay * by;
}

template<class T>
Calc<T> norm2(const Vector<T>& v) {
    return dot(v, v);
}

template<class T>
ld norm(const Vector<T>& v) {
    return std::hypotl(static_cast<ld>(v.x), static_cast<ld>(v.y));
}

template<class T>
ld distance(const Point<T>& a, const Point<T>& b) {
    return std::hypotl(static_cast<ld>(a.x) - static_cast<ld>(b.x),
                       static_cast<ld>(a.y) - static_cast<ld>(b.y));
}

template<class T>
Vector<T> rotate90(const Vector<T>& v) {
    return {-v.y, v.x};
}

template<class T>
Point<ld> rotate(const Vector<T>& v, ld radians) {
    const ld c = std::cos(radians), s = std::sin(radians);
    return {static_cast<ld>(v.x) * c - static_cast<ld>(v.y) * s,
            static_cast<ld>(v.x) * s + static_cast<ld>(v.y) * c};
}

template<class T>
ld argument(const Vector<T>& v) {
    return std::atan2(static_cast<ld>(v.y), static_cast<ld>(v.x));
}

// 零ベクトルには方向がないため、いずれかが零ベクトルならfalseを返す。
template<class T>
bool same_direction(const Vector<T>& lhs, const Vector<T>& rhs) {
    if constexpr (std::is_integral_v<T>) {
        if ((lhs.x == 0 && lhs.y == 0) || (rhs.x == 0 && rhs.y == 0)) {
            return false;
        }
        if (cross(lhs, rhs) != 0) return false;
        if (lhs.x != 0) return (lhs.x > 0) == (rhs.x > 0);
        return (lhs.y > 0) == (rhs.y > 0);
    } else {
        const bool lhs_is_zero = sign<T>(calc(lhs.x)) == 0
                              && sign<T>(calc(lhs.y)) == 0;
        const bool rhs_is_zero = sign<T>(calc(rhs.x)) == 0
                              && sign<T>(calc(rhs.y)) == 0;
        return !lhs_is_zero && !rhs_is_zero
            && sign<T>(cross(lhs, rhs)) == 0 && dot(lhs, rhs) > 0;
    }
}

// 零ベクトルには方向がないため、いずれかが零ベクトルならfalseを返す。
template<class T>
bool opposite_direction(const Vector<T>& lhs, const Vector<T>& rhs) {
    if constexpr (std::is_integral_v<T>) {
        if ((lhs.x == 0 && lhs.y == 0) || (rhs.x == 0 && rhs.y == 0)) {
            return false;
        }
        if (cross(lhs, rhs) != 0) return false;
        if (lhs.x != 0) return (lhs.x > 0) != (rhs.x > 0);
        return (lhs.y > 0) != (rhs.y > 0);
    } else {
        const bool lhs_is_zero = sign<T>(calc(lhs.x)) == 0
                              && sign<T>(calc(lhs.y)) == 0;
        const bool rhs_is_zero = sign<T>(calc(rhs.x)) == 0
                              && sign<T>(calc(rhs.y)) == 0;
        return !lhs_is_zero && !rhs_is_zero
            && sign<T>(cross(lhs, rhs)) == 0 && dot(lhs, rhs) < 0;
    }
}

// argument()が返す[-pi, pi]の偏角の昇順と互換な順序を定義する。
// 零ベクトルの偏角は0として扱う。同じ半直線上のベクトルは同値とし、
// 順序を維持する必要がある場合はstd::stable_sortを使用する。
template<class T>
int argument_partition(const Vector<T>& v) {
    if (v.y < 0) return 0;
    if (v.y == 0 && v.x >= 0) return 1;
    return 2;
}

template<class T>
bool argument_less(const Vector<T>& lhs, const Vector<T>& rhs) {
    const int lhs_partition = argument_partition(lhs);
    const int rhs_partition = argument_partition(rhs);
    if (lhs_partition != rhs_partition) {
        return lhs_partition < rhs_partition;
    }
    return cross(lhs, rhs) > 0;
}

template<class T>
bool argument_greater(const Vector<T>& lhs, const Vector<T>& rhs) {
    return argument_less(rhs, lhs);
}

template<class T>
bool argument_less_equal(const Vector<T>& lhs, const Vector<T>& rhs) {
    return !argument_greater(lhs, rhs);
}

template<class T>
bool argument_greater_equal(const Vector<T>& lhs, const Vector<T>& rhs) {
    return !argument_less(lhs, rhs);
}

template<class T>
bool argument_equal(const Vector<T>& lhs, const Vector<T>& rhs) {
    return !argument_less(lhs, rhs) && !argument_greater(lhs, rhs);
}

template<class T>
bool argument_not_equal(const Vector<T>& lhs, const Vector<T>& rhs) {
    return !argument_equal(lhs, rhs);
}

template<class T>
void sort_by_argument(std::vector<Point<T>>& points) {
    std::sort(points.begin(), points.end(), argument_less<T>);
}

template<class T>
void sort_by_argument(std::vector<Point<T>>& points, const Point<T>& origin) {
    const Point<T> center = origin;
    const auto partition = [&](const Point<T>& p) {
        const auto x = calc(p.x) - calc(center.x);
        const auto y = calc(p.y) - calc(center.y);
        if (y < 0) return 0;
        if (y == 0 && x >= 0) return 1;
        return 2;
    };
    std::sort(points.begin(), points.end(), [&](const Point<T>& lhs,
                                                const Point<T>& rhs) {
        const int lhs_partition = partition(lhs);
        const int rhs_partition = partition(rhs);
        if (lhs_partition != rhs_partition) {
            return lhs_partition < rhs_partition;
        }
        return cross(center, lhs, rhs) > 0;
    });
}

template<class T>
Point<ld> normalized(const Vector<T>& v) {
    const ld length = norm(v);
    if (length <= EPS) throw std::invalid_argument("zero vector cannot be normalized");
    return {static_cast<ld>(v.x) / length, static_cast<ld>(v.y) / length};
}

// -1: 時計回り、0: 一直線上、1: 反時計回り。
template<class T>
int orientation(const Point<T>& a, const Point<T>& b, const Point<T>& c) {
    return sign<T>(cross(a, b, c));
}

enum class Ccw {
    Clockwise = -1,
    OnSegment = 0,
    CounterClockwise = 1,
    OnlineBack = 2,
    OnlineFront = -2,
};

template<class T>
Ccw ccw(const Point<T>& a, const Point<T>& b, const Point<T>& c) {
    const int turn = orientation(a, b, c);
    if (turn > 0) return Ccw::CounterClockwise;
    if (turn < 0) return Ccw::Clockwise;
    if (sign<T>(dot_from(a, b, c)) < 0) return Ccw::OnlineBack;
    if (sign<T>(dot_from(a, c, c) - dot_from(a, b, b)) > 0) return Ccw::OnlineFront;
    return Ccw::OnSegment;
}

template<class T>
struct Line {
    Point<T> a, b;
};

template<class T>
struct Segment {
    Point<T> a, b;
};

u128 magnitude(i128 value) {
    u128 converted = static_cast<u128>(value);
    return value < 0 ? -converted : converted;
}

u128 gcd128(u128 a, u128 b) {
    while (b != 0) {
        const u128 remainder = a % b;
        a = b;
        b = remainder;
    }
    return a;
}

struct IntegerLine {
    i128 a, b, c;

    IntegerLine(i128 a_, i128 b_, i128 c_) : a(a_), b(b_), c(c_) {
        if (a == 0 && b == 0) {
            throw std::invalid_argument("a line requires a non-zero normal vector");
        }

        const u128 divisor = gcd128(gcd128(magnitude(a), magnitude(b)),
                                    magnitude(c));
        const i128 signed_divisor = static_cast<i128>(divisor);
        a /= signed_divisor;
        b /= signed_divisor;
        c /= signed_divisor;

        if (a < 0 || (a == 0 && b < 0)) {
            a = -a;
            b = -b;
            c = -c;
        }
    }

    friend bool operator==(const IntegerLine&, const IntegerLine&) = default;
};

enum class LineIntersectionType {
    None,
    Point,
    Coincident,
};

template<std::integral T>
IntegerLine to_integer_line(const Line<T>& line) {
    if (line.a == line.b) {
        throw std::invalid_argument("a line requires two distinct points");
    }

    const i128 x1 = line.a.x, y1 = line.a.y;
    const i128 x2 = line.b.x, y2 = line.b.y;
    return {y1 - y2, x2 - x1, x1 * y2 - x2 * y1};
}

std::pair<i128, i128> normalized_normal(const IntegerLine& line) {
    const u128 divisor = gcd128(magnitude(line.a), magnitude(line.b));
    const i128 signed_divisor = static_cast<i128>(divisor);
    i128 a = line.a / signed_divisor;
    i128 b = line.b / signed_divisor;
    if (a < 0 || (a == 0 && b < 0)) {
        a = -a;
        b = -b;
    }
    return {a, b};
}

template<std::integral T>
bool on_line(const IntegerLine& line, const Point<T>& p) {
    return line.a * static_cast<i128>(p.x)
         + line.b * static_cast<i128>(p.y) + line.c == 0;
}

bool parallel(const IntegerLine& lhs, const IntegerLine& rhs) {
    return normalized_normal(lhs) == normalized_normal(rhs);
}

bool perpendicular(const IntegerLine& lhs, const IntegerLine& rhs) {
    return lhs.a * rhs.a + lhs.b * rhs.b == 0;
}

LineIntersectionType intersection_type(const IntegerLine& lhs,
                                       const IntegerLine& rhs) {
    if (!parallel(lhs, rhs)) return LineIntersectionType::Point;
    return lhs == rhs ? LineIntersectionType::Coincident
                      : LineIntersectionType::None;
}

std::optional<Point<ld>> intersection_point(const IntegerLine& lhs,
                                            const IntegerLine& rhs) {
    if (intersection_type(lhs, rhs) != LineIntersectionType::Point) {
        return std::nullopt;
    }

    const i128 determinant = lhs.a * rhs.b - rhs.a * lhs.b;
    const i128 x_numerator = lhs.b * rhs.c - rhs.b * lhs.c;
    const i128 y_numerator = lhs.c * rhs.a - rhs.c * lhs.a;
    const ld denominator = static_cast<ld>(determinant);
    return {{static_cast<ld>(x_numerator) / denominator,
             static_cast<ld>(y_numerator) / denominator}};
}

template<class T>
Point<ld> projection(const IntegerLine& line, const Point<T>& p) {
    const ld a = static_cast<ld>(line.a);
    const ld b = static_cast<ld>(line.b);
    const ld c = static_cast<ld>(line.c);
    const Point<ld> point = to_ld(p);
    const ld ratio = (a * point.x + b * point.y + c) / (a * a + b * b);
    return {point.x - a * ratio, point.y - b * ratio};
}

template<class T>
Point<ld> reflection(const IntegerLine& line, const Point<T>& p) {
    const Point<ld> point = to_ld(p);
    return projection(line, p) * 2.0L - point;
}

template<class T>
ld distance(const Point<T>& p, const IntegerLine& line) {
    const ld a = static_cast<ld>(line.a);
    const ld b = static_cast<ld>(line.b);
    const ld c = static_cast<ld>(line.c);
    const Point<ld> point = to_ld(p);
    return std::abs(a * point.x + b * point.y + c) / std::hypotl(a, b);
}

template<class T>
ld distance(const IntegerLine& line, const Point<T>& p) {
    return distance(p, line);
}

ld distance(const IntegerLine& lhs, const IntegerLine& rhs) {
    if (intersection_type(lhs, rhs) != LineIntersectionType::None) return 0;

    // 正規化した平行線の(a,b)の向きは同じでも、3係数全体の縮尺は異なる場合がある。
    // cを直接引かず、lhs上の任意の点をrhsへ代入して距離を求める。
    const ld a1 = static_cast<ld>(lhs.a);
    const ld b1 = static_cast<ld>(lhs.b);
    const ld c1 = static_cast<ld>(lhs.c);
    Point<ld> point;
    if (std::abs(a1) >= std::abs(b1)) point = {-c1 / a1, 0};
    else point = {0, -c1 / b1};
    return distance(point, rhs);
}

template<class T>
bool is_degenerate(const Line<T>& line) {
    return equal(line.a, line.b);
}

template<class T>
bool is_degenerate(const Segment<T>& segment) {
    return equal(segment.a, segment.b);
}

template<class T>
bool on_line(const Line<T>& line, const Point<T>& p) {
    if constexpr (std::is_integral_v<T>) {
        return on_line(to_integer_line(line), p);
    } else {
        if (is_degenerate(line)) {
            throw std::invalid_argument("a line requires two distinct points");
        }
        return orientation(line.a, line.b, p) == 0;
    }
}

template<class T>
bool on_segment(const Segment<T>& segment, const Point<T>& p) {
    if (orientation(segment.a, segment.b, p) != 0) return false;
    const auto pax = calc(p.x) - calc(segment.a.x);
    const auto pay = calc(p.y) - calc(segment.a.y);
    const auto pbx = calc(p.x) - calc(segment.b.x);
    const auto pby = calc(p.y) - calc(segment.b.y);
    return sign<T>(pax * pbx + pay * pby) <= 0;
}

template<class T>
bool parallel(const Line<T>& lhs, const Line<T>& rhs) {
    if constexpr (std::is_integral_v<T>) {
        return parallel(to_integer_line(lhs), to_integer_line(rhs));
    } else {
        if (is_degenerate(lhs) || is_degenerate(rhs)) {
            throw std::invalid_argument("a line requires two distinct points");
        }
        const auto lx = calc(lhs.b.x) - calc(lhs.a.x);
        const auto ly = calc(lhs.b.y) - calc(lhs.a.y);
        const auto rx = calc(rhs.b.x) - calc(rhs.a.x);
        const auto ry = calc(rhs.b.y) - calc(rhs.a.y);
        return sign<T>(lx * ry - ly * rx) == 0;
    }
}

template<class T>
bool perpendicular(const Line<T>& lhs, const Line<T>& rhs) {
    if constexpr (std::is_integral_v<T>) {
        return perpendicular(to_integer_line(lhs), to_integer_line(rhs));
    } else {
        if (is_degenerate(lhs) || is_degenerate(rhs)) {
            throw std::invalid_argument("a line requires two distinct points");
        }
        const auto lx = calc(lhs.b.x) - calc(lhs.a.x);
        const auto ly = calc(lhs.b.y) - calc(lhs.a.y);
        const auto rx = calc(rhs.b.x) - calc(rhs.a.x);
        const auto ry = calc(rhs.b.y) - calc(rhs.a.y);
        return sign<T>(lx * rx + ly * ry) == 0;
    }
}

template<class T>
LineIntersectionType intersection_type(const Line<T>& lhs, const Line<T>& rhs) {
    if constexpr (std::is_integral_v<T>) {
        return intersection_type(to_integer_line(lhs), to_integer_line(rhs));
    } else {
        if (!parallel(lhs, rhs)) return LineIntersectionType::Point;
        return on_line(lhs, rhs.a) ? LineIntersectionType::Coincident
                                   : LineIntersectionType::None;
    }
}

template<class T>
std::optional<Point<ld>> intersection_point(const Line<T>& lhs, const Line<T>& rhs) {
    if constexpr (std::is_integral_v<T>) {
        return intersection_point(to_integer_line(lhs), to_integer_line(rhs));
    } else {
        if (intersection_type(lhs, rhs) != LineIntersectionType::Point) {
            return std::nullopt;
        }
        const Point<ld> p = to_ld(lhs.a), q = to_ld(rhs.a);
        const Point<ld> r = to_ld(lhs.b) - p;
        const Point<ld> s = to_ld(rhs.b) - q;
        const ld t = cross(q - p, s) / cross(r, s);
        return p + r * t;
    }
}

template<class T>
Point<ld> projection(const Line<T>& line, const Point<T>& p) {
    if constexpr (std::is_integral_v<T>) {
        return projection(to_integer_line(line), p);
    } else {
        if (is_degenerate(line)) {
            throw std::invalid_argument("a line requires two distinct points");
        }
        const Point<ld> a = to_ld(line.a), b = to_ld(line.b), point = to_ld(p);
        const auto direction = b - a;
        const ld t = dot(point - a, direction) / dot(direction, direction);
        return a + direction * t;
    }
}

template<class T>
Point<ld> reflection(const Line<T>& line, const Point<T>& p) {
    const Point<ld> point = to_ld(p);
    return projection(line, p) * 2.0L - point;
}

template<class T>
ld distance(const Point<T>& p, const Line<T>& line) {
    if constexpr (std::is_integral_v<T>) {
        return distance(p, to_integer_line(line));
    } else {
        if (is_degenerate(line)) {
            throw std::invalid_argument("a line requires two distinct points");
        }
        const Point<ld> a = to_ld(line.a), b = to_ld(line.b), point = to_ld(p);
        return std::abs(cross(b - a, point - a)) / norm(b - a);
    }
}

template<class T>
ld distance(const Line<T>& line, const Point<T>& p) {
    return distance(p, line);
}

template<class T>
ld distance(const Point<T>& p, const Segment<T>& segment) {
    if (is_degenerate(segment)) return distance(p, segment.a);
    const Point<ld> a = to_ld(segment.a), b = to_ld(segment.b), point = to_ld(p);
    const auto direction = b - a;
    const ld t = dot(point - a, direction) / dot(direction, direction);
    if (t <= 0) return distance(point, a);
    if (t >= 1) return distance(point, b);
    return distance(point, a + direction * t);
}

template<class T>
ld distance(const Segment<T>& segment, const Point<T>& p) {
    return distance(p, segment);
}

template<class T>
std::vector<Point<ld>> intersection_points(const Segment<T>& lhs,
                                            const Segment<T>& rhs) {
    std::vector<Point<ld>> result;
    const auto add = [&](const Point<T>& p) {
        const Point<ld> converted = to_ld(p);
        for (const auto& current : result) {
            if (equal(current, converted)) return;
        }
        result.push_back(converted);
    };

    if (on_segment(lhs, rhs.a)) add(rhs.a);
    if (on_segment(lhs, rhs.b)) add(rhs.b);
    if (on_segment(rhs, lhs.a)) add(lhs.a);
    if (on_segment(rhs, lhs.b)) add(lhs.b);

    const int o1 = orientation(lhs.a, lhs.b, rhs.a);
    const int o2 = orientation(lhs.a, lhs.b, rhs.b);
    const int o3 = orientation(rhs.a, rhs.b, lhs.a);
    const int o4 = orientation(rhs.a, rhs.b, lhs.b);
    if (result.empty() && o1 * o2 < 0 && o3 * o4 < 0) {
        const auto point = intersection_point(Line<T>{lhs.a, lhs.b},
                                              Line<T>{rhs.a, rhs.b});
        if (point) result.push_back(*point);
    }

    std::sort(result.begin(), result.end());
    return result;
}

enum class SegmentIntersectionType {
    None,
    Point,
    Overlap,
};

template<class T>
SegmentIntersectionType intersection_type(const Segment<T>& lhs,
                                           const Segment<T>& rhs) {
    const auto points = intersection_points(lhs, rhs);
    if (points.empty()) return SegmentIntersectionType::None;
    if (points.size() == 1) return SegmentIntersectionType::Point;
    return SegmentIntersectionType::Overlap;
}

template<class T>
bool intersects(const Segment<T>& lhs, const Segment<T>& rhs) {
    return intersection_type(lhs, rhs) != SegmentIntersectionType::None;
}

template<class T>
ld distance(const Line<T>& lhs, const Line<T>& rhs) {
    if constexpr (std::is_integral_v<T>) {
        return distance(to_integer_line(lhs), to_integer_line(rhs));
    } else {
        if (intersection_type(lhs, rhs) != LineIntersectionType::None) return 0;
        return distance(lhs.a, rhs);
    }
}

template<class T>
ld distance(const Segment<T>& lhs, const Segment<T>& rhs) {
    if (intersects(lhs, rhs)) return 0;
    return std::min({distance(lhs.a, rhs), distance(lhs.b, rhs),
                     distance(rhs.a, lhs), distance(rhs.b, lhs)});
}

template<std::integral T>
IntegerLine perpendicular_bisector(const Point<T>& a, const Point<T>& b) {
    if (a == b) throw std::invalid_argument("two distinct points are required");

    const i128 x1 = a.x, y1 = a.y;
    const i128 x2 = b.x, y2 = b.y;
    return {2 * (x2 - x1), 2 * (y2 - y1),
            x1 * x1 + y1 * y1 - x2 * x2 - y2 * y2};
}

template<std::floating_point T>
Line<ld> perpendicular_bisector(const Point<T>& a, const Point<T>& b) {
    if (equal(a, b)) throw std::invalid_argument("two distinct points are required");
    const Point<ld> lhs = to_ld(a), rhs = to_ld(b);
    const Point<ld> midpoint = (lhs + rhs) / 2.0L;
    const auto direction = rotate90(rhs - lhs);
    return {midpoint, midpoint + direction};
}

template<class T>
auto perpendicular_bisector(const Segment<T>& segment) {
    return perpendicular_bisector(segment.a, segment.b);
}

template<class T>
using Polygon = std::vector<Point<T>>;

template<class T>
Calc<T> signed_area2(const Polygon<T>& polygon) {
    Calc<T> result = 0;
    for (std::size_t i = 0; i < polygon.size(); ++i) {
        const auto& a = polygon[i];
        const auto& b = polygon[(i + 1) % polygon.size()];
        result += calc(a.x) * calc(b.y) - calc(a.y) * calc(b.x);
    }
    return result;
}

template<class T>
ld area(const Polygon<T>& polygon) {
    return std::abs(static_cast<ld>(signed_area2(polygon))) / 2.0L;
}

template<class T>
ld perimeter(const Polygon<T>& polygon) {
    if (polygon.size() <= 1) return 0;
    ld result = 0;
    for (std::size_t i = 0; i < polygon.size(); ++i) {
        result += distance(polygon[i], polygon[(i + 1) % polygon.size()]);
    }
    return result;
}

template<class T>
bool is_counter_clockwise(const Polygon<T>& polygon) {
    return sign<T>(signed_area2(polygon)) > 0;
}

// 既定では一直線上の頂点を許容するが、全頂点が一直線上の多角形は凸とみなさない。
template<class T>
bool is_convex(const Polygon<T>& polygon, bool allow_collinear = true) {
    if (polygon.size() < 3) return false;
    int direction = 0;
    for (std::size_t i = 0; i < polygon.size(); ++i) {
        const int turn = orientation(polygon[i], polygon[(i + 1) % polygon.size()],
                                     polygon[(i + 2) % polygon.size()]);
        if (turn == 0) {
            if (!allow_collinear) return false;
            continue;
        }
        if (direction != 0 && direction != turn) return false;
        direction = turn;
    }
    return direction != 0;
}

enum class PointInPolygon { Outside, Boundary, Inside };

// 単純多角形に対し、巻き数を用いて点の包含関係を判定する。
template<class T>
PointInPolygon point_in_polygon(const Polygon<T>& polygon, const Point<T>& p) {
    if (polygon.empty()) return PointInPolygon::Outside;
    if (polygon.size() == 1) {
        return equal(polygon.front(), p) ? PointInPolygon::Boundary
                                         : PointInPolygon::Outside;
    }
    if (polygon.size() == 2) {
        return on_segment(Segment<T>{polygon[0], polygon[1]}, p)
             ? PointInPolygon::Boundary : PointInPolygon::Outside;
    }
    int winding_number = 0;
    for (std::size_t i = 0; i < polygon.size(); ++i) {
        const Point<T>& a = polygon[i];
        const Point<T>& b = polygon[(i + 1) % polygon.size()];
        if (on_segment(Segment<T>{a, b}, p)) return PointInPolygon::Boundary;
        const int ay = sign<T>(calc(a.y) - calc(p.y));
        const int by = sign<T>(calc(b.y) - calc(p.y));
        const int turn = orientation(a, b, p);
        if (ay <= 0 && by > 0 && turn > 0) ++winding_number;
        if (by <= 0 && ay > 0 && turn < 0) --winding_number;
    }
    return winding_number == 0 ? PointInPolygon::Outside
                               : PointInPolygon::Inside;
}

// O(log N)。Nが3以上の場合、polygonは隣接する重複頂点を持たない非退化凸多角形を
// 反時計回りに並べたものとする。境界上の一直線に並ぶ頂点は許容し、
// 先頭の頂点は末尾に重ねない。
template<class T>
PointInPolygon point_in_convex_polygon(const Polygon<T>& polygon,
                                       const Point<T>& p) {
    if (polygon.size() <= 2) return point_in_polygon(polygon, p);

    const Point<T>& origin = polygon[0];
    if (equal(origin, p)) return PointInPolygon::Boundary;

    const int first_side = orientation(origin, polygon[1], p);
    const int last_side = orientation(origin, polygon.back(), p);
    if (first_side < 0 || last_side > 0) {
        return PointInPolygon::Outside;
    }

    const bool on_first_ray = first_side == 0
        && sign<T>(dot_from(origin, polygon[1], p)) > 0;
    const bool on_last_ray = last_side == 0
        && sign<T>(dot_from(origin, polygon.back(), p)) > 0;
    if (on_first_ray) {
        std::size_t low = 1, high = polygon.size() - 1;
        while (high - low > 1) {
            const std::size_t middle = std::midpoint(low, high);
            const bool same_ray = orientation(origin, polygon[1],
                                               polygon[middle]) == 0
                && sign<T>(dot_from(origin, polygon[1], polygon[middle])) > 0;
            if (same_ray) low = middle;
            else high = middle;
        }
        return on_segment(Segment<T>{origin, polygon[low]}, p)
             ? PointInPolygon::Boundary : PointInPolygon::Outside;
    }
    if (on_last_ray) {
        std::size_t low = 1, high = polygon.size() - 1;
        while (high - low > 1) {
            const std::size_t middle = std::midpoint(low, high);
            const bool same_ray = orientation(origin, polygon[middle],
                                               polygon.back()) == 0
                && sign<T>(dot_from(origin, polygon[middle], polygon.back())) > 0;
            if (same_ray) high = middle;
            else low = middle;
        }
        return on_segment(Segment<T>{origin, polygon[high]}, p)
             ? PointInPolygon::Boundary : PointInPolygon::Outside;
    }
    if (first_side == 0 || last_side == 0) {
        return PointInPolygon::Outside;
    }

    std::size_t low = 1, high = polygon.size() - 1;
    while (high - low > 1) {
        const std::size_t middle = std::midpoint(low, high);
        if (orientation(origin, polygon[middle], p) >= 0) low = middle;
        else high = middle;
    }

    const Segment<T> boundary_edge{polygon[low], polygon[high]};
    const int edge_side = orientation(boundary_edge.a, boundary_edge.b, p);
    if (edge_side < 0) return PointInPolygon::Outside;
    if (edge_side == 0) {
        return on_segment(boundary_edge, p) ? PointInPolygon::Boundary
                                            : PointInPolygon::Outside;
    }
    return PointInPolygon::Inside;
}

template<class T>
Point<ld> centroid(const Polygon<T>& polygon) {
    ld twice_area = 0;
    Point<ld> numerator{0, 0};
    for (std::size_t i = 0; i < polygon.size(); ++i) {
        const Point<ld> a = to_ld(polygon[i]);
        const Point<ld> b = to_ld(polygon[(i + 1) % polygon.size()]);
        const ld weight = cross(a, b);
        twice_area += weight;
        numerator += (a + b) * weight;
    }
    if (std::abs(twice_area) <= EPS) {
        throw std::invalid_argument("a centroid requires a non-zero-area polygon");
    }
    return numerator / (3.0L * twice_area);
}

// 重複しない頂点を辞書順最小の点から反時計回りに返す。先頭の頂点は末尾に重ねない。
template<class T>
Polygon<T> convex_hull(Polygon<T> points, bool keep_collinear = false) {
    std::sort(points.begin(), points.end());
    points.erase(std::unique(points.begin(), points.end(),
                             [](const Point<T>& a, const Point<T>& b) {
                                 return a == b;
                             }),
                 points.end());
    if (points.size() <= 2) return points;
    bool all_collinear = true;
    for (std::size_t i = 2; i < points.size(); ++i) {
        if (orientation(points[0], points[1], points[i]) != 0) {
            all_collinear = false;
            break;
        }
    }
    if (all_collinear) {
        if (keep_collinear) return points;
        return {points.front(), points.back()};
    }
    Polygon<T> lower, upper;
    const auto should_remove = [&](const Point<T>& a, const Point<T>& b,
                                   const Point<T>& c) {
        const int turn = orientation(a, b, c);
        return keep_collinear ? turn < 0 : turn <= 0;
    };
    for (const auto& p : points) {
        while (lower.size() >= 2
               && should_remove(lower[lower.size() - 2], lower.back(), p)) {
            lower.pop_back();
        }
        lower.push_back(p);
    }
    for (auto it = points.rbegin(); it != points.rend(); ++it) {
        while (upper.size() >= 2
               && should_remove(upper[upper.size() - 2], upper.back(), *it)) {
            upper.pop_back();
        }
        upper.push_back(*it);
    }
    lower.pop_back();
    upper.pop_back();
    lower.insert(lower.end(), upper.begin(), upper.end());
    return lower;
}

template<class T>
struct Circle {
    Point<T> center;
    T radius{};
};

template<class T>
void validate_circle(const Circle<T>& circle) {
    if (circle.radius < T{}) {
        throw std::invalid_argument("a circle requires a non-negative radius");
    }
}

template<class T>
Calc<T> squared_distance(const Point<T>& a, const Point<T>& b) {
    const auto dx = calc(a.x) - calc(b.x);
    const auto dy = calc(a.y) - calc(b.y);
    return dx * dx + dy * dy;
}

template<class T>
ld circumference(const Circle<T>& circle) {
    validate_circle(circle);
    return 2.0L * std::numbers::pi_v<ld> * static_cast<ld>(circle.radius);
}

template<class T>
ld area(const Circle<T>& circle) {
    validate_circle(circle);
    const ld radius = static_cast<ld>(circle.radius);
    return std::numbers::pi_v<ld> * radius * radius;
}

enum class PointCircleRelation { Inside, On, Outside };

template<class T>
PointCircleRelation relation(const Circle<T>& circle, const Point<T>& p) {
    validate_circle(circle);
    const auto radius = calc(circle.radius);
    const int comparison = sign<T>(squared_distance(circle.center, p)
                                    - radius * radius);
    if (comparison < 0) return PointCircleRelation::Inside;
    if (comparison > 0) return PointCircleRelation::Outside;
    return PointCircleRelation::On;
}

template<class T>
bool on_circle(const Circle<T>& circle, const Point<T>& p) {
    return relation(circle, p) == PointCircleRelation::On;
}

enum class CircleLineIntersectionType { None, Tangent, TwoPoints };

template<class T>
CircleLineIntersectionType intersection_type(const Circle<T>& circle,
                                             const Line<T>& line) {
    validate_circle(circle);
    if (is_degenerate(line)) {
        throw std::invalid_argument("a line requires two distinct points");
    }
    const auto dx = calc(line.b.x) - calc(line.a.x);
    const auto dy = calc(line.b.y) - calc(line.a.y);
    const auto cx = calc(circle.center.x) - calc(line.a.x);
    const auto cy = calc(circle.center.y) - calc(line.a.y);
    const auto determinant = dx * cy - dy * cx;
    const auto radius = calc(circle.radius);
    const int comparison = sign<T>(determinant * determinant
        - radius * radius * (dx * dx + dy * dy));
    if (comparison > 0) return CircleLineIntersectionType::None;
    if (comparison == 0) return CircleLineIntersectionType::Tangent;
    return CircleLineIntersectionType::TwoPoints;
}

template<class T>
CircleLineIntersectionType intersection_type(const Line<T>& line,
                                             const Circle<T>& circle) {
    return intersection_type(circle, line);
}

template<class T>
std::vector<Point<ld>> intersection_points(const Circle<T>& circle,
                                            const Line<T>& line) {
    const auto type = intersection_type(circle, line);
    if (type == CircleLineIntersectionType::None) return {};
    const Point<ld> foot = projection(line, circle.center);
    if (type == CircleLineIntersectionType::Tangent) return {foot};
    const Point<ld> direction = normalized(to_ld(line.b) - to_ld(line.a));
    const ld radius = static_cast<ld>(circle.radius);
    const ld center_to_line = distance(circle.center, line);
    const ld offset = std::sqrt(std::max<ld>(
        0, radius * radius - center_to_line * center_to_line));
    std::vector<Point<ld>> result{foot - direction * offset,
                                  foot + direction * offset};
    std::sort(result.begin(), result.end());
    return result;
}

template<class T>
std::vector<Point<ld>> intersection_points(const Line<T>& line,
                                            const Circle<T>& circle) {
    return intersection_points(circle, line);
}

template<class T>
std::vector<Point<ld>> intersection_points(const Circle<T>& circle,
                                            const Segment<T>& segment) {
    validate_circle(circle);
    if (is_degenerate(segment)) {
        return on_circle(circle, segment.a)
             ? std::vector<Point<ld>>{to_ld(segment.a)}
             : std::vector<Point<ld>>{};
    }
    std::vector<Point<ld>> result;
    const Segment<ld> converted{to_ld(segment.a), to_ld(segment.b)};
    for (const auto& point :
         intersection_points(circle, Line<T>{segment.a, segment.b})) {
        if (on_segment(converted, point)) result.push_back(point);
    }
    return result;
}

template<class T>
std::vector<Point<ld>> intersection_points(const Segment<T>& segment,
                                            const Circle<T>& circle) {
    return intersection_points(circle, segment);
}

enum class CircleIntersectionType {
    Separate, ExternallyTangent, TwoPoints, InternallyTangent, Contained,
    Coincident,
};

template<class T>
CircleIntersectionType intersection_type(const Circle<T>& lhs,
                                         const Circle<T>& rhs) {
    validate_circle(lhs);
    validate_circle(rhs);
    const bool same_center = equal(lhs.center, rhs.center);
    const bool same_radius = [&] {
        if constexpr (std::is_integral_v<T>) return lhs.radius == rhs.radius;
        else return equal(static_cast<ld>(lhs.radius),
                          static_cast<ld>(rhs.radius));
    }();
    if (same_center && same_radius) return CircleIntersectionType::Coincident;
    const auto distance2 = squared_distance(lhs.center, rhs.center);
    const auto radius_sum = calc(lhs.radius) + calc(rhs.radius);
    auto radius_difference = calc(lhs.radius) - calc(rhs.radius);
    if (radius_difference < 0) radius_difference = -radius_difference;
    const int outer = sign<T>(distance2 - radius_sum * radius_sum);
    if (outer > 0) return CircleIntersectionType::Separate;
    if (outer == 0) return CircleIntersectionType::ExternallyTangent;
    const int inner = sign<T>(distance2 - radius_difference * radius_difference);
    if (inner < 0) return CircleIntersectionType::Contained;
    if (inner == 0) return CircleIntersectionType::InternallyTangent;
    return CircleIntersectionType::TwoPoints;
}

// 一致する非退化円には交点が無限個存在する。
template<class T>
std::vector<Point<ld>> intersection_points(const Circle<T>& lhs,
                                            const Circle<T>& rhs) {
    const auto type = intersection_type(lhs, rhs);
    if (type == CircleIntersectionType::Coincident) {
        if (sign<T>(calc(lhs.radius)) == 0) return {to_ld(lhs.center)};
        return {};
    }
    if (type == CircleIntersectionType::Separate
        || type == CircleIntersectionType::Contained) return {};
    const Point<ld> lhs_center = to_ld(lhs.center);
    const Point<ld> difference = to_ld(rhs.center) - lhs_center;
    const ld center_distance = norm(difference);
    const ld lhs_radius = static_cast<ld>(lhs.radius);
    const ld rhs_radius = static_cast<ld>(rhs.radius);
    const ld along = (lhs_radius * lhs_radius - rhs_radius * rhs_radius
                      + center_distance * center_distance)
                   / (2.0L * center_distance);
    const Point<ld> base = lhs_center + difference * (along / center_distance);
    if (type == CircleIntersectionType::ExternallyTangent
        || type == CircleIntersectionType::InternallyTangent) return {base};
    const ld height = std::sqrt(std::max<ld>(
        0, lhs_radius * lhs_radius - along * along));
    const Point<ld> offset = rotate90(difference) * (height / center_distance);
    std::vector<Point<ld>> result{base - offset, base + offset};
    std::sort(result.begin(), result.end());
    return result;
}

template<class T>
std::vector<Point<ld>> tangent_points(const Circle<T>& circle,
                                      const Point<T>& p) {
    validate_circle(circle);
    const auto point_relation = relation(circle, p);
    if (point_relation == PointCircleRelation::Inside) return {};
    if (point_relation == PointCircleRelation::On) return {to_ld(p)};
    const Point<ld> center = to_ld(circle.center);
    const Point<ld> difference = to_ld(p) - center;
    const ld distance2 = norm2(difference);
    const ld radius = static_cast<ld>(circle.radius);
    const Point<ld> base = center + difference * (radius * radius / distance2);
    const Point<ld> offset = rotate90(difference)
        * (radius * std::sqrt(std::max<ld>(0, distance2 - radius * radius))
           / distance2);
    std::vector<Point<ld>> result{base - offset};
    if (!equal(base - offset, base + offset)) result.push_back(base + offset);
    std::sort(result.begin(), result.end());
    return result;
}

template<class T>
std::optional<Circle<ld>> circumcircle(const Point<T>& a, const Point<T>& b,
                                       const Point<T>& c) {
    if (orientation(a, b, c) == 0) return std::nullopt;
    const Point<ld> lhs = to_ld(a), rhs = to_ld(b), other = to_ld(c);
    const ld determinant = 2.0L * cross(rhs - lhs, other - lhs);
    const ld rhs_norm = norm2(rhs - lhs);
    const ld other_norm = norm2(other - lhs);
    const Point<ld> center{
        lhs.x + ((other.y - lhs.y) * rhs_norm
                 - (rhs.y - lhs.y) * other_norm) / determinant,
        lhs.y + ((rhs.x - lhs.x) * other_norm
                 - (other.x - lhs.x) * rhs_norm) / determinant,
    };
    return Circle<ld>{center, distance(center, lhs)};
}

} // namespace geometry
