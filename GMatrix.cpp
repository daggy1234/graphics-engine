#include "include/GMatrix.h"

std::optional<GMatrix> GMatrix::invert() const
{
    float det = fMat[0] * fMat[3] - fMat[1] * fMat[2];
    if (det == 0.0f)
    {
        return std::nullopt;
    }

    float invDet = 1.0f / det;
    return GMatrix(
        fMat[3] * invDet, -fMat[2] * invDet, (fMat[2] * fMat[5] - fMat[3] * fMat[4]) * invDet,
        -fMat[1] * invDet, fMat[0] * invDet, (fMat[1] * fMat[4] - fMat[0] * fMat[5]) * invDet);
}

GMatrix GMatrix::Scale(float sx, float sy)
{
    return {sx, 0, 0, 0, sy, 0};
}

GMatrix GMatrix::Concat(const GMatrix &a, const GMatrix &b)
{
    return {
        a.fMat[0] * b.fMat[0] + a.fMat[2] * b.fMat[1],
        a.fMat[0] * b.fMat[2] + a.fMat[2] * b.fMat[3],
        a.fMat[0] * b.fMat[4] + a.fMat[2] * b.fMat[5] + a.fMat[4],
        a.fMat[1] * b.fMat[0] + a.fMat[3] * b.fMat[1],
        a.fMat[1] * b.fMat[2] + a.fMat[3] * b.fMat[3],
        a.fMat[1] * b.fMat[4] + a.fMat[3] * b.fMat[5] + a.fMat[5]};
}

GMatrix GMatrix::Rotate(float radians)
{
    float cosTheta = cosf(radians);
    float sinTheta = sinf(radians);
    return {cosTheta, -sinTheta, 0, sinTheta, cosTheta, 0};
}

GMatrix GMatrix::Translate(float tx, float ty)
{
    return {1, 0, tx, 0, 1, ty};
}

GMatrix::GMatrix()
{
    fMat[0] = 1.0f; // a
    fMat[1] = 0.0f; // b
    fMat[2] = 0.0f; // c
    fMat[3] = 1.0f; // d
    fMat[4] = 0.0f; // e
    fMat[5] = 0.0f; // f
}

void GMatrix::mapPoints(GPoint dst[], const GPoint src[], int count) const
{
    for (int i = 0; i < count; ++i)
    {
        float x = fMat[0] * src[i].x + fMat[2] * src[i].y + fMat[4];
        float y = fMat[1] * src[i].x + fMat[3] * src[i].y + fMat[5];
        dst[i] = {x, y};
    }
}
