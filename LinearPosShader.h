#include "include/GShader.h"
#include "include/GBitmap.h"
#include "include/GMatrix.h"
#include <vector>
#include <cmath>

static inline GPixel convert_pixa(const GColor &color)
{
    int a = GRoundToInt((color.a * 255.0f));
    int r = GRoundToInt((color.a * color.r * 255.0f));
    int g = GRoundToInt((color.a * color.g * 255.0f));
    int b = GRoundToInt((color.a * color.b * 255.0f));
    return GPixel_PackARGB(a, r, g, b);
}

class LinearPosGradient : public GShader
{
    GPoint start;
    GPoint end;
    std::vector<float> pos;
    std::vector<GColor> colors;
    GMatrix fInverse;
    GMatrix localMatrix;
    int colorCount;

public:
    LinearPosGradient(GPoint p0, GPoint p1,
                      const GColor colors[],
                      const float pos[],
                      int count)
        : start(p0), end(p1), pos(pos, pos + count), colors(colors, colors + count), colorCount(count)
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

        if (colorCount == 1)
        {
            GPixel packed = convert_pixa(colors[0]);
            for (int i = 0; i < count; ++i)
            {
                row[i] = packed;
            }
            return;
        }
        else
        {
            for (int i = 0; i < count; ++i)
            {
                float t = std::max(0.0f, std::min(loc.x, 1.0f));
                int index = 0;
                for (int i = 0; i < colorCount - 1; i++)
                {
                    if (t <= pos[i + 1])
                    {
                        index = i;
                        break;
                    }
                }
                float range = pos[index + 1] - pos[index];
                float t_val = 0.0f;
                if (range != 0)
                {
                    t_val = (t - pos[index]) / range;
                }
                GColor colorf = GColor::RGBA(
                    colors[index].r * (1 - t_val) + colors[index + 1].r * t_val,
                    colors[index].g * (1 - t_val) + colors[index + 1].g * t_val,
                    colors[index].b * (1 - t_val) + colors[index + 1].b * t_val,
                    colors[index].a * (1 - t_val) + colors[index + 1].a * t_val);

                row[i] = convert_pixa(colorf);
                loc.x += step_vec.x;
            }
        }
    }
};