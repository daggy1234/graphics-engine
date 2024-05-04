#include "include/GFinal.h"
#include "ColorMatrixShader.h"
#include "LinearPosShader.h"
#include "include/GPoint.h"

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
};

std::unique_ptr<GFinal>
GCreateFinal()
{
    return std::unique_ptr<GFinal>(new MyFinal);
}
