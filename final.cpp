#include "include/GFinal.h"
#include "shaders/ColorMatrixShader.h"
#include "shaders/LinearPosShader.h"
#include "include/GPoint.h"
#include "shaders/VeroniShader.h"

class MyFinal : public GFinal
{
public:
    std::unique_ptr<GShader> createColorMatrixShader(const GColorMatrix &mat, GShader *realShader) override
    {
        return std::make_unique<ColorMatrixShader>(ColorMatrixShader(mat, realShader));
    }

    std::unique_ptr<GShader> createLinearPosGradient(GPoint p0, GPoint p1,
                                                     const GColor colors[],
                                                     const float pos[],
                                                     int count) override
    {
        return std::make_unique<LinearPosGradient>(LinearPosGradient(p0, p1,
                                                                     colors,
                                                                     pos,
                                                                     count));
    }

    GPath strokePolygon(const GPoint points[], int count, float width, bool isClosed)
        override

    {
        GPath *npath = new GPath();
        if (count < 2)
        {
            return *npath;
        }

        for (int i = 0; i < count - 1; i++)
        {
            GPoint p0 = points[i];
            GPoint p1 = points[i + 1];

            // Rest of the code for processing the pair of points
            // ...

            float slope = (p1.y - p0.y) / (p1.x - p0.x);
            float perp = 1 / -slope;

            float corner_angle = atan(perp);

            float corner_dx = cos(corner_angle) * width / 2;
            float corner_dy = sin(corner_angle) * width / 2;

            GPoint c0 = GPoint{p0.x + corner_dx, p0.y + corner_dy};
            GPoint c1 = GPoint{p0.x - corner_dx, p0.y - corner_dy};
            GPoint c2 = GPoint{p1.x - corner_dx, p1.y - corner_dy};
            GPoint c3 = GPoint{p1.x + corner_dx, p1.y + corner_dy};
            npath->moveTo(c0);
            npath->lineTo(c1);
            npath->lineTo(c2);
            npath->lineTo(c3);

            return *npath;
        }
    }

    std::unique_ptr<GShader> createVoronoiShader(const GPoint points[],
                                                         const GColor colors[],
                                                         int count) override
    {

        return std::make_unique<VeroniShader>(VeroniShader(points, colors, count));
    }


};

std::unique_ptr<GFinal>
GCreateFinal()
{
    return std::unique_ptr<GFinal>(new MyFinal);
}
