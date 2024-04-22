#include "include/GShader.h"
#include "include/GBitmap.h"
#include "include/GMatrix.h"

static inline GPixel convert_to_pix(const GColor &color)
{
    int a = GRoundToInt((color.a * 255.0f));
    int r = GRoundToInt((color.a * color.r * 255.0f));
    int g = GRoundToInt((color.a * color.g * 255.0f));
    int b = GRoundToInt((color.a * color.b * 255.0f));
    return GPixel_PackARGB(a, r, g, b);
}

class TriColorShader : public GShader
{
    GMatrix fInverse;
    GMatrix localMatrix;
    GColor fColors[3];
    GPoint fPoints[3];
    GColor DC0;
    GColor DC1;

public:
    TriColorShader(GColor cols[], GPoint points[])
    {
        localMatrix = GMatrix(points[1].x - points[0].x, points[2].x - points[0].x, points[0].x,
                              points[1].y - points[0].y, points[2].y - points[0].y, points[0].y);
        fColors[0] = cols[0];
        fColors[1] = cols[1];
        fColors[2] = cols[2];
        fPoints[0] = points[0];
        fPoints[1] = points[1];
        fPoints[2] = points[2];
    }

    bool isOpaque() override
    {
        return false;
    }

    bool setContext(const GMatrix &ctm) override
    {

        auto inv = (ctm * localMatrix).invert();
        if (!inv)
        {
            return false;
        }
        DC0 = GColor::RGBA(fColors[1].r - fColors[0].r, fColors[1].g - fColors[0].g, fColors[1].b - fColors[0].b, fColors[1].a - fColors[0].a);
        DC1 = GColor::RGBA(fColors[2].r - fColors[0].r, fColors[2].g - fColors[0].g, fColors[2].b - fColors[0].b, fColors[2].a - fColors[0].a);
        fInverse = *inv;
        return true;
    }

    void shadeRow(int x, int y, int count, GPixel row[]) override
    {
        GPoint f = fInverse * GPoint{x + 0.5f, y + 0.5f};
        GColor c = GColor::RGBA(
            std::max(std::min(1.0f, f.x * DC0.r + f.y * DC1.r + fColors[0].r), 0.0f),
            std::max(std::min(1.0f, f.x * DC0.g + f.y * DC1.g + fColors[0].g), 0.0f),
            std::max(std::min(1.0f, f.x * DC0.b + f.y * DC1.b + fColors[0].b), 0.0f),
            std::max(std::min(1.0f, f.x * DC0.a + f.y * DC1.a + fColors[0].a), 0.0f));
        row[0] = convert_to_pix(c);
        GColor dc = GColor::RGBA(DC0.r * fInverse[0] + DC1.r * fInverse[1], DC0.g * fInverse[0] + DC1.g * fInverse[1], DC0.b * fInverse[0] + DC1.b * fInverse[1], DC0.a * fInverse[0] + DC1.a * fInverse[1]);
        GColor iter_col = c;
        GColor temp;
        for (int i = 1; i < count; ++i)
        {
            temp = GColor::RGBA(
                std::max(std::min(1.0f, iter_col.r + dc.r), 0.0f),
                std::max(std::min(1.0f, iter_col.g + dc.g), 0.0f),
                std::max(std::min(1.0f, iter_col.b + dc.b), 0.0f),
                std::max(std::min(1.0f, iter_col.a + dc.a), 0.0f));
            iter_col = temp;
            row[i] = convert_to_pix(iter_col);
        }
    }
};

static inline std::unique_ptr<GShader> makeTriColorShader(GColor colors[], GPoint points[])
{
    auto shader = TriColorShader(colors, points);
    return std::make_unique<TriColorShader>(shader);
}
