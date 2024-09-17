#include "../include/GShader.h"
#include "../include/GBitmap.h"
#include "../include/GMatrix.h"
#include "../include/GColor.h"
#include "../include/GFinal.h"
#include "../utils/GColorMod.h"

static inline GPixel convert_pixc(const GColor &color)
{
    int a = GRoundToInt((color.a * 255.0f));
    int r = GRoundToInt((color.a * color.r * 255.0f));
    int g = GRoundToInt((color.a * color.g * 255.0f));
    int b = GRoundToInt((color.a * color.b * 255.0f));
    return GPixel_PackARGB(a, r, g, b);
}

class ColorMatrixShader : public GShader

{

    GColorMatrix colorMat;
    GShader *baseShader;

public:
    ColorMatrixShader(const GColorMatrix &mat, GShader *baseShader)
        : colorMat(mat), baseShader(baseShader) {}

    bool isOpaque() override
    {
        return baseShader->isOpaque();
    }

    bool setContext(const GMatrix &ctm) override
    {
        return baseShader->setContext(ctm);
    }

    void shadeRow(int x, int y, int count, GPixel row[]) override
    {
        baseShader->shadeRow(x, y, count, row);
        float pr_comp = 1.0f / 255.0f;
        for (int i = 0; i < count; i++)
        {
            if (GPixel_GetA(row[i]) == 0)
                continue;
            GPixel pre_col = row[i];
            // std::cout << GPixel_GetA(pre_col) << " " << GPixel_GetR(pre_col) << " " << GPixel_GetG(pre_col) << " " << GPixel_GetB(pre_col) << std::endl;
            float pre_mul_a = (float)GPixel_GetA(pre_col) * pr_comp;
            float alph = 1 / pre_mul_a;
            float pre_mul_r = (float)GPixel_GetR(pre_col) * alph * pr_comp;
            float pre_mul_g = (float)GPixel_GetG(pre_col) * alph * pr_comp;
            float pre_mul_b = (float)GPixel_GetB(pre_col) * alph * pr_comp;
            float new_r = std::min(1.0f, std::max(0.0f, colorMat[0] * pre_mul_r + colorMat[4] * pre_mul_g + colorMat[8] * pre_mul_b + colorMat[12] * pre_mul_a + colorMat[16]));
            float new_g = std::min(1.0f, std::max(0.0f, colorMat[1] * pre_mul_r + colorMat[5] * pre_mul_g + colorMat[9] * pre_mul_b + colorMat[13] * pre_mul_a + colorMat[17]));
            float new_b = std::min(1.0f, std::max(0.0f, colorMat[2] * pre_mul_r + colorMat[6] * pre_mul_g + colorMat[10] * pre_mul_b + colorMat[14] * pre_mul_a + colorMat[18]));
            float new_a = std::min(1.0f, std::max(0.0f, colorMat[3] * pre_mul_r + colorMat[7] * pre_mul_g + colorMat[11] * pre_mul_b + colorMat[15] * pre_mul_a + colorMat[19]));
            row[i] = convert_pixc(GColor::RGBA(new_r, new_g, new_b, new_a));
        }
    }
};