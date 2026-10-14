#pragma once

#include <bits/stdc++.h>

namespace geometry_light {

using ll = long long;
using Real = double;

constexpr Real EPS = 1e-10;

#ifdef LOCAL
#define GEOMETRY_LIGHT_ASSERT(condition) assert(condition)
#else
#define GEOMETRY_LIGHT_ASSERT(condition) ((void)sizeof(condition))
#endif

// 整数点の入力座標は絶対値10^9以下とする。
// 外積、内積、距離の二乗、および面積の累積を含むすべての整数演算が
// long longに収まることは呼び出し側が保証する。
// 特に、計算で生成した座標、任意のスカラー倍、外積同士の積には
// 入力座標に対する保証がそのまま適用されない。
// 位相的な判定はlong long、距離、交点、射影、円の作図はdoubleで行う。

inline int sign(ll value) {
    return (value > 0) - (value < 0);
}

inline int sign(Real value, Real scale = 1.0) {
    const Real tolerance = EPS
        + 16.0 * std::numeric_limits<Real>::epsilon()
            * std::max(1.0, scale);
    return (value > tolerance) - (value < -tolerance);
}

inline int compare(Real lhs, Real rhs) {
    return sign(lhs - rhs, std::max(std::abs(lhs), std::abs(rhs)));
}

inline bool equal(Real lhs, Real rhs) {
    return compare(lhs, rhs) == 0;
}

template<class T>
struct Point {
    static_assert(std::same_as<T, ll> || std::same_as<T, Real>);

    T x{}, y{};

    Point() = default;
    Point(T x_, T y_) : x(x_), y(y_) {}

    Point operator+() const { return *this; }
    Point operator-() const { return {-x, -y}; }

    Point& operator+=(const Point& point) {
        x += point.x;
        y += point.y;
        return *this;
    }

    Point& operator-=(const Point& point) {
        x -= point.x;
        y -= point.y;
        return *this;
    }

    Point& operator*=(T scalar) {
        x *= scalar;
        y *= scalar;
        return *this;
    }

    Point& operator/=(T scalar) {
        x /= scalar;
        y /= scalar;
        return *this;
    }

    friend Point operator+(Point lhs, const Point& rhs) {
        return lhs += rhs;
    }

    friend Point operator-(Point lhs, const Point& rhs) {
        return lhs -= rhs;
    }

    friend Point operator*(Point point, T scalar) {
        return point *= scalar;
    }

    friend Point operator*(T scalar, Point point) {
        return point *= scalar;
    }

    friend Point operator/(Point point, T scalar) {
        return point /= scalar;
    }

    friend bool operator==(const Point&, const Point&) = default;

    friend bool operator<(const Point& lhs, const Point& rhs) {
        return std::tie(lhs.x, lhs.y) < std::tie(rhs.x, rhs.y);
    }
};

using IPoint = Point<ll>;
using FPoint = Point<Real>;

template<class T>
using Vector = Point<T>;

template<class T>
std::istream& operator>>(std::istream& stream, Point<T>& point) {
    return stream >> point.x >> point.y;
}

template<class T>
std::ostream& operator<<(std::ostream& stream, const Point<T>& point) {
    return stream << point.x << ' ' << point.y;
}

inline bool equal(const FPoint& lhs, const FPoint& rhs) {
    return equal(lhs.x, rhs.x) && equal(lhs.y, rhs.y);
}

inline bool equal(const IPoint& lhs, const IPoint& rhs) {
    return lhs == rhs;
}

inline FPoint to_double(const IPoint& point) {
    return {static_cast<Real>(point.x), static_cast<Real>(point.y)};
}

inline FPoint to_double(const FPoint& point) {
    return point;
}

inline IPoint manhattan_to_chebyshev(const IPoint& point) {
    return {point.x + point.y, point.x - point.y};
}

inline IPoint chebyshev_to_manhattan(const IPoint& point) {
    GEOMETRY_LIGHT_ASSERT((point.x + point.y) % 2 == 0);
    return {(point.x + point.y) / 2, (point.x - point.y) / 2};
}

inline ll manhattan_distance(const IPoint& lhs, const IPoint& rhs) {
    return std::abs(lhs.x - rhs.x) + std::abs(lhs.y - rhs.y);
}

inline ll chebyshev_distance(const IPoint& lhs, const IPoint& rhs) {
    return std::max(std::abs(lhs.x - rhs.x), std::abs(lhs.y - rhs.y));
}

inline ll manhattan_diameter(const std::vector<IPoint>& points) {
    if (points.size() <= 1) return 0;

    ll minimum_sum = points.front().x + points.front().y;
    ll maximum_sum = minimum_sum;
    ll minimum_difference = points.front().x - points.front().y;
    ll maximum_difference = minimum_difference;

    for (const auto& point : points) {
        const ll sum = point.x + point.y;
        const ll difference = point.x - point.y;
        minimum_sum = std::min(minimum_sum, sum);
        maximum_sum = std::max(maximum_sum, sum);
        minimum_difference = std::min(minimum_difference, difference);
        maximum_difference = std::max(maximum_difference, difference);
    }

    return std::max(maximum_sum - minimum_sum,
                    maximum_difference - minimum_difference);
}

inline ll chebyshev_diameter(const std::vector<IPoint>& points) {
    if (points.size() <= 1) return 0;

    ll minimum_x = points.front().x;
    ll maximum_x = points.front().x;
    ll minimum_y = points.front().y;
    ll maximum_y = points.front().y;

    for (const auto& point : points) {
        minimum_x = std::min(minimum_x, point.x);
        maximum_x = std::max(maximum_x, point.x);
        minimum_y = std::min(minimum_y, point.y);
        maximum_y = std::max(maximum_y, point.y);
    }

    return std::max(maximum_x - minimum_x, maximum_y - minimum_y);
}

template<class T>
T dot(const Vector<T>& lhs, const Vector<T>& rhs) {
    return lhs.x * rhs.x + lhs.y * rhs.y;
}

template<class T>
T cross(const Vector<T>& lhs, const Vector<T>& rhs) {
    return lhs.x * rhs.y - lhs.y * rhs.x;
}

template<class T>
T cross(const Point<T>& a, const Point<T>& b, const Point<T>& c) {
    return cross(b - a, c - a);
}

template<class T>
T dot_from(const Point<T>& origin, const Point<T>& lhs,
           const Point<T>& rhs) {
    return dot(lhs - origin, rhs - origin);
}

template<class T>
T norm2(const Vector<T>& vector) {
    return dot(vector, vector);
}

template<class T>
T squared_distance(const Point<T>& lhs, const Point<T>& rhs) {
    return norm2(lhs - rhs);
}

template<class T>
int cross_sign(const Vector<T>& lhs, const Vector<T>& rhs) {
    if constexpr (std::same_as<T, ll>) {
        return sign(cross(lhs, rhs));
    } else {
        const Real positive = lhs.x * rhs.y;
        const Real negative = lhs.y * rhs.x;
        return sign(positive - negative,
                    std::abs(positive) + std::abs(negative));
    }
}

template<class T>
int dot_sign(const Vector<T>& lhs, const Vector<T>& rhs) {
    if constexpr (std::same_as<T, ll>) {
        return sign(dot(lhs, rhs));
    } else {
        const Real x_product = lhs.x * rhs.x;
        const Real y_product = lhs.y * rhs.y;
        return sign(x_product + y_product,
                    std::abs(x_product) + std::abs(y_product));
    }
}

template<class T>
Real norm(const Vector<T>& vector) {
    const Real x = static_cast<Real>(vector.x);
    const Real y = static_cast<Real>(vector.y);
    return std::sqrt(x * x + y * y);
}

template<class T>
Real distance(const Point<T>& lhs, const Point<T>& rhs) {
    const Real x = static_cast<Real>(lhs.x) - static_cast<Real>(rhs.x);
    const Real y = static_cast<Real>(lhs.y) - static_cast<Real>(rhs.y);
    return std::sqrt(x * x + y * y);
}

template<class T>
Vector<T> rotate90(const Vector<T>& vector) {
    return {-vector.y, vector.x};
}

template<class T>
FPoint rotate(const Vector<T>& vector, Real radians) {
    const Real cosine = std::cos(radians);
    const Real sine = std::sin(radians);
    return {static_cast<Real>(vector.x) * cosine
                - static_cast<Real>(vector.y) * sine,
            static_cast<Real>(vector.x) * sine
                + static_cast<Real>(vector.y) * cosine};
}

template<class T>
Real argument(const Vector<T>& vector) {
    return std::atan2(static_cast<Real>(vector.y),
                      static_cast<Real>(vector.x));
}

template<class T>
FPoint normalized(const Vector<T>& vector) {
    const Real length = norm(vector);
    GEOMETRY_LIGHT_ASSERT(length > EPS);
    return {static_cast<Real>(vector.x) / length,
            static_cast<Real>(vector.y) / length};
}

template<class T>
bool same_direction(const Vector<T>& lhs, const Vector<T>& rhs) {
    if (equal(lhs, Vector<T>{}) || equal(rhs, Vector<T>{})) return false;
    return cross_sign(lhs, rhs) == 0 && dot_sign(lhs, rhs) > 0;
}

template<class T>
bool opposite_direction(const Vector<T>& lhs, const Vector<T>& rhs) {
    if (equal(lhs, Vector<T>{}) || equal(rhs, Vector<T>{})) return false;
    return cross_sign(lhs, rhs) == 0 && dot_sign(lhs, rhs) < 0;
}

inline int argument_partition(const IPoint& vector) {
    if (vector.y < 0) return 0;
    if (vector.y == 0 && vector.x >= 0) return 1;
    return 2;
}

inline bool argument_less(const IPoint& lhs, const IPoint& rhs) {
    const int lhs_partition = argument_partition(lhs);
    const int rhs_partition = argument_partition(rhs);
    if (lhs_partition != rhs_partition) {
        return lhs_partition < rhs_partition;
    }
    return cross(lhs, rhs) > 0;
}

inline bool argument_greater(const IPoint& lhs, const IPoint& rhs) {
    return argument_less(rhs, lhs);
}

inline bool argument_equal(const IPoint& lhs, const IPoint& rhs) {
    return !argument_less(lhs, rhs) && !argument_less(rhs, lhs);
}

inline void sort_by_argument(std::vector<IPoint>& points) {
    std::sort(points.begin(), points.end(), argument_less);
}

inline void sort_by_argument(std::vector<IPoint>& points,
                             const IPoint& origin) {
    std::sort(points.begin(), points.end(), [&](const IPoint& lhs,
                                                const IPoint& rhs) {
        const IPoint lhs_vector = lhs - origin;
        const IPoint rhs_vector = rhs - origin;
        const int lhs_partition = argument_partition(lhs_vector);
        const int rhs_partition = argument_partition(rhs_vector);
        if (lhs_partition != rhs_partition) {
            return lhs_partition < rhs_partition;
        }
        return cross(lhs_vector, rhs_vector) > 0;
    });
}

template<class T>
int orientation(const Point<T>& a, const Point<T>& b, const Point<T>& c) {
    return cross_sign(b - a, c - a);
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
    if (dot_sign(b - a, c - a) < 0) return Ccw::OnlineBack;
    if constexpr (std::same_as<T, ll>) {
        if (squared_distance(a, c) > squared_distance(a, b)) {
            return Ccw::OnlineFront;
        }
    } else if (compare(squared_distance(a, c), squared_distance(a, b)) > 0) {
        return Ccw::OnlineFront;
    }
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

using ILine = Line<ll>;
using FLine = Line<Real>;
using ISegment = Segment<ll>;
using FSegment = Segment<Real>;

template<class T>
bool is_degenerate(const Line<T>& line) {
    return equal(line.a, line.b);
}

template<class T>
bool is_degenerate(const Segment<T>& segment) {
    return equal(segment.a, segment.b);
}

template<class T>
bool on_line(const Line<T>& line, const Point<T>& point) {
    GEOMETRY_LIGHT_ASSERT(!is_degenerate(line));
    return orientation(line.a, line.b, point) == 0;
}

template<class T>
bool on_segment(const Segment<T>& segment, const Point<T>& point) {
    if (orientation(segment.a, segment.b, point) != 0) return false;
    return dot_sign(point - segment.a, point - segment.b) <= 0;
}

template<class T>
bool parallel(const Line<T>& lhs, const Line<T>& rhs) {
    GEOMETRY_LIGHT_ASSERT(!is_degenerate(lhs) && !is_degenerate(rhs));
    return cross_sign(lhs.b - lhs.a, rhs.b - rhs.a) == 0;
}

template<class T>
bool perpendicular(const Line<T>& lhs, const Line<T>& rhs) {
    GEOMETRY_LIGHT_ASSERT(!is_degenerate(lhs) && !is_degenerate(rhs));
    return dot_sign(lhs.b - lhs.a, rhs.b - rhs.a) == 0;
}

enum class LineIntersectionType {
    None,
    Point,
    Coincident,
};

template<class T>
LineIntersectionType intersection_type(const Line<T>& lhs,
                                       const Line<T>& rhs) {
    GEOMETRY_LIGHT_ASSERT(!is_degenerate(lhs) && !is_degenerate(rhs));
    if (!parallel(lhs, rhs)) return LineIntersectionType::Point;
    return orientation(lhs.a, lhs.b, rhs.a) == 0
        ? LineIntersectionType::Coincident : LineIntersectionType::None;
}

template<class T>
std::optional<FPoint> intersection_point(const Line<T>& lhs,
                                         const Line<T>& rhs) {
    if (intersection_type(lhs, rhs) != LineIntersectionType::Point) {
        return std::nullopt;
    }

    const Point<T> lhs_direction = lhs.b - lhs.a;
    const Point<T> rhs_direction = rhs.b - rhs.a;
    const T numerator = cross(rhs.a - lhs.a, rhs_direction);
    const T denominator = cross(lhs_direction, rhs_direction);
    const Real ratio = static_cast<Real>(numerator)
                     / static_cast<Real>(denominator);
    return to_double(lhs.a) + to_double(lhs_direction) * ratio;
}

template<class T>
FPoint projection(const Line<T>& line, const Point<T>& point) {
    GEOMETRY_LIGHT_ASSERT(!is_degenerate(line));
    const FPoint a = to_double(line.a);
    const FPoint direction = to_double(line.b) - a;
    const FPoint converted = to_double(point);
    const Real ratio = dot(converted - a, direction) / norm2(direction);
    return a + direction * ratio;
}

template<class T>
FPoint reflection(const Line<T>& line, const Point<T>& point) {
    const FPoint converted = to_double(point);
    return projection(line, point) * 2.0 - converted;
}

template<class T>
Real distance(const Point<T>& point, const Line<T>& line) {
    GEOMETRY_LIGHT_ASSERT(!is_degenerate(line));
    const FPoint a = to_double(line.a);
    const FPoint direction = to_double(line.b) - a;
    return std::abs(cross(direction, to_double(point) - a))
         / norm(direction);
}

template<class T>
Real distance(const Line<T>& line, const Point<T>& point) {
    return distance(point, line);
}

template<class T>
Real distance(const Point<T>& point, const Segment<T>& segment) {
    if (is_degenerate(segment)) return distance(point, segment.a);
    const FPoint a = to_double(segment.a);
    const FPoint direction = to_double(segment.b) - a;
    const FPoint converted = to_double(point);
    const Real ratio = dot(converted - a, direction) / norm2(direction);
    if (ratio <= 0) return distance(converted, a);
    if (ratio >= 1) return distance(converted, a + direction);
    return distance(converted, a + direction * ratio);
}

template<class T>
Real distance(const Segment<T>& segment, const Point<T>& point) {
    return distance(point, segment);
}

struct PointResult {
    std::array<FPoint, 2> points{};
    int count = 0;

    const FPoint& operator[](int index) const {
        return points[index];
    }

    FPoint& operator[](int index) {
        return points[index];
    }

    const FPoint* begin() const { return points.data(); }
    const FPoint* end() const { return points.data() + count; }

    void add(const FPoint& point) {
        for (int i = 0; i < count; ++i) {
            if (equal(points[i], point)) return;
        }
        GEOMETRY_LIGHT_ASSERT(count < 2);
        if (count == 2) return;
        points[count++] = point;
    }

    void sort() {
        if (count == 2 && points[1] < points[0]) {
            std::swap(points[0], points[1]);
        }
    }
};

template<class T>
bool intersects(const Segment<T>& lhs, const Segment<T>& rhs) {
    const int o1 = orientation(lhs.a, lhs.b, rhs.a);
    const int o2 = orientation(lhs.a, lhs.b, rhs.b);
    const int o3 = orientation(rhs.a, rhs.b, lhs.a);
    const int o4 = orientation(rhs.a, rhs.b, lhs.b);

    if (o1 == 0 && on_segment(lhs, rhs.a)) return true;
    if (o2 == 0 && on_segment(lhs, rhs.b)) return true;
    if (o3 == 0 && on_segment(rhs, lhs.a)) return true;
    if (o4 == 0 && on_segment(rhs, lhs.b)) return true;
    return o1 * o2 < 0 && o3 * o4 < 0;
}

template<class T>
PointResult intersection_points(const Segment<T>& lhs,
                                const Segment<T>& rhs) {
    PointResult result;

    if (on_segment(lhs, rhs.a)) result.add(to_double(rhs.a));
    if (on_segment(lhs, rhs.b)) result.add(to_double(rhs.b));
    if (on_segment(rhs, lhs.a)) result.add(to_double(lhs.a));
    if (on_segment(rhs, lhs.b)) result.add(to_double(lhs.b));

    if (result.count == 0 && intersects(lhs, rhs)) {
        const auto point = intersection_point(Line<T>{lhs.a, lhs.b},
                                              Line<T>{rhs.a, rhs.b});
        GEOMETRY_LIGHT_ASSERT(point.has_value());
        result.add(*point);
    }

    result.sort();
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
    const PointResult points = intersection_points(lhs, rhs);
    if (points.count == 0) return SegmentIntersectionType::None;
    if (points.count == 1) return SegmentIntersectionType::Point;
    return SegmentIntersectionType::Overlap;
}

template<class T>
Real distance(const Line<T>& lhs, const Line<T>& rhs) {
    if (intersection_type(lhs, rhs) != LineIntersectionType::None) return 0;
    return distance(lhs.a, rhs);
}

template<class T>
Real distance(const Segment<T>& lhs, const Segment<T>& rhs) {
    if (intersects(lhs, rhs)) return 0;
    return std::min({distance(lhs.a, rhs), distance(lhs.b, rhs),
                     distance(rhs.a, lhs), distance(rhs.b, lhs)});
}

template<class T>
FLine perpendicular_bisector(const Point<T>& lhs, const Point<T>& rhs) {
    GEOMETRY_LIGHT_ASSERT(!equal(lhs, rhs));
    const FPoint a = to_double(lhs);
    const FPoint b = to_double(rhs);
    const FPoint midpoint = (a + b) / 2.0;
    return {midpoint, midpoint + rotate90(b - a)};
}

template<class T>
FLine perpendicular_bisector(const Segment<T>& segment) {
    return perpendicular_bisector(segment.a, segment.b);
}

template<class T>
using Polygon = std::vector<Point<T>>;

using IPolygon = Polygon<ll>;
using FPolygon = Polygon<Real>;

inline ll signed_area2(const IPolygon& polygon) {
    if (polygon.empty()) return 0;
    ll result = 0;
    for (std::size_t i = 0; i < polygon.size(); ++i) {
        result += cross(polygon[i], polygon[(i + 1) % polygon.size()]);
    }
    return result;
}

inline Real signed_area2(const FPolygon& polygon) {
    if (polygon.empty()) return 0;
    Real result = 0;
    for (std::size_t i = 0; i < polygon.size(); ++i) {
        result += cross(polygon[i], polygon[(i + 1) % polygon.size()]);
    }
    return result;
}

template<class T>
Real area(const Polygon<T>& polygon) {
    return std::abs(static_cast<Real>(signed_area2(polygon))) / 2.0;
}

template<class T>
Real perimeter(const Polygon<T>& polygon) {
    if (polygon.size() <= 1) return 0;
    Real result = 0;
    for (std::size_t i = 0; i < polygon.size(); ++i) {
        result += distance(polygon[i], polygon[(i + 1) % polygon.size()]);
    }
    return result;
}

inline bool is_counter_clockwise(const IPolygon& polygon) {
    return signed_area2(polygon) > 0;
}

inline bool is_counter_clockwise(const FPolygon& polygon) {
    const Real twice_area = signed_area2(polygon);
    Real scale = 0;
    if (!polygon.empty()) {
        for (std::size_t i = 0; i < polygon.size(); ++i) {
            scale += std::abs(cross(polygon[i],
                                    polygon[(i + 1) % polygon.size()]));
        }
    }
    return sign(twice_area, scale) > 0;
}

template<class T>
bool is_convex(const Polygon<T>& polygon, bool allow_collinear = true) {
    if (polygon.size() < 3) return false;
    int direction = 0;
    for (std::size_t i = 0; i < polygon.size(); ++i) {
        const int turn = orientation(polygon[i],
                                     polygon[(i + 1) % polygon.size()],
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

enum class PointInPolygon {
    Outside,
    Boundary,
    Inside,
};

template<class T>
int compare_coordinate(T lhs, T rhs) {
    if constexpr (std::same_as<T, ll>) return sign(lhs - rhs);
    else return compare(lhs, rhs);
}

template<class T>
PointInPolygon point_in_polygon(const Polygon<T>& polygon,
                                const Point<T>& point) {
    if (polygon.empty()) return PointInPolygon::Outside;
    if (polygon.size() == 1) {
        return equal(polygon.front(), point)
            ? PointInPolygon::Boundary : PointInPolygon::Outside;
    }
    if (polygon.size() == 2) {
        return on_segment(Segment<T>{polygon[0], polygon[1]}, point)
            ? PointInPolygon::Boundary : PointInPolygon::Outside;
    }

    int winding_number = 0;
    for (std::size_t i = 0; i < polygon.size(); ++i) {
        const Point<T>& a = polygon[i];
        const Point<T>& b = polygon[(i + 1) % polygon.size()];
        if (on_segment(Segment<T>{a, b}, point)) {
            return PointInPolygon::Boundary;
        }
        const int a_y = compare_coordinate(a.y, point.y);
        const int b_y = compare_coordinate(b.y, point.y);
        const int turn = orientation(a, b, point);
        if (a_y <= 0 && b_y > 0 && turn > 0) ++winding_number;
        if (b_y <= 0 && a_y > 0 && turn < 0) --winding_number;
    }
    return winding_number == 0 ? PointInPolygon::Outside
                               : PointInPolygon::Inside;
}

template<class T>
PointInPolygon point_in_convex_polygon(const Polygon<T>& polygon,
                                       const Point<T>& point) {
    if (polygon.size() <= 2) return point_in_polygon(polygon, point);

    const Point<T>& origin = polygon[0];
    if (equal(origin, point)) return PointInPolygon::Boundary;

    const int first_side = orientation(origin, polygon[1], point);
    const int last_side = orientation(origin, polygon.back(), point);
    if (first_side < 0 || last_side > 0) return PointInPolygon::Outside;

    const bool on_first_ray = first_side == 0
        && dot_sign(polygon[1] - origin, point - origin) > 0;
    const bool on_last_ray = last_side == 0
        && dot_sign(polygon.back() - origin, point - origin) > 0;

    if (on_first_ray) {
        std::size_t low = 1;
        std::size_t high = polygon.size() - 1;
        while (high - low > 1) {
            const std::size_t middle = std::midpoint(low, high);
            const bool same_ray = orientation(origin, polygon[1],
                                              polygon[middle]) == 0
                && dot_sign(polygon[1] - origin,
                            polygon[middle] - origin) > 0;
            if (same_ray) low = middle;
            else high = middle;
        }
        return on_segment(Segment<T>{origin, polygon[low]}, point)
            ? PointInPolygon::Boundary : PointInPolygon::Outside;
    }

    if (on_last_ray) {
        std::size_t low = 1;
        std::size_t high = polygon.size() - 1;
        while (high - low > 1) {
            const std::size_t middle = std::midpoint(low, high);
            const bool same_ray = orientation(origin, polygon[middle],
                                              polygon.back()) == 0
                && dot_sign(polygon[middle] - origin,
                            polygon.back() - origin) > 0;
            if (same_ray) high = middle;
            else low = middle;
        }
        return on_segment(Segment<T>{origin, polygon[high]}, point)
            ? PointInPolygon::Boundary : PointInPolygon::Outside;
    }

    if (first_side == 0 || last_side == 0) {
        return PointInPolygon::Outside;
    }

    std::size_t low = 1;
    std::size_t high = polygon.size() - 1;
    while (high - low > 1) {
        const std::size_t middle = std::midpoint(low, high);
        if (orientation(origin, polygon[middle], point) >= 0) low = middle;
        else high = middle;
    }

    const Segment<T> edge{polygon[low], polygon[high]};
    const int edge_side = orientation(edge.a, edge.b, point);
    if (edge_side < 0) return PointInPolygon::Outside;
    if (edge_side == 0) {
        return on_segment(edge, point) ? PointInPolygon::Boundary
                                       : PointInPolygon::Outside;
    }

    return PointInPolygon::Inside;
}

template<class T>
FPoint centroid(const Polygon<T>& polygon) {
    Real twice_area = 0;
    FPoint numerator{};
    if (!polygon.empty()) {
        for (std::size_t i = 0; i < polygon.size(); ++i) {
            const FPoint a = to_double(polygon[i]);
            const FPoint b = to_double(polygon[(i + 1) % polygon.size()]);
            const Real weight = cross(a, b);
            twice_area += weight;
            numerator += (a + b) * weight;
        }
    }
    GEOMETRY_LIGHT_ASSERT(!equal(twice_area, 0.0));
    return numerator / (3.0 * twice_area);
}

inline IPolygon convex_hull(IPolygon points, bool keep_collinear = false) {
    std::sort(points.begin(), points.end());
    points.erase(std::unique(points.begin(), points.end()), points.end());
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

    IPolygon lower;
    IPolygon upper;
    const auto should_remove = [&](const IPoint& a, const IPoint& b,
                                   const IPoint& c) {
        const int turn = orientation(a, b, c);
        return keep_collinear ? turn < 0 : turn <= 0;
    };

    for (const auto& point : points) {
        while (lower.size() >= 2
               && should_remove(lower[lower.size() - 2], lower.back(), point)) {
            lower.pop_back();
        }
        lower.push_back(point);
    }

    for (auto iterator = points.rbegin(); iterator != points.rend(); ++iterator) {
        while (upper.size() >= 2
               && should_remove(upper[upper.size() - 2], upper.back(),
                                *iterator)) {
            upper.pop_back();
        }
        upper.push_back(*iterator);
    }

    lower.pop_back();
    upper.pop_back();
    lower.insert(lower.end(), upper.begin(), upper.end());
    return lower;
}

struct Circle {
    FPoint center;
    Real radius{};
};

inline void validate_circle(const Circle& circle) {
    GEOMETRY_LIGHT_ASSERT(circle.radius >= 0);
}

inline Real circumference(const Circle& circle) {
    validate_circle(circle);
    return 2.0 * std::numbers::pi_v<Real> * circle.radius;
}

inline Real area(const Circle& circle) {
    validate_circle(circle);
    return std::numbers::pi_v<Real> * circle.radius * circle.radius;
}

enum class PointCircleRelation {
    Inside,
    On,
    Outside,
};

template<class T>
PointCircleRelation relation(const Circle& circle, const Point<T>& point) {
    validate_circle(circle);
    const Real distance2 = squared_distance(circle.center, to_double(point));
    const Real radius2 = circle.radius * circle.radius;
    const int comparison = compare(distance2, radius2);
    if (comparison < 0) return PointCircleRelation::Inside;
    if (comparison > 0) return PointCircleRelation::Outside;
    return PointCircleRelation::On;
}

template<class T>
bool on_circle(const Circle& circle, const Point<T>& point) {
    return relation(circle, point) == PointCircleRelation::On;
}

enum class CircleLineIntersectionType {
    None,
    Tangent,
    TwoPoints,
};

template<class T>
CircleLineIntersectionType intersection_type(const Circle& circle,
                                             const Line<T>& line) {
    validate_circle(circle);
    GEOMETRY_LIGHT_ASSERT(!is_degenerate(line));
    const FLine converted{to_double(line.a), to_double(line.b)};
    const int comparison = compare(distance(circle.center, converted),
                                   circle.radius);
    if (comparison > 0) return CircleLineIntersectionType::None;
    if (comparison == 0) return CircleLineIntersectionType::Tangent;
    return CircleLineIntersectionType::TwoPoints;
}

template<class T>
CircleLineIntersectionType intersection_type(const Line<T>& line,
                                             const Circle& circle) {
    return intersection_type(circle, line);
}

template<class T>
PointResult intersection_points(const Circle& circle, const Line<T>& line) {
    PointResult result;
    const FLine converted{to_double(line.a), to_double(line.b)};
    const auto type = intersection_type(circle, converted);
    if (type == CircleLineIntersectionType::None) return result;

    const FPoint foot = projection(converted, circle.center);
    if (type == CircleLineIntersectionType::Tangent) {
        result.add(foot);
        return result;
    }

    const FPoint direction = normalized(converted.b - converted.a);
    const Real center_to_line = distance(circle.center, converted);
    const Real offset = std::sqrt(std::max(
        0.0, circle.radius * circle.radius
             - center_to_line * center_to_line));
    result.add(foot - direction * offset);
    result.add(foot + direction * offset);
    result.sort();
    return result;
}

template<class T>
PointResult intersection_points(const Line<T>& line, const Circle& circle) {
    return intersection_points(circle, line);
}

template<class T>
PointResult intersection_points(const Circle& circle,
                                const Segment<T>& segment) {
    PointResult result;
    validate_circle(circle);
    if (is_degenerate(segment)) {
        if (on_circle(circle, segment.a)) result.add(to_double(segment.a));
        return result;
    }

    const FSegment converted{to_double(segment.a), to_double(segment.b)};
    for (const FPoint& point :
         intersection_points(circle, FLine{converted.a, converted.b})) {
        if (on_segment(converted, point)) result.add(point);
    }
    result.sort();
    return result;
}

template<class T>
PointResult intersection_points(const Segment<T>& segment,
                                const Circle& circle) {
    return intersection_points(circle, segment);
}

enum class CircleIntersectionType {
    Separate,
    ExternallyTangent,
    TwoPoints,
    InternallyTangent,
    Contained,
    Coincident,
};

inline CircleIntersectionType intersection_type(const Circle& lhs,
                                                const Circle& rhs) {
    validate_circle(lhs);
    validate_circle(rhs);
    const Real center_distance = distance(lhs.center, rhs.center);
    if (equal(center_distance, 0.0) && equal(lhs.radius, rhs.radius)) {
        return CircleIntersectionType::Coincident;
    }

    const Real radius_sum = lhs.radius + rhs.radius;
    const Real radius_difference = std::abs(lhs.radius - rhs.radius);
    const int outer = compare(center_distance, radius_sum);
    if (outer > 0) return CircleIntersectionType::Separate;
    if (outer == 0) return CircleIntersectionType::ExternallyTangent;
    const int inner = compare(center_distance, radius_difference);
    if (inner < 0) return CircleIntersectionType::Contained;
    if (inner == 0) return CircleIntersectionType::InternallyTangent;
    return CircleIntersectionType::TwoPoints;
}

inline PointResult intersection_points(const Circle& lhs,
                                       const Circle& rhs) {
    PointResult result;
    const auto type = intersection_type(lhs, rhs);
    if (type == CircleIntersectionType::Coincident) {
        if (equal(lhs.radius, 0.0)) result.add(lhs.center);
        return result;
    }
    if (type == CircleIntersectionType::Separate
        || type == CircleIntersectionType::Contained) return result;

    const FPoint difference = rhs.center - lhs.center;
    const Real center_distance = norm(difference);
    const Real along = (lhs.radius * lhs.radius - rhs.radius * rhs.radius
                        + center_distance * center_distance)
                     / (2.0 * center_distance);
    const FPoint base = lhs.center
                      + difference * (along / center_distance);
    if (type == CircleIntersectionType::ExternallyTangent
        || type == CircleIntersectionType::InternallyTangent) {
        result.add(base);
        return result;
    }

    const Real height = std::sqrt(std::max(
        0.0, lhs.radius * lhs.radius - along * along));
    const FPoint offset = rotate90(difference)
                        * (height / center_distance);
    result.add(base - offset);
    result.add(base + offset);
    result.sort();
    return result;
}

template<class T>
PointResult tangent_points(const Circle& circle, const Point<T>& point) {
    PointResult result;
    const auto point_relation = relation(circle, point);
    if (point_relation == PointCircleRelation::Inside) return result;
    if (point_relation == PointCircleRelation::On) {
        result.add(to_double(point));
        return result;
    }

    const FPoint converted = to_double(point);
    const FPoint difference = converted - circle.center;
    const Real distance2 = norm2(difference);
    const FPoint base = circle.center
        + difference * (circle.radius * circle.radius / distance2);
    const FPoint offset = rotate90(difference)
        * (circle.radius * std::sqrt(std::max(
            0.0, distance2 - circle.radius * circle.radius)) / distance2);
    result.add(base - offset);
    result.add(base + offset);
    result.sort();
    return result;
}

template<class T>
std::optional<Circle> circumcircle(const Point<T>& a, const Point<T>& b,
                                   const Point<T>& c) {
    if (orientation(a, b, c) == 0) return std::nullopt;
    const FPoint lhs = to_double(a);
    const FPoint rhs = to_double(b);
    const FPoint other = to_double(c);
    const FPoint rhs_difference = rhs - lhs;
    const FPoint other_difference = other - lhs;
    const Real determinant = 2.0 * cross(rhs_difference, other_difference);
    const Real rhs_norm = norm2(rhs_difference);
    const Real other_norm = norm2(other_difference);
    const FPoint center{
        lhs.x + (other_difference.y * rhs_norm
                 - rhs_difference.y * other_norm) / determinant,
        lhs.y + (rhs_difference.x * other_norm
                 - other_difference.x * rhs_norm) / determinant,
    };
    return Circle{center, distance(center, lhs)};
}

#undef GEOMETRY_LIGHT_ASSERT

}
