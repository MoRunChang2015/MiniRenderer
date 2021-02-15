#include "graphics.h"

#include <algorithm>

Matrix4x4 View;
Matrix4x4 Projection;
Matrix4x4 Viewport;

void viewport(int x, int y, int w, int h) {
    Viewport = Matrix4x4::identity();
    Viewport[0][3] = x + w / 2.f;
    Viewport[1][3] = y + h / 2.f;
    Viewport[2][3] = depth / 2.f;
    Viewport[0][0] = w / 2.f;
    Viewport[1][1] = h / 2.f;
    Viewport[2][2] = depth / 2.f;
}
void projection(float coeff) {
    Projection = Matrix4x4::identity();
    Projection[3][2] = coeff;
}
void lookat(Vec3f eye, Vec3f center, Vec3f up) {
    Vec3f z = (eye - center).normalize();
    Vec3f x = cross(up, z).normalize();
    Vec3f y = cross(z, x).normalize();
    View = Matrix4x4::identity();
    for (int i = 0; i < 3; i++) {
        View[0][i] = x[i];
        View[1][i] = y[i];
        View[2][i] = z[i];
        View[i][3] = -center[i];
    }
}

Vec3f barycentric(Vec3f a, Vec3f b, Vec3f c, Vec2i p) {
    const Vec3f v0(b.x - a.x, c.x - a.x, a.x - p.x);
    const Vec3f v1(b.y - a.y, c.y - a.y, a.y - p.y);
    const Vec3f ans = cross(v0, v1);
    if (std::abs(ans.z) < 1e-2) return {-1.f, -1.f, 1.f};
    return {1.0f - (ans.x + ans.y) / ans.z, ans.x / ans.z, ans.y / ans.z};
}

void triangle(const Vec4f inPts[], const IShader& shader, TGAImage& output, float zbuffer[]) {
    Vec2f min{static_cast<float>(output.get_width() - 1), static_cast<float>(output.get_height() - 1)};
    Vec2f max{0, 0};
    const Vec2f clamp = min;
    Vec4f pts[4];
    for (int i = 0; i < 3; ++i) {
        pts[i] = (inPts[i] / inPts[i][3]).round();
        for (int j = 0; j < 2; ++j) {
            min[j] = std::max(0.f, std::min(min[j], pts[i][j]));
            max[j] = std::min(clamp[j], std::max(max[j], pts[i][j]));
        }
    }

    Vec2i p;
    for (p.x = static_cast<int>(min.x); p.x <= max.x; ++p.x)
        for (p.y = static_cast<int>(min.y); p.y <= max.y; ++p.y) {
            const Vec3f ans = barycentric(projection<3>(pts[0] / pts[0][3]), projection<3>(pts[1] / pts[0][3]),
                                          projection<3>(pts[2] / pts[0][3]), p);
            if (ans.x < 0 || ans.y < 0 || ans.z < 0) continue;
            const float z = ans.x * pts[0][2] + ans.y * pts[1][2] + ans.z * pts[2][2];
            const float w = ans.x * pts[0][3] + ans.y * pts[1][3] + ans.z * pts[2][3];
            float d = z / w;
            if (zbuffer[static_cast<int>(p.x + p.y * output.get_width())] < d) {
                TGAColor c;
                if (!shader.fragment(ans, c)) {
                    zbuffer[static_cast<int>(p.x + p.y * output.get_width())] = d;
                    output.set(p.x, p.y, c);
                }
            }
        }
}