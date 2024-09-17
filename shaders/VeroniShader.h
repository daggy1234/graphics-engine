//
// Created by Arnav Jindal on 9/17/24.
//


#include "../include/GShader.h"
#include "../include/GBitmap.h"
#include "../include/GMatrix.h"
#include "../include/GColor.h"
#include "../include/GFinal.h"

static inline GPixel convert_veroni(const GColor &color)
{
    int a = GRoundToInt((color.a * 255.0f));
    int r = GRoundToInt((color.a * color.r * 255.0f));
    int g = GRoundToInt((color.a * color.g * 255.0f));
    int b = GRoundToInt((color.a * color.b * 255.0f));
    return GPixel_PackARGB(a, r, g, b);
}

class VeroniShader : public GShader {
public:
    std::vector<GColor> colors;
    std::vector<GPoint> points;
    GMatrix localMatrix;
    GMatrix fInverse;
    GBitmap canvas;

    VeroniShader(const GPoint points_v[],const GColor colors_v[], int count): colors(colors_v, colors_v + count), points(points_v, points_v + count) {}


    bool isOpaque() override
    {
        return canvas.isOpaque();
    }

    bool setContext(const GMatrix &ctm) override
    {
        auto inv = (ctm * localMatrix).invert();
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
        for (int i = 0; i < count; ++i)
        {
            row[i] = findClosestColor(loc.x, loc.y);
            loc.x += step_vec.x;
        }
    }

    GPixel findClosestColor(float x, float y) {
        float minDistance = std::numeric_limits<float>::max();
        int closestIndex = 0;

        for (int i = 0; i < points.size(); ++i) {
            float dx = x - points[i].x;
            float dy = y - points[i].y;
            float distance = dx * dx + dy * dy;

            if (distance < minDistance) {
                minDistance = distance;
                closestIndex = i;
            }
        }

        // Return the color corresponding to the closest point
        return convert_veroni(colors[closestIndex]);
    }

};

