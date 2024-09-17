#include "../include/GShader.h"
#include "../include/GBitmap.h"
#include "../include/GMatrix.h"

class BitmapShader : public GShader
{
    GBitmap canvas;
    GMatrix fInverse;
    GMatrix localMatrix;
    GTileMode tiling;

public:
    BitmapShader(GBitmap inp_canvas, GMatrix localMat, GTileMode tiling_m)
    {
        canvas = inp_canvas;
        localMatrix = localMat;
        tiling = tiling_m;
    }

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
        int width = canvas.width();
        int height = canvas.height();
        // float inv_width = 1 / width;
        // float inv_height = 1 / height;
        const auto step_vec = fInverse.e0();
        GPoint loc = fInverse * GPoint{x + 0.5f, y + 0.5f};

        if (tiling == GTileMode::kRepeat)
        {
            for (int i = 0; i < count; i++)
            {
                int source_x_c = GFloorToInt(loc.x) % width;
                int source_y_c = GFloorToInt(loc.y) % height;
                if (source_x_c < 0)
                {
                    source_x_c += width;
                }

                if (source_y_c < 0)
                {
                    source_y_c += height;
                }
                int x_bar = std::min(std::max(source_x_c, 0), width - 1);
                int y_bar = std::min(std::max(source_y_c, 0), height - 1);
                row[i] = *canvas.getAddr(x_bar, y_bar);
                loc += step_vec;
            }
        }
        else if (tiling == GTileMode::kMirror)
        {
            for (int i = 0; i < count; i++)
            {
                int source_x_c = GFloorToInt(loc.x) % (2 * width);
                int source_y_c = GFloorToInt(loc.y) % (2 * height);
                if (source_x_c < 0)
                {
                    source_x_c += 2 * width;
                }

                if (source_y_c < 0)
                {
                    source_y_c += 2 * height;
                }

                if (source_x_c >= width)
                {
                    source_x_c = 2 * width - 1 - source_x_c;
                }

                if (source_y_c >= height)
                {
                    source_y_c = 2 * height - 1 - source_y_c;
                }

                int x_bar = std::min(std::max(source_x_c, 0), width - 1);
                int y_bar = std::min(std::max(source_y_c, 0), height - 1);
                row[i] = *canvas.getAddr(x_bar, y_bar);
                loc += step_vec;
            }
        }
        else
        {
            for (int i = 0; i < count; i++)
            {
                int x_bar = std::min(std::max(GFloorToInt(loc.x), 0), width - 1);
                int y_bar = std::min(std::max(GFloorToInt(loc.y), 0), height - 1);
                row[i] = *canvas.getAddr(x_bar, y_bar);
                loc += step_vec;
            }
        }
    }
};