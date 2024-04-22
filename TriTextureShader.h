#include "include/GShader.h"
#include "include/GBitmap.h"
#include "include/GMatrix.h"

class TriTextureShader : public GShader
{
    GShader *baseShader;
    GMatrix fExtraTransform;

public:
    TriTextureShader(GShader *shader, const GMatrix &extraTransform)
        : baseShader(shader), fExtraTransform(extraTransform) {}

    bool isOpaque() override
    {
        return baseShader->isOpaque();
    }

    bool setContext(const GMatrix &ctm) override
    {
        return baseShader->setContext(ctm * fExtraTransform);
    }

    void shadeRow(int x, int y, int count, GPixel row[]) override
    {
        baseShader->shadeRow(x, y, count, row);
    }
};

static inline std::unique_ptr<GShader> makeTriTextureShader(GShader *shader, GPoint points[], GPoint texs[])
{
    GMatrix finalMat;
    auto texure_mat = GMatrix(texs[1].x - texs[0].x, texs[2].x - texs[0].x, texs[0].x,
                              texs[1].y - texs[0].y, texs[2].y - texs[0].y, texs[0].y);
    auto points_mat = GMatrix(points[1].x - points[0].x, points[2].x - points[0].x, points[0].x,
                              points[1].y - points[0].y, points[2].y - points[0].y, points[0].y);
    auto extraTransform = texure_mat.invert();
    if (extraTransform)
    {
        auto new_shader = TriTextureShader(shader, points_mat * *extraTransform);
        return std::make_unique<TriTextureShader>(new_shader);
    }
    return nullptr;
}