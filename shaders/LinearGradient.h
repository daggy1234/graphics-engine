#include "../include/GShader.h"
#include "../include/GBitmap.h"
#include "../include/GMatrix.h"
#include <vector>
#include <cmath>

static inline GPixel convert_pix(const GColor &color)
{
    int a = GRoundToInt((color.a * 255.0f));
    int r = GRoundToInt((color.a * color.r * 255.0f));
    int g = GRoundToInt((color.a * color.g * 255.0f));
    int b = GRoundToInt((color.a * color.b * 255.0f));
    return GPixel_PackARGB(a, r, g, b);
}

class LinearGradient : public GShader
{
    GPoint start;
    GPoint end;
    std::vector<GColor> colors;
    GMatrix fInverse;
    GMatrix localMatrix;
    int colorCount;
    float invLength;
    GTileMode tiling;

public:
    LinearGradient(GPoint start, GPoint end, const GColor inputColors[], int colCount, GTileMode tiling_m)
        : start(start), end(end), colors(inputColors, inputColors + colCount), colorCount(colCount - 1), tiling(tiling_m)
    {
        float dx = end.x - start.x;
        float dy = end.y - start.y;
        localMatrix = GMatrix(dx, -dy, start.x, dy, dx, start.y);
    }

    bool isOpaque() override
    {
        return false;
    }

    bool setContext(const GMatrix &ctm) override
    {

        auto matrixToInvert = ctm * localMatrix;

        auto inv = matrixToInvert.invert();
        if (inv)
        {
            fInverse = *inv;
            return true;
        }
        return false;
    }

    void shadeRow(int x, int y, int count, GPixel row[]) override
    {
        GPoint loc = fInverse * GPoint{x + 0.5f, y + 0.5f};
        const auto step_vec = fInverse.e0();

        if (colorCount == 0)
        {
            GPixel packed = convert_pix(colors[0]);
            for (int i = 0; i < count; ++i)
            {
                row[i] = packed;
            }
            return;
        }

        if (tiling == GTileMode::kRepeat)
        {
            for (int i = 0; i < count; ++i)
            {

                float clamped_t = std::max(0.0f, std::min(1.0f, loc.x - floor(loc.x)));
                int index = GFloorToInt(clamped_t * colorCount);
                float span = clamped_t * colorCount - index;
                auto c1 = colors[index];
                auto c2 = colors[index + 1];
                float rev_span = 1.0f - span;
                float alpha = span * c2.a + rev_span * c1.a;
                row[i] = GPixel_PackARGB(
                    GRoundToInt(alpha * 255.0f),
                    GRoundToInt((rev_span * c1.r + span * c2.r) * alpha * 255.0f),
                    GRoundToInt((rev_span * c1.g + span * c2.g) * alpha * 255.0f),
                    GRoundToInt((rev_span * c1.b + span * c2.b) * alpha * 255.0f));
                loc.x += step_vec.x;
            }
        }
        else if (tiling == GTileMode::kMirror)
        {
            for (int i = 0; i < count; ++i)
            {
                float pt = loc.x;
                pt *= 0.5;
                pt = pt - floor(pt);
                if (pt > 0.5)
                {
                    pt = 1 - pt;
                }
                pt *= 2;
                float clamped_t = std::max(0.0f, std::min(1.0f, pt));
                int index = GFloorToInt(clamped_t * colorCount);
                float span = clamped_t * colorCount - index;
                auto c1 = colors[index];
                auto c2 = colors[index + 1];
                float rev_span = 1.0f - span;
                float alpha = span * c2.a + rev_span * c1.a;
                row[i] = GPixel_PackARGB(
                    GRoundToInt(alpha * 255.0f),
                    GRoundToInt((rev_span * c1.r + span * c2.r) * alpha * 255.0f),
                    GRoundToInt((rev_span * c1.g + span * c2.g) * alpha * 255.0f),
                    GRoundToInt((rev_span * c1.b + span * c2.b) * alpha * 255.0f));
                loc.x += step_vec.x;
            }
        }
        else
        {

            for (int i = 0; i < count; ++i)
            {
                float clamped_t = std::max(0.0f, std::min(1.0f, loc.x));
                int index = GFloorToInt(clamped_t * colorCount);
                float span = clamped_t * colorCount - index;
                auto c1 = colors[index];
                auto c2 = colors[index + 1];
                float rev_span = 1.0f - span;
                float alpha = span * c2.a + rev_span * c1.a;
                row[i] = GPixel_PackARGB(
                    GRoundToInt(alpha * 255.0f),
                    GRoundToInt((rev_span * c1.r + span * c2.r) * alpha * 255.0f),
                    GRoundToInt((rev_span * c1.g + span * c2.g) * alpha * 255.0f),
                    GRoundToInt((rev_span * c1.b + span * c2.b) * alpha * 255.0f));
                loc.x += step_vec.x;
            }
        }
    }
};