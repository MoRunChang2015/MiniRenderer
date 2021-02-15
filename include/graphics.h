#pragma once

#include "util/tgaImage.h"
#include "util//geometry.h"

extern Matrix4x4 View;
extern Matrix4x4 Viewport;
extern Matrix4x4 Projection;
const float depth = 2000.f;
constexpr float PI = 3.14159265358979323846;

void viewport(int x, int y, int w, int h);
void projection(float coeff = 0.f); // coeff = -1/c
void lookat(Vec3f eye, Vec3f center, Vec3f up);

struct IShader {
    virtual ~IShader() = default;
    virtual Vec4f vertex(const int& faceIdx, const int& nthvert) = 0;
    virtual bool fragment(const Vec3f& viewCoord, const Vec3f bar, TGAColor& outColor) const = 0;
};

void triangle(const Vec4f inPts[], const IShader& shader, TGAImage& output, float zbuffer[]);