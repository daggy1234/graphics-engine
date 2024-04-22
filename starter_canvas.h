/*
 *  Copyright 2024 Daggy1234
 */

#ifndef _g_starter_canvas_h_
#define _g_starter_canvas_h_

#include "include/GCanvas.h"
#include "include/GRect.h"
#include "include/GColor.h"
#include "include/GBitmap.h"
#include "stack"

class MyCanvas : public GCanvas
{
public:
    explicit MyCanvas(const GBitmap &device) : fDevice(device), ctm_no(0)
    {
        CTM.emplace();
        pointsmap.resize(device.height());
    }

    void clear(const GColor &color) override;
    //    void fillRect(const GRect& rect, const GColor& color) override;
    void drawRect(const GRect &rect, const GPaint &paint) override;
    //    void drawConvexPolygon(const GPoint* points, int count, const GPaint& paint) override;
    void drawConvexPolygon(const GPoint points[], int count, const GPaint &) override;
    void drawPath(const GPath &, const GPaint &) override;
    void drawLine(GPoint p0, GPoint p1);
    void save() override;
    void restore() override;
    void concat(const GMatrix &matrix) override;
    virtual void drawMesh(const GPoint verts[], const GColor colors[], const GPoint texs[],
                          int count, const int indices[], const GPaint &paint) override;
    virtual void drawQuad(const GPoint verts[4], const GColor colors[4], const GPoint texs[4],
                          int level, const GPaint &) override;

private:
    // Note: we store a copy of the bitmap
    const GBitmap fDevice;
    std::stack<GMatrix> CTM;
    int ctm_no;
    std::vector<std::vector<std::pair<int, int>>> pointsmap;
    // Add whatever other fields you need
};

#endif