/*
 *  Copyright 2024 Daggy1234
 */
#include "starter_canvas.h"
#include "iostream"
#include <map>
#include "include/GShader.h"
#include "GPath.h"
#include <cassert>
#include "GColorMod.h"
#include "BitmapShader.h"
#include "LinearGradient.h"
#include "TriTextureShader.h"
#include "TriColorShader.h"
#include "TriCombined.h"

typedef GPixel (*BlendProc)(GColorMod, GColorMod);
typedef void (*BlendProcE)(int, int, int, GColorMod, GBitmap);

static inline GColorMod FromP(GPixel p)
{
    return GColorMod{
        GPixel_GetR(p),
        GPixel_GetG(p),
        GPixel_GetB(p),
        GPixel_GetA(p),
    };
}

static inline GColorMod convert_mod(const GColor &color)
{
    GColorMod ret{};
    ret.a = GRoundToInt((color.a * 255.0f));
    ret.r = GRoundToInt((color.a * color.r * 255.0f));
    ret.g = GRoundToInt((color.a * color.g * 255.0f));
    ret.b = GRoundToInt((color.a * color.b * 255.0f));
    return ret;
}

static inline GPixel convert(const GColor &color)
{
    int a = GRoundToInt((color.a * 255.0f));
    int r = GRoundToInt((color.a * color.r * 255.0f));
    int g = GRoundToInt((color.a * color.g * 255.0f));
    int b = GRoundToInt((color.a * color.b * 255.0f));
    return GPixel_PackARGB(a, r, g, b);
}

static inline void blit_row(int start_x, int start_y, int row_size, GColorMod converted, GBitmap canvas, BlendProc blend_mode)
{
    GPixel *pix = canvas.getAddr(start_x, start_y);
    int iter_n = row_size / 4;
    for (int i = 0; i < iter_n; i++)
    {
        pix[i * 4] = blend_mode(converted, {GPixel_GetR(pix[i * 4]), GPixel_GetG(pix[i * 4]), GPixel_GetB(pix[i * 4]), GPixel_GetA(pix[i * 4])});
        pix[i * 4 + 1] = blend_mode(converted, {GPixel_GetR(pix[i * 4 + 1]), GPixel_GetG(pix[i * 4 + 1]), GPixel_GetB(pix[i * 4 + 1]), GPixel_GetA(pix[i * 4 + 1])});
        pix[i * 4 + 2] = blend_mode(converted, {GPixel_GetR(pix[i * 4 + 2]), GPixel_GetG(pix[i * 4 + 2]), GPixel_GetB(pix[i * 4 + 2]), GPixel_GetA(pix[i * 4 + 2])});
        pix[i * 4 + 3] = blend_mode(converted, {GPixel_GetR(pix[i * 4 + 3]), GPixel_GetG(pix[i * 4 + 3]), GPixel_GetB(pix[i * 4 + 3]), GPixel_GetA(pix[i * 4 + 3])});
    }
    for (int k = iter_n * 4; k < row_size; k++)
    {
        pix[k] = blend_mode(converted, {GPixel_GetR(pix[k]), GPixel_GetG(pix[k]), GPixel_GetB(pix[k]), GPixel_GetA(pix[k])});
    }
}

static inline void blit_row_shader(int start_x, int start_y, int row_size, GColorMod converted, GPixel row[], GBitmap canvas, BlendProc blend_mode)
{
    GPixel *pix = canvas.getAddr(start_x, start_y);
    int iter_n = row_size / 4;
    for (int i = 0; i < iter_n; i++)
    {
        pix[i * 4] = blend_mode(FromP(row[i * 4]), {GPixel_GetR(pix[i * 4]), GPixel_GetG(pix[i * 4]), GPixel_GetB(pix[i * 4]), GPixel_GetA(pix[i * 4])});
        pix[i * 4 + 1] = blend_mode(FromP(row[i * 4 + 1]), {GPixel_GetR(pix[i * 4 + 1]), GPixel_GetG(pix[i * 4 + 1]), GPixel_GetB(pix[i * 4 + 1]), GPixel_GetA(pix[i * 4 + 1])});
        pix[i * 4 + 2] = blend_mode(FromP(row[i * 4 + 2]), {GPixel_GetR(pix[i * 4 + 2]), GPixel_GetG(pix[i * 4 + 2]), GPixel_GetB(pix[i * 4 + 2]), GPixel_GetA(pix[i * 4 + 2])});
        pix[i * 4 + 3] = blend_mode(FromP(row[i * 4 + 3]), {GPixel_GetR(pix[i * 4 + 3]), GPixel_GetG(pix[i * 4 + 3]), GPixel_GetB(pix[i * 4 + 3]), GPixel_GetA(pix[i * 4 + 3])});
    }
    for (int k = iter_n * 4; k < row_size; k++)
    {
        pix[k] = blend_mode(FromP(row[k]), {GPixel_GetR(pix[k]), GPixel_GetG(pix[k]), GPixel_GetB(pix[k]), GPixel_GetA(pix[k])});
    }
}

static inline int divSrcOver(int alph, int dest, int src)
{
    return (src) + (((alph * dest) + 128) * 257 >> 16);
}

static inline GPixel srcOver(const GColorMod src, const GColorMod dest)
{
    int inv_src_alpha = 255 - src.a;
    int a = divSrcOver(inv_src_alpha, dest.a, src.a);
    int r = divSrcOver(inv_src_alpha, dest.r, src.r);
    int g = divSrcOver(inv_src_alpha, dest.g, src.g);
    int b = divSrcOver(inv_src_alpha, dest.b, src.b);
    return GPixel_PackARGB(a, r, g, b);
    ;
}
static inline GPixel destOver(const GColorMod src, const GColorMod dest)
{
    return srcOver(dest, src);
}

static inline int divSrcIn(int alpha, int src)
{
    return ((src * alpha) + 128) * 257 >> 16;
}

static inline GPixel srcIn(const GColorMod src, const GColorMod dest)
{
    int a = divSrcIn(dest.a, src.a);
    int r = divSrcIn(dest.a, src.r);
    int g = divSrcIn(dest.a, src.g);
    int b = divSrcIn(dest.a, src.b);
    return GPixel_PackARGB(a, r, g, b);
    ;
}

static inline GPixel destIn(const GColorMod src, const GColorMod dest)
{
    return srcIn(dest, src);
}

static inline GPixel srcOut(const GColorMod src, const GColorMod dest)
{
    int inv_src_alpha = 255 - dest.a;
    int a = divSrcIn(inv_src_alpha, src.a);
    int r = divSrcIn(inv_src_alpha, src.r);
    int g = divSrcIn(inv_src_alpha, src.g);
    int b = divSrcIn(inv_src_alpha, src.b);
    return GPixel_PackARGB(a, r, g, b);
}

static inline GPixel destOut(const GColorMod src, const GColorMod dest)
{
    return srcOut(dest, src);
}

static inline int SrcTop(int alpha_a, int alpha_b, int src, int dest)
{
    return (((alpha_a * src) + 128) * 257 >> 16) + (((alpha_b * dest) + 128) * 257 >> 16);
}

static inline GPixel srcTop(const GColorMod src, const GColorMod dest)
{
    int inv_src_alpha = 255 - src.a;
    int dest_alpha = dest.a;
    int a = SrcTop(dest_alpha, inv_src_alpha, src.a, dest.a);
    int r = SrcTop(dest_alpha, inv_src_alpha, src.r, dest.r);
    int g = SrcTop(dest_alpha, inv_src_alpha, src.g, dest.g);
    int b = SrcTop(dest_alpha, inv_src_alpha, src.b, dest.b);
    return GPixel_PackARGB(a, r, g, b);
}

static inline GPixel destTop(const GColorMod src, const GColorMod dest)
{
    return srcTop(dest, src);
}

static inline GPixel kClear(const GColorMod src, const GColorMod dest)
{
    return GPixel_PackARGB(0u, 0u, 0u, 0u);
}

static inline GPixel kSrc(const GColorMod src, const GColorMod dest)
{
    return GPixel_PackARGB(src.a, src.r, src.g, src.b);
}
static inline GPixel kDst(const GColorMod src, const GColorMod dest)
{
    return GPixel_PackARGB(dest.a, dest.r, dest.g, dest.b);
}

static inline int divXor(int alpha_a, int alpha_b, int src, int dest)
{
    return ((((alpha_a * src)) + ((alpha_b * dest)) + 128) * 257 >> 16);
}

static inline GPixel srcXor(const GColorMod src, const GColorMod dest)
{
    int inv_src_alpha = 255 - src.a;
    int dest_alpha = 255 - dest.a;
    int a = divXor(inv_src_alpha, dest_alpha, dest.a, src.a);
    int r = divXor(inv_src_alpha, dest_alpha, dest.r, src.r);
    int g = divXor(inv_src_alpha, dest_alpha, dest.g, src.g);
    int b = divXor(inv_src_alpha, dest_alpha, dest.b, src.b);
    return GPixel_PackARGB(a, r, g, b);
}

static inline void clear_efficient(int start_x, int start_y, int row_size, GColorMod converted, GBitmap canvas)
{
    blit_row(start_x, start_y, row_size, converted, canvas, kClear);
}

static inline void src_efficient(int start_x, int start_y, int row_size, GColorMod converted, GBitmap canvas)
{
    if (converted.a == 0)
    {
        return blit_row(start_x, start_y, row_size, converted, canvas, kClear);
    }
    blit_row(start_x, start_y, row_size, converted, canvas, kSrc);
}

static inline void dst_efficient(int start_x, int start_y, int row_size, GColorMod converted, GBitmap canvas) {}

static inline void src_over_efficient(int start_x, int start_y, int row_size, GColorMod converted, GBitmap canvas)
{
    if (converted.a == 255)
    {
        return blit_row(start_x, start_y, row_size, converted, canvas, kSrc);
    }
    if (converted.a == 0)
    {
        return;
    }
    return blit_row(start_x, start_y, row_size, converted, canvas, srcOver);
}

static inline void dst_over_efficient(int start_x, int start_y, int row_size, GColorMod converted, GBitmap canvas)
{
    if (converted.a == 0)
    {
        return;
    }
    return blit_row(start_x, start_y, row_size, converted, canvas, destOver);
}

static inline void src_in_efficient(int start_x, int start_y, int row_size, GColorMod converted, GBitmap canvas)
{
    if (converted.a == 0)
    {
        return blit_row(start_x, start_y, row_size, converted, canvas, kClear);
    }
    blit_row(start_x, start_y, row_size, converted, canvas, srcIn);
}

static inline void dst_in_efficient(int start_x, int start_y, int row_size, GColorMod converted, GBitmap canvas)
{
    if (converted.a == 0)
    {
        return blit_row(start_x, start_y, row_size, converted, canvas, kClear);
    }
    if (converted.a == 255)
    {
        return;
    }
    blit_row(start_x, start_y, row_size, converted, canvas, destIn);
}

static inline void src_out_efficient(int start_x, int start_y, int row_size, GColorMod converted, GBitmap canvas)
{
    if (converted.a == 0)
    {
        return blit_row(start_x, start_y, row_size, converted, canvas, kClear);
    }
    blit_row(start_x, start_y, row_size, converted, canvas, srcOut);
}

static inline void dst_out_efficient(int start_x, int start_y, int row_size, GColorMod converted, GBitmap canvas)
{
    if (converted.a == 255)
    {
        return blit_row(start_x, start_y, row_size, converted, canvas, kClear);
    }
    if (converted.a == 0)
    {
        return;
    }
    blit_row(start_x, start_y, row_size, converted, canvas, destOut);
}

static inline void src_top_efficient(int start_x, int start_y, int row_size, GColorMod converted, GBitmap canvas)
{
    if (converted.a == 255)
    {
        return blit_row(start_x, start_y, row_size, converted, canvas, srcIn);
    }
    if (converted.a == 0)
    {
        return;
    }
    blit_row(start_x, start_y, row_size, converted, canvas, srcTop);
}

static inline void dst_top_efficient(int start_x, int start_y, int row_size, GColorMod converted, GBitmap canvas)
{

    if (converted.a == 0)
    {
        return blit_row(start_x, start_y, row_size, converted, canvas, kClear);
    }
    blit_row(start_x, start_y, row_size, converted, canvas, destTop);
}

static inline void src_xor_efficient(int start_x, int start_y, int row_size, GColorMod converted, GBitmap canvas)
{
    if (converted.a == 255)
    {
        return blit_row(start_x, start_y, row_size, converted, canvas, srcOut);
    }
    if (converted.a == 0)
    {
        return;
    }
    blit_row(start_x, start_y, row_size, converted, canvas, srcXor);
}

const BlendProc gProcs[] = {
    kClear, kSrc, kDst, srcOver, destOver, srcIn, destIn, srcOut, destOut, srcTop, destTop, srcXor};

const BlendProcE gProcE[] = {
    clear_efficient, src_efficient, dst_efficient, src_over_efficient, dst_over_efficient, src_in_efficient,
    dst_in_efficient, src_out_efficient, dst_out_efficient, src_top_efficient, dst_top_efficient, src_xor_efficient};

void MyCanvas::clear(const GColor &color)
{
    GPixel packed = convert(color);
    int height = fDevice.height();
    int width = fDevice.width();
    for (int j = 0; j < height; j++)
    {
        for (int i = 0; i < width; i++)
        {
            GPixel *pixel = fDevice.getAddr(i, j);
            *pixel = packed;
        }
    }
}

void MyCanvas::save()
{
    GMatrix top = CTM.top();
    CTM.push(top);
    ctm_no++;
}

void MyCanvas::restore()
{
    CTM.pop();
    ctm_no--;
}

void MyCanvas::concat(const GMatrix &matrix)
{
    GMatrix proc = GMatrix::Concat(CTM.top(), matrix);
    CTM.pop();
    CTM.push(proc);
    if (ctm_no == 0)
    {
        ctm_no++;
    }
}

void MyCanvas::drawRect(const GRect &rect, const GPaint &paint)
{
    if (ctm_no > 0)
    {
        GPoint p = GPoint();
        p.x = rect.x();
        p.y = rect.y();
        GPoint q = GPoint();
        q.x = rect.x() + rect.width();
        q.y = rect.y();
        GPoint r = GPoint();
        r.x = rect.x() + rect.width();
        r.y = rect.y() + rect.height();
        GPoint s = GPoint();
        s.x = rect.x();
        s.y = rect.y() + rect.height();
        GPoint arr[4]{
            p, q, r, s};
        MyCanvas::drawConvexPolygon(arr, 4, paint);
        return;
    }
    GBlendMode blend_mode = paint.getBlendMode();
    auto shader = paint.getShader();
    GColor color = paint.getColor();
    GColorMod converted = convert_mod(color);
    GIRect processed = rect.round();
    int w = processed.width();
    int h = processed.height();
    int max_h = fDevice.height();
    int max_w = fDevice.width();
    int x = processed.x();
    int y = processed.y();
    int rect_x_start = std::max(0, x);
    int rect_x_end = std::min(x + w, max_w);
    int rect_y_start = std::max(0, y);
    int rect_y_end = std::min(y + h, max_h);

    if (rect_x_start >= max_w)
    {
        return;
    }
    if (shader == nullptr)
    {
        BlendProcE proc = gProcE[(int)blend_mode];
        for (int j = rect_y_start; j < rect_y_end; j++)
        {
            proc(rect_x_start, j, rect_x_end - rect_x_start, converted, this->fDevice);
        }
        return;
    }

    if (!shader->setContext(CTM.top()))
    {
        return;
    }

    if (shader->isOpaque())
    {
        for (int j = rect_y_start; j < rect_y_end; j++)
        {
            int count = rect_x_end - rect_x_start;
            GPixel points[count];
            shader->shadeRow(rect_x_start, j, count, points);
            blit_row_shader(rect_x_start, j, count, converted, points, fDevice, kSrc);
        }
        return;
    }
    BlendProc proc = gProcs[(int)blend_mode];
    for (int j = rect_y_start; j < rect_y_end; j++)
    {
        int count = rect_x_end - rect_x_start;
        GPixel points[count];
        shader->shadeRow(rect_x_start, j, count, points);
        blit_row_shader(rect_x_start, j, count, converted, points, fDevice, proc);
    }
}

void MyCanvas::drawConvexPolygon(const GPoint points[], int count, const GPaint &paint)
{

    if (count <= 2)
    {
        return;
    }

    GPoint dest[count];
    if (ctm_no != 0)
    {
        CTM.top().mapPoints(dest, points, count);
    }
    else
    {
        std::copy(points, points + count, dest);
    }

    GColorMod converted = convert_mod(paint.getColor());
    GBlendMode blend_mode = paint.getBlendMode();
    if (blend_mode == GBlendMode::kDst)
    {
        return;
    }
    auto max_h = fDevice.height();
    auto max_hg = fDevice.height();
    int max_ww = fDevice.width();
    std::vector<std::pair<int, int>> pointsmap(fDevice.height(), {std::numeric_limits<int>::max(), std::numeric_limits<int>::min()});

    for (int i = 0; i < count; i++)
    {
        GPoint first_point = dest[i];
        GPoint next_point = dest[(i + 1) % count];
        GPoint bottom, top;
        if (next_point.y < first_point.y)
        {
            bottom = first_point;
            top = next_point;
        }
        else
        {
            bottom = next_point;
            top = first_point;
        }

        bool vertical = top.x == bottom.x;
        bool horizontal = top.y == bottom.y;
        float gradient = 0.0f;
        float intercept = 0.0f;
        if (!vertical)
        {
            gradient = ((top.y - bottom.y) / (top.x - bottom.x));
            intercept = top.y - ((float)gradient * top.x);
        }

        if (bottom.y < 0)
        {
            continue;
        }
        if (top.y > max_h)
        {
            continue;
        }
        if (bottom.y >= max_h)
        {
            bottom.y = max_h;
            if (!vertical && !horizontal)
            {
                bottom.x = (bottom.y - intercept) / gradient;
            }
        }
        if (top.y < 0)
        {
            top.y = 0;
            if (!vertical && !horizontal)
            {
                top.x = (top.y - intercept) / gradient;
            }
        }

        if (top.x == bottom.x && top.y == bottom.y)
        {
            continue;
        }
        int bot_y = GRoundToInt(bottom.y - 1.0f);
        int top_y = GRoundToInt(top.y);

        if (!horizontal)
        {
            for (int y = bot_y; y >= top_y; y--)
            {
                int x = 0;
                if (vertical)
                {
                    x = GRoundToInt(top.x);
                }
                else
                {
                    x = GRoundToInt((float)(((float)y + 0.5f) - intercept) / gradient);
                }
                if (x < 0)
                {
                    x = 0;
                }

                if (x >= max_ww)
                {
                    x = max_ww - 1; //
                    //                    x = max_ww; // Prevents 100% color clock
                }

                if (pointsmap[y].first == std::numeric_limits<int>::max() && pointsmap[y].second == std::numeric_limits<int>::min())
                {
                    pointsmap[y] = std::make_pair(x, x);
                }
                else
                {
                    pointsmap[y].first = std::min(pointsmap[y].first, x);
                    pointsmap[y].second = std::max(pointsmap[y].second, x);
                }
            }
        }
    }
    auto shader = paint.getShader();
    if (shader == nullptr)
    {
        BlendProcE proc = gProcE[(int)blend_mode];
        for (int y = 0; y < max_hg; ++y)
        {
            const auto &[start_x, end_x] = pointsmap[y];

            if (start_x != std::numeric_limits<int>::max() && end_x != std::numeric_limits<int>::min())
            {
                proc(start_x, y, end_x - start_x, converted, fDevice);
            }
        }
        return;
    }
    BlendProc proc = gProcs[(int)blend_mode];
    if (!(shader->setContext(CTM.top())))
    {
        return;
    }

    if (shader->isOpaque())
    {
        for (int y = 0; y < max_hg; ++y)
        {
            const auto &[start_x, end_x] = pointsmap[y];
            if (start_x != std::numeric_limits<int>::max() && end_x != std::numeric_limits<int>::min())
            {
                int count = end_x - start_x;
                if (count > 0)
                {
                    GPixel points[count];
                    shader->shadeRow(start_x, y, count, points);
                    blit_row_shader(start_x, y, end_x - start_x, converted, points, fDevice, kSrc);
                }
            }
        }
        return;
    }

    for (int y = 0; y < max_hg; ++y)
    {
        const auto &[start_x, end_x] = pointsmap[y];
        if (start_x != std::numeric_limits<int>::max() && end_x != std::numeric_limits<int>::min())
        {
            int count = end_x - start_x;
            GPixel points[count];
            shader->shadeRow(start_x, y, count, points);
            blit_row_shader(start_x, y, end_x - start_x, converted, points, fDevice, proc);
        }
    }
}

std::unique_ptr<GCanvas> GCreateCanvas(const GBitmap &device)
{
    return std::unique_ptr<GCanvas>(new MyCanvas(device));
}

void MyCanvas::drawLine(GPoint first_point, GPoint next_point)
{
    int max_h = fDevice.height();
    int max_ww = fDevice.width();
    GPoint bottom, top;
    int winding = 0;
    if (next_point.y < first_point.y)
    {
        bottom = first_point;
        top = next_point;
        winding = 1;
    }
    else
    {
        bottom = next_point;
        top = first_point;
        winding = -1;
    }

    bool vertical = top.x == bottom.x;
    bool horizontal = top.y == bottom.y;
    float gradient = 0.0f;
    float intercept = 0.0f;
    if (!vertical)
    {
        gradient = ((top.y - bottom.y) / (top.x - bottom.x));
        intercept = top.y - ((float)gradient * top.x);
    }

    if (bottom.y < 0)
    {
        return;
    }
    if (top.y > max_h)
    {
        return;
    }
    if (bottom.y >= max_h)
    {
        bottom.y = max_h;
        if (!vertical && !horizontal)
        {
            bottom.x = (bottom.y - intercept) / gradient;
        }
    }
    if (top.y < 0)
    {
        top.y = 0;
        if (!vertical && !horizontal)
        {
            top.x = (top.y - intercept) / gradient;
        }
    }

    if (top.x == bottom.x && top.y == bottom.y)
    {
        return;
    }
    int bot_y = GRoundToInt(bottom.y - 1.0f);
    int top_y = GRoundToInt(top.y);
    if (!horizontal)
    {
        for (int y = bot_y; y >= top_y; y--)
        {
            int x = 0;
            if (vertical)
            {
                x = GRoundToInt(top.x);
            }
            else
            {
                x = GRoundToInt((float)(((float)y + 0.5f) - intercept) / gradient);
            }
            if (x < 0)
            {
                x = 0;
            }

            if (x >= max_ww)
            {
                x = max_ww - 1;
            }

            pointsmap[y].push_back(std::make_pair(x, winding));
        }
    }
}

void MyCanvas::drawPath(const GPath &path, const GPaint &paint)
{

    GPath transformedPath = path;
    if (ctm_no != 0)
    {
        transformedPath.transform(CTM.top());
    }

    GColorMod converted = convert_mod(paint.getColor());
    GBlendMode blend_mode = paint.getBlendMode();
    if (blend_mode == GBlendMode::kDst)
    {
        return;
    }
    auto max_h = fDevice.height();
    auto max_hg = fDevice.height();
    int max_ww = fDevice.width();
    pointsmap.assign(max_h, std::vector<std::pair<int, int>>{});
    GPoint points[GPath::kMaxNextPoints];
    GPath::Edger edger(transformedPath);

    while (auto verb = edger.next(points))
    {
        // Line
        if (verb.value() == 1)
        {
            GPoint first_point = points[0];
            GPoint next_point = points[1];
            drawLine(first_point, next_point);
        }
        else if (verb.value() == 2)
        {
            GPoint A = points[0];
            GPoint B = points[1];
            GPoint C = points[2];

            GPoint vect = (A - 2 * B + C) * 0.25;
            float magnitude = sqrt(vect.x * vect.x + vect.y * vect.y);
            int seg_count = GCeilToInt(sqrt(magnitude) * 2);
            float iter_c = 1.0f / seg_count;
            GPoint segs[seg_count + 1];
            GPoint dst[5];
            for (int i = 0; i <= seg_count; ++i)
            {
                float t = i * iter_c;
                GPath::ChopQuadAt(points, dst, t);
                segs[i] = dst[2];
            }
            for (int i = 0; i < seg_count; i++)
            {
                drawLine(segs[i], segs[i + 1]);
            }
        }
        else if (verb.value() == 3)
        {
            GPoint A = points[0];
            GPoint B = points[1];
            GPoint C = points[2];
            GPoint D = points[3];

            GPoint vect_a = (A - 2 * B + C) * 0.25;
            GPoint vect_b = (B - 2 * C + D) * 0.25;
            float magnitude_a = sqrt(vect_a.x * vect_a.x + vect_a.y * vect_a.y);
            float magnitude_b = sqrt(vect_b.x * vect_b.x + vect_b.y * vect_b.y);
            int seg_count = GCeilToInt(sqrt(0.75 * std::max(magnitude_a, magnitude_b)) * 2);
            float iter_c = 1.0f / seg_count;
            GPoint segs[seg_count + 1];
            GPoint dst[7];
            for (int i = 0; i <= seg_count; ++i)
            {
                float t = i * iter_c;
                GPath::ChopCubicAt(points, dst, t);
                segs[i] = dst[3];
            }
            for (int i = 0; i < seg_count; i++)
            {
                drawLine(segs[i], segs[i + 1]);
            }
        }
    }
    auto shader = paint.getShader();
    if (shader == nullptr)
    {
        BlendProcE proc = gProcE[(int)blend_mode];
        for (int y = 0; y < max_hg; ++y)
        {
            if (pointsmap[y].size() > 0)
            {

                if (pointsmap[y].size() == 1)
                {
                    pointsmap[y].push_back(std::make_pair(max_ww - 1, 0));
                }
                else
                {
                    std::sort(pointsmap[y].begin(), pointsmap[y].end(),
                              [](const std::pair<int, int> &a, const std::pair<int, int> &b)
                              {
                                  return a.first < b.first;
                              });
                }

                int winding = 0;
                for (size_t i = 0; i < pointsmap[y].size() - 1; ++i)
                {
                    std::pair<int, int> first = pointsmap[y][i];
                    std::pair<int, int> second = pointsmap[y][i + 1];
                    winding += first.second;
                    if (winding != 0)
                    {
                        proc(first.first, y, second.first - first.first, converted, fDevice);
                    }
                }
            }
        }
        return;
    }
    BlendProc proc = gProcs[(int)blend_mode];
    if (!(shader->setContext(CTM.top())))
    {
        return;
    }

    // if (shader->isOpaque())
    // {
    //     proc = kSrc;
    // }
    for (int y = 0; y < max_hg; ++y)
    {
        if (pointsmap[y].size() > 0)
        {

            if (pointsmap[y].size() == 1)
            {
                pointsmap[y].push_back(std::make_pair(max_ww - 2, 0));
            }
            else
            {
                std::sort(pointsmap[y].begin(), pointsmap[y].end(),
                          [](const std::pair<int, int> &a, const std::pair<int, int> &b)
                          {
                              return a.first < b.first;
                          });
            }

            int winding = 0;
            for (size_t i = 0; i < pointsmap[y].size() - 1; ++i)
            {
                std::pair<int, int> first = pointsmap[y][i];
                std::pair<int, int> second = pointsmap[y][i + 1];
                winding += first.second;
                if (winding != 0)
                {
                    int count = second.first - first.first;
                    GPixel points[count];
                    shader->shadeRow(first.first, y, count, points);
                    blit_row_shader(first.first, y, count, converted, points, fDevice, proc);
                }
            }
        }
    }
}

static inline GPoint procGPoint(GPoint a, GPoint b, GPoint c, GPoint d, float i, float j)
{
    return GPoint{a.x * (1 - i) * (1 - j) + b.x * i * (1 - j) + c.x * i * j + d.x * (1 - i) * j, a.y * (1 - i) * (1 - j) + b.y * i * (1 - j) + c.y * i * j + d.y * (1 - i) * j};
}

static inline GColor procGCol(GColor a, GColor b, GColor c, GColor d, float i, float j)
{
    return GColor::RGBA(
        a.r * (1 - i) * (1 - j) + b.r * i * (1 - j) + c.r * i * j + d.r * (1 - i) * j,
        a.g * (1 - i) * (1 - j) + b.g * i * (1 - j) + c.g * i * j + d.g * (1 - i) * j,
        a.b * (1 - i) * (1 - j) + b.b * i * (1 - j) + c.b * i * j + d.b * (1 - i) * j,
        a.a * (1 - i) * (1 - j) + b.a * i * (1 - j) + c.a * i * j + d.a * (1 - i) * j);
}

void MyCanvas::drawQuad(const GPoint verts[4], const GColor colors[4], const GPoint texs[4], int level, const GPaint &paint)
{
    GPoint a = verts[0];
    GPoint b = verts[1];
    GPoint c = verts[2];
    GPoint d = verts[3];

    int indices[6] = {0, 1, 3, 1, 2, 3};
    float level_inf = 1.0f / (level + 1);

    GPoint final_points[4];
    GColor proc_colors[4];
    GPoint proc_texs[4];
    GColor *cp = nullptr;
    GPoint *dp = nullptr;

    for (int i = 0; i <= level; i++)
    {
        float i_a = i * level_inf;
        float i_b = (i + 1) * level_inf;
        for (int j = 0; j <= level; j++)
        {
            float j_a = j * level_inf;
            float j_b = (j + 1) * level_inf;
            final_points[0] = procGPoint(a, b, c, d, i_a, j_a);
            final_points[1] = procGPoint(a, b, c, d, i_b, j_a);
            final_points[2] = procGPoint(a, b, c, d, i_b, j_b);
            final_points[3] = procGPoint(a, b, c, d, i_a, j_b);

            if (texs != NULL)
            {
                proc_texs[0] = procGPoint(texs[0], texs[1], texs[2], texs[3], i_a, j_a);
                proc_texs[1] = procGPoint(texs[0], texs[1], texs[2], texs[3], i_b, j_a);
                proc_texs[2] = procGPoint(texs[0], texs[1], texs[2], texs[3], i_b, j_b);
                proc_texs[3] = procGPoint(texs[0], texs[1], texs[2], texs[3], i_a, j_b);
                dp = proc_texs;
            }

            if (colors != NULL)
            {
                proc_colors[0] = procGCol(colors[0], colors[1], colors[2], colors[3], i_a, j_a);
                proc_colors[1] = procGCol(colors[0], colors[1], colors[2], colors[3], i_b, j_a);
                proc_colors[2] = procGCol(colors[0], colors[1], colors[2], colors[3], i_b, j_b);
                proc_colors[3] = procGCol(colors[0], colors[1], colors[2], colors[3], i_a, j_b);
                cp = proc_colors;
            }

            drawMesh(final_points, cp, dp, 2, indices, paint);
        }
    }
};

void MyCanvas::drawMesh(const GPoint verts[], const GColor colors[], const GPoint texs[], int count, const int indices[], const GPaint &paint)
{
    int iter_val = 0;
    GPoint trig_cords[3];
    GPoint tex_cords[3];
    GColor trig_colors[3];
    for (int i = 0; i < count; i++)
    {
        trig_cords[0] = verts[indices[iter_val]];
        trig_cords[1] = verts[indices[iter_val + 1]];
        trig_cords[2] = verts[indices[iter_val + 2]];

        if (colors != NULL && texs != NULL)
        {
            trig_colors[0] = colors[indices[iter_val]];
            trig_colors[1] = colors[indices[iter_val + 1]];
            trig_colors[2] = colors[indices[iter_val + 2]];
            tex_cords[0] = texs[indices[iter_val]];
            tex_cords[1] = texs[indices[iter_val + 1]];
            tex_cords[2] = texs[indices[iter_val + 2]];
            auto col_shader = makeTriColorShader(trig_colors, trig_cords);
            auto tex_shader = makeTriTextureShader(paint.getShader(), trig_cords, tex_cords);
            auto merge_shader = makeTriangleCombined(col_shader.get(), tex_shader.get());
            auto paint = GPaint();
            paint.setShader(merge_shader.get());
            drawConvexPolygon(trig_cords, 3, paint);
        }
        else if (colors != NULL)
        {
            trig_colors[0] = colors[indices[iter_val]];
            trig_colors[1] = colors[indices[iter_val + 1]];
            trig_colors[2] = colors[indices[iter_val + 2]];
            auto shader = makeTriColorShader(trig_colors, trig_cords);
            auto paint = GPaint(shader.get());
            drawConvexPolygon(trig_cords, 3, paint);
        }
        else if (texs != NULL)
        {
            tex_cords[0] = texs[indices[iter_val]];
            tex_cords[1] = texs[indices[iter_val + 1]];
            tex_cords[2] = texs[indices[iter_val + 2]];
            auto shader = makeTriTextureShader(paint.getShader(), trig_cords, tex_cords);
            auto paint = GPaint();
            paint.setShader(shader.get());
            drawConvexPolygon(trig_cords, 3, paint);
        }

        iter_val += 3;
    }
}

std::string GDrawSomething(GCanvas *canvas, GISize dim)
{
    // as fancy as you like
    // GBitmap bm;
    // bm.readFromFile("assets/mike.png");

    // std::cout << dim.height << "  " << dim.width << "\n";
    // auto sh = GCreateBitmapShader(bm, GMatrix::Scale(1.5f, 1.5f));
    // GPaint paint(sh.get());
    // canvas->drawRect(GRect::WH(256, 256), paint);
    // GPaint red_paint(GColor::RGB(0.627f, 0.322f, 0.176f));
    // red_paint.setBlendMode(GBlendMode::kSrc);
    // canvas->save();

    // canvas->save();
    // canvas->concat(GMatrix::Rotate(5.93412f));
    // canvas->drawRect(GRect::XYWH(20, 200, 60, 15), red_paint);
    // canvas->restore();
    // canvas->concat(GMatrix::Rotate(0.349066f));
    // canvas->drawRect(GRect::XYWH(200, 100, 60, 15), red_paint);
    // canvas->restore();

    return "Mike Will Shave";
}

std::unique_ptr<GShader> GCreateBitmapShader(const GBitmap &canvas, const GMatrix &localMatrix, GTileMode tiling)
{
    auto shader = BitmapShader(canvas, localMatrix, tiling);
    return std::make_unique<BitmapShader>(shader);
}

std::unique_ptr<GShader> GCreateLinearGradient(GPoint p0, GPoint p1, const GColor colors[], int count, GTileMode tiling)
{
    auto shader = LinearGradient(p0, p1, colors, count, tiling);
    return std::make_unique<LinearGradient>(shader);
    // return std::unique_ptr<GShader>(nullptr);
}