#include "../include/GPath.h"
#include <cmath>

// Append a rectangle to the path
void GPath::addRect(const GRect &rect, Direction dir)
{
    this->moveTo(rect.left, rect.top);
    if (dir == Direction::kCW_Direction)
    {
        this->lineTo(rect.right, rect.top);
        this->lineTo(rect.right, rect.bottom);
        this->lineTo(rect.left, rect.bottom);
    }
    else
    {
        this->lineTo(rect.left, rect.bottom);
        this->lineTo(rect.right, rect.bottom);
        this->lineTo(rect.right, rect.top);
    }
    this->lineTo(rect.left, rect.top);
}

// Append a polygon to the path
void GPath::addPolygon(const GPoint pts[], int count)
{
    if (count > 0)
    {
        this->moveTo(pts[0]);
        for (int i = 1; i < count; ++i)
        {
            this->lineTo(pts[i]);
        }
    }
}

// static inline quad_points
static inline GRect quad_points(GPoint A, GPoint B, GPoint C)
{
    std::vector<GPoint> points;
    points.push_back(C);
    float left = A.x;
    float right = A.x;
    float top = A.y;
    float bottom = A.y;
    float denmo_x = A.x - (2 * B.x) + C.x;
    points.push_back((0.5f * A * 0.5f) + (2 * B * 0.5f * 0.5f) + (0.5f * C * 0.5f));
    if (denmo_x != 0)
    {
        float t = (A.x - B.x) / denmo_x;
        float t_inv = 1.0f - t;
        points.push_back((t_inv * A * t_inv) + (2 * B * t_inv * t) + (t * C * t));
    }
    float denmo_y = A.y - (2 * B.y) + C.y;
    if (denmo_y != 0)
    {
        float t = (A.y - B.y) / denmo_y;
        float t_inv = 1.0f - t;
        points.push_back((t_inv * A * t_inv) + (2 * B * t_inv * t) + (t * C * t));
    }
    for (const auto &pt : points)
    {
        left = std::min(left, pt.x);
        right = std::max(right, pt.x);
        top = std::min(top, pt.y);
        bottom = std::max(bottom, pt.y);
    }
    return GRect::LTRB(left, top, right, bottom);
}

static inline GRect cubic_points(GPoint A, GPoint B, GPoint C, GPoint D)
{
    std::vector<GPoint> points;
    points.push_back(D);
    float left = A.x;
    float right = A.x;
    float top = A.y;
    float bottom = A.y;
    points.push_back(0.5 * 0.5 * 0.5 * A + 3 * 0.5 * 0.5 * 0.5 * B + 3 * 0.5 * 0.5 * 0.5 * C + 0.5 * 0.5 * 0.5 * D);
    float denmo_x = (A.x - (3 * B.x) + (3 * C.x) - D.x);
    if (denmo_x != 0)
    {
        float to_sqrt = (-A.x * C.x) + A.x * D.x + B.x * B.x - B.x * C.x - B.x * D.x + C.x * C.x;
        // if (to_sqrt >= 0)
        // {
        float t_pos = (A.x - (2 * B.x) + C.x + sqrtf(to_sqrt)) / denmo_x;
        float t_neg = (A.x - (2 * B.x) + C.x - sqrtf(to_sqrt)) / denmo_x;
        if (t_pos <= 1 && t_pos >= 0)
        {
            float t_pos_inv = 1.0f - t_pos;
            points.push_back((t_pos_inv * A * t_pos_inv * t_pos_inv) + (3 * B * t_pos_inv * t_pos_inv * t_pos) + (3 * C * t_pos * t_pos_inv * t_pos) + (t_pos * D * t_pos * t_pos));
        }
        if (t_neg <= 1 && t_neg >= 0)
        {
            float t_neg_inv = 1.0f - t_neg;
            points.push_back((t_neg_inv * A * t_neg_inv * t_neg_inv) + (3 * B * t_neg_inv * t_neg * t_neg_inv) + (3 * C * t_neg * t_neg_inv * t_neg) + (t_neg * D * t_neg * t_neg));
        }
        // }
    }
    float denmo_y = (A.y - (3 * B.y) + (3 * C.y) - D.y);
    if (denmo_y != 0)
    {
        float to_sqrt = -A.y * C.y + A.y * D.y + B.y * B.y - B.y * C.y - B.y * D.y + C.y * C.y;
        // if (to_sqrt >= 0)
        // {
        float t_pos = (A.y - (2 * B.y) + C.y + sqrtf(to_sqrt)) / denmo_y;
        float t_neg = (A.y - (2 * B.y) + C.y - sqrtf(to_sqrt)) / denmo_y;
        if (t_pos <= 1 && t_pos >= 0)
        {
            float t_pos_inv = 1.0f - t_pos;
            points.push_back((t_pos_inv * A * t_pos_inv * t_pos_inv) + (3 * B * t_pos_inv * t_pos_inv * t_pos) + (3 * C * t_pos * t_pos_inv * t_pos) + (t_pos * D * t_pos * t_pos));
        }
        if (t_neg <= 1 && t_neg >= 0)
        {
            float t_neg_inv = 1.0f - t_neg;
            points.push_back((t_neg_inv * A * t_neg_inv * t_neg_inv) + (3 * B * t_neg_inv * t_neg * t_neg_inv) + (3 * C * t_neg * t_neg_inv * t_neg) + (t_neg * D * t_neg * t_neg));
        }
        // }
    }
    for (const auto &pt : points)
    {
        left = std::min(left, pt.x);
        right = std::max(right, pt.x);
        top = std::min(top, pt.y);
        bottom = std::max(bottom, pt.y);
    }
    return GRect::LTRB(left, top, right, bottom);
}

// Compute the bounds of the path
GRect GPath::bounds() const
{
    // return GRect::XYWH(0.0f, 0.0f, 0.0f, 0.0f);
    if (fPts.empty())
    {
        return GRect::XYWH(0.0f, 0.0f, 0.0f, 0.0f);
    }

    float left = 999999999999;
    float right = -1;
    float top = 999999999999;
    float bottom = -1;

    Edger edger = Edger(*this);
    GPoint points[GPath::kMaxNextPoints];
    while (auto verb = edger.next(points))
    {
        if (verb == GPath::kLine)
        {
            left = std::min(left, std::min(points[0].x, points[1].x));
            right = std::max(right, std::max(points[0].x, points[1].x));
            top = std::min(top, std::min(points[0].y, points[1].y));
            bottom = std::max(bottom, std::max(points[0].y, points[1].y));
        }
        else if (verb == GPath::kQuad)
        {
            GRect boundr = quad_points(points[0], points[1], points[2]);
            left = std::min(left, boundr.left);
            right = std::max(right, boundr.right);
            top = std::min(top, boundr.top);
            bottom = std::max(bottom, boundr.bottom);
        }
        else if (verb == GPath::kCubic)
        {
            GRect boundr = cubic_points(points[0], points[1], points[2], points[3]);
            left = std::min(left, boundr.left);
            right = std::max(right, boundr.right);
            top = std::min(top, boundr.top);
            bottom = std::max(bottom, boundr.bottom);
        }
        else
        {
            break;
        }
    };
    return GRect::LTRB(left, top, right, bottom);
}

// Transform the path in-place
void GPath::transform(const GMatrix &matrix)
{

    if (fPts.empty())
        return;

    std::vector<GPoint> transformedPoints(fPts.size());

    matrix.mapPoints(transformedPoints.data(), fPts.data(), static_cast<int>(fPts.size()));

    fPts = std::move(transformedPoints);
}

void GPath::addCircle(GPoint center, float radius, GPath::Direction direction)
{
    const float SQRT = 0.70710678118f * radius;
    const float TAN = 0.41421356237f * radius;

    if (direction == GPath::Direction::kCCW_Direction)
    {
        this->moveTo({center.x + radius, center.y});
        this->quadTo({center.x + radius, center.y - TAN}, {center.x + SQRT, center.y - SQRT});
        this->quadTo({center.x + TAN, center.y - radius}, {center.x, center.y - radius});
        this->quadTo({center.x - TAN, center.y - radius}, {center.x - SQRT, center.y - SQRT});
        this->quadTo({center.x - radius, center.y - TAN}, {center.x - radius, center.y});
        this->quadTo({center.x - radius, center.y + TAN}, {center.x - SQRT, center.y + SQRT});
        this->quadTo({center.x - TAN, center.y + radius}, {center.x, center.y + radius});
        this->quadTo({center.x + TAN, center.y + radius}, {center.x + SQRT, center.y + SQRT});
        this->quadTo({center.x + radius, center.y + TAN}, {center.x + radius, center.y});
    }
    else
    { // CCW direction
        this->moveTo({center.x + radius, center.y});
        this->quadTo({center.x + radius, center.y + TAN}, {center.x + SQRT, center.y + SQRT});
        this->quadTo({center.x + TAN, center.y + radius}, {center.x, center.y + radius});
        this->quadTo({center.x - TAN, center.y + radius}, {center.x - SQRT, center.y + SQRT});
        this->quadTo({center.x - radius, center.y + TAN}, {center.x - radius, center.y});
        this->quadTo({center.x - radius, center.y - TAN}, {center.x - SQRT, center.y - SQRT});
        this->quadTo({center.x - TAN, center.y - radius}, {center.x, center.y - radius});
        this->quadTo({center.x + TAN, center.y - radius}, {center.x + SQRT, center.y - SQRT});
        this->quadTo({center.x + radius, center.y - TAN}, {center.x + radius, center.y});
    }
};

void GPath::ChopCubicAt(const GPoint src[4], GPoint dst[7], float t)
{

    float prt = 1.0f - t;
    GPoint pointoncurve = prt * src[1] + t * src[2];

    dst[0] = src[0];
    dst[6] = src[3];
    dst[1] = prt * src[0] + t * src[1];
    dst[5] = prt * src[2] + t * src[3];
    dst[2] = prt * dst[1] + t * pointoncurve;
    dst[4] = prt * pointoncurve + t * dst[5];
    dst[3] = prt * dst[2] + t * dst[4];
};

void GPath::ChopQuadAt(const GPoint src[3], GPoint dst[5], float t)
{
    float prt = 1.0f - t;
    dst[0] = src[0];
    dst[4] = src[2];
    dst[1] = src[0] * prt + src[1] * t;
    dst[3] = src[1] * prt + src[2] * t;
    dst[2] = dst[1] * prt + dst[3] * t;
};