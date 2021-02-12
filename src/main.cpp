#include <algorithm>

#include "resource/model.h"
#include "util/tgaImage.h"

const TGAColor white = TGAColor(255, 255, 255, 255);
const TGAColor red = TGAColor(255, 0, 0, 255);
const TGAColor green = TGAColor(0, 255, 0, 255);

void line(int x0, int y0, int x1, int y1, TGAImage& image, const TGAColor& color) {
    bool steep = false;
    if (std::abs(x0 - x1) < std::abs(y0 - y1)) {
        std::swap(x0, y0);
        std::swap(x1, y1);
        steep = true;
    }
    if (x0 > x1) {
        std::swap(x0, x1);
        std::swap(y0, y1);
    }
    const int dx = x1 - x0;
    const int dy = y1 - y0;
    const int derror2 = std::abs(dy) * 2;
    int error2 = 0;
    int y = y0;
    for (int x = x0; x <= x1; x++) {
        if (!steep) {
            image.set(x, y, color);
        } else {
            image.set(y, x, color);
        }
        error2 += derror2;
        if (error2 > dx) {
            y += (y1 > y0 ? 1 : -1);
            error2 -= dx * 2;
        }
    }
}

Vec3f barycentric(Vec2i a, Vec2i b, Vec2i c, Vec2i p) {
    const Vec3i v0(b.x - a.x, c.x - a.x, a.x - p.x);
    const Vec3i v1(b.y - a.y, c.y - a.y, a.y - p.y);
    const Vec3i ans = v0 ^ v1;
    if (ans.z == 0) return {-1.f, -1.f, 1.f};
    return {1.0f - static_cast<float>(ans.x + ans.y) / static_cast<float>(ans.z), ans.x / static_cast<float>(ans.z),
            ans.y / static_cast<float>(ans.z)};
}

void triangle(Vec2i t0, Vec2i t1, Vec2i t2, TGAImage& image, const TGAColor& color) {
    Vec2i min{image.get_width() - 1, image.get_height() - 1};
    Vec2i max{0, 0};
    const Vec2i clamp = min;

    min = {std::max(0, std::min(min.x, t0.x)), std::max(0, std::min(min.y, t0.y))};
    min = {std::max(0, std::min(min.x, t1.x)), std::max(0, std::min(min.y, t1.y))};
    min = {std::max(0, std::min(min.x, t2.x)), std::max(0, std::min(min.y, t2.y))};

    max = {std::min(clamp.x, std::max(max.x, t0.x)), std::min(clamp.y, std::max(max.y, t0.y))};
    max = {std::min(clamp.x, std::max(max.x, t1.x)), std::min(clamp.y, std::max(max.y, t1.y))};
    max = {std::min(clamp.x, std::max(max.x, t2.x)), std::min(clamp.y, std::max(max.y, t2.y))};

    Vec2i p;
    for (p.x = min.x; p.x <= max.x; ++p.x)
        for (p.y = min.y; p.y <= max.y; ++p.y) {
            const Vec3f ans = barycentric(t0, t1, t2, p);
            if (ans.x < 0 || ans.y < 0 || ans.z < 0) continue;
            image.set(p.x, p.y, color);
        }
}

int main(int argc, char** argv) {
    Model* model;
    const int width = 800;
    const int height = 800;
    if (argc == 2) {
        model = new Model(argv[1]);
    } else {
        model = new Model("../resource/african_head.obj");
    }
    TGAImage image(width, height, TGAImage::RGB);

    const Vec3f light_dir{0.f, 0.f, -1.0f};

    for (int i = 0; i < model->nfaces(); ++i) {
        const std::vector<int>& face = model->face(i);
        std::vector<Vec2i> face_vec2{};
        std::vector<Vec3f> face_vec3f{};
        for (int j = 0; j < 3; ++j) {
            const Vec3f& v0 = model->vert(face[j]);
            face_vec3f.emplace_back(v0);
            face_vec2.emplace_back(static_cast<int>((v0.x + 1.0) * width / 2.0),
                                   static_cast<int>((v0.y + 1.0) * height / 2.0));
        }
        Vec3f n = (face_vec3f[2] - face_vec3f[0]) ^ (face_vec3f[1] - face_vec3f[0]);
        n.noramlize();
        const float& intensity = light_dir * n;
        if (intensity > 0) {
            triangle(face_vec2[0], face_vec2[1], face_vec2[2], image,
                     TGAColor(intensity * 255, intensity * 255, intensity * 255, 255));
        }
    }

    image.flip_vertically();  // i want to have the origin at the left bottom corner of the image
    image.write_tga_file("output.tga");
    return 0;
}