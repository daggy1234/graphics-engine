#include "include/GShader.h"
#include "include/GBitmap.h"
#include "include/GMatrix.h"

class TriangleCombined : public GShader
{

    GShader *colorShader;
    GShader *textureShader;

public:
    TriangleCombined(GShader *colorShader, GShader *textureShader)
        : colorShader(colorShader), textureShader(textureShader) {}

    bool isOpaque() override
    {
        return false;
    }

    bool setContext(const GMatrix &ctm) override
    {
        return colorShader->setContext(ctm) && textureShader->setContext(ctm);
    }

    void shadeRow(int x, int y, int count, GPixel row[]) override
    {
        GPixel colorRow[count + 1];
        GPixel textureRow[count + 1];
        colorShader->shadeRow(x, y, count, colorRow);
        textureShader->shadeRow(x, y, count, textureRow);
        auto div255 = 1 / 255.0f;
        for (int i = 0; i < count; i++)
        {
            GPixel color = colorRow[i];
            GPixel texture = textureRow[i];
            int a = GRoundToInt((GPixel_GetA(color) * GPixel_GetA(texture)) * div255);
            int r = GRoundToInt((GPixel_GetR(color) * GPixel_GetR(texture)) * div255);
            int g = GRoundToInt((GPixel_GetG(color) * GPixel_GetG(texture)) * div255);
            int b = GRoundToInt((GPixel_GetB(color) * GPixel_GetB(texture)) * div255);
            row[i] = GPixel_PackARGB(a, r, g, b);
        }
    }
};

static inline std::unique_ptr<GShader> makeTriangleCombined(GShader *colorShader, GShader *textureShader)
{
    return std::make_unique<TriangleCombined>(colorShader, textureShader);
}