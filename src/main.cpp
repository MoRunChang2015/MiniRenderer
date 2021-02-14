#include <algorithm>

#include "resource/model.h"
#include "util/tgaImage.h"

const TGAColor white = TGAColor(255, 255, 255, 255);
const TGAColor red = TGAColor(255, 0, 0, 255);
const TGAColor green = TGAColor(0, 255, 0, 255);
constexpr int width = 800;
constexpr int height = 800;
constexpr int depth = 255;

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

Vec3f barycentric(Vec3f a, Vec3f b, Vec3f c, Vec3f p) {
    const Vec3f v0(b.x - a.x, c.x - a.x, a.x - p.x);
    const Vec3f v1(b.y - a.y, c.y - a.y, a.y - p.y);
    const Vec3f ans = v0 ^ v1;
    if (std::abs(ans.z) < 1e-2) return {-1.f, -1.f, 1.f};
    return {1.0f - (ans.x + ans.y) / ans.z, ans.x / ans.z, ans.y / ans.z};
}

void triangle(Vec3f pts[], Vec3f uvs[], float zbuffer[], const TGAImage& diffuse, TGAImage& output,
              const Vec3f& lightInentsity) {
    Vec2f min{static_cast<float>(output.get_width() - 1), static_cast<float>(output.get_height() - 1)};
    Vec2f max{0, 0};
    const Vec2f clamp = min;

    for (int i = 0; i < 3; i++) {
        for (int j = 0; j < 2; j++) {
            min[j] = std::max(0.f, std::min(min[j], pts[i][j]));
            max[j] = std::min(clamp[j], std::max(max[j], pts[i][j]));
        }
    }

    Vec3f p;
    for (p.x = min.x; p.x <= max.x; ++p.x)
        for (p.y = min.y; p.y <= max.y; ++p.y) {
            const Vec3f ans = barycentric(pts[0], pts[1], pts[2], p);
            if (ans.x < 0 || ans.y < 0 || ans.z < 0) continue;
            p.z = ans.x * pts[0].z + ans.y * pts[1].z + ans.z * pts[2].z;
            if (zbuffer[static_cast<int>(p.x + p.y * width)] < p.z) {
                zbuffer[static_cast<int>(p.x + p.y * width)] = p.z;

                Vec2f uv = {0.f, 0.f};
                for (int i = 0; i < 3; ++i)
                    for (int j = 0; j < 2; ++j) uv[j] += ans[i] * uvs[i][j];
                output.set(static_cast<int>(p.x), static_cast<int>(p.y),
                           diffuse.get(uv.u * diffuse.get_width(), uv.v * diffuse.get_height()) * lightInentsity);
            }
        }
}

Vec3f homogeneousToVector(const Matrix& m) { return {m[0][0] / m[3][0], m[1][0] / m[3][0], m[2][0] / m[3][0]}; }
Matrix vectorToHomogeneous(const Vec3f& v) {
    Matrix m(4, 1);
    m[0][0] = v.x;
    m[1][0] = v.y;
    m[2][0] = v.z;
    m[3][0] = 1;
    return m;
}

Matrix viewport(const int& x, const int& y, const int& w, const int& h) {
    Matrix m = Matrix::identity(4);
    m[0][3] = x + w / 2.f;
    m[1][3] = y + h / 2.f;
    m[2][3] = depth / 2.f;

    m[0][0] = w / 2.f;
    m[1][1] = h / 2.f;
    m[2][2] = depth / 2.f;
    return m;
}

Matrix lookat(const Vec3f& eye, const Vec3f& center, const Vec3f& up) {
    Vec3f z = (eye - center).normalize();
    Vec3f x = (up ^ z).normalize();
    Vec3f y = (z ^ x).normalize();
    Matrix res = Matrix::identity(4);
    for (int i = 0; i < 3; i++) {
        res[0][i] = x[i];
        res[1][i] = y[i];
        res[2][i] = z[i];
        res[i][3] = -center[i];
    }
    return res;
}

inline Vec3f worldToScreen(const Matrix& vp, const Vec3f& v) {
    return (homogeneousToVector(vp * vectorToHomogeneous(v))).round();
}

int main(int argc, char** argv) {
    Model* model;
    TGAImage diffuse;
    if (argc == 3) {
        model = new Model(argv[1]);
        diffuse.read_tga_file(argv[2]);
    } else {
        model = new Model("../resource/african_head.obj");
        diffuse.read_tga_file("../resource/african_head_diffuse.tga");
    }
    TGAImage image(width, height, TGAImage::RGB);
    diffuse.flip_vertically();

    float* zbuffer = new float[width * height + 1];
    for (int i = 0; i < width * height; ++i) zbuffer[i] = -std::numeric_limits<float>::max();

    const Vec3f light_dir{0.f, 0.f, -1.0f};
    const Vec3f eye_pos{0.f, 0.f, 3.f};
    const Vec3f center{0.f, 0.f, 0.f};

    Matrix modelView = lookat(eye_pos, center, {0.f, 1.f, 0.f});
    Matrix projection = Matrix::identity(4);
    Matrix v = viewport(width / 8, height / 8, width * 3 / 4, height * 3 / 4);
    projection[3][2] = -1.0f / (eye_pos - center).norm();

    const Matrix vp = v * projection * modelView;

    for (int i = 0; i < model->nfaces(); ++i) {
        const std::vector<Model::Vertex>& face = model->face(i);
        std::vector<Vec3f> face_world{};
        std::vector<Vec3f> face_vert{};
        std::vector<Vec3f> face_uv{};
        for (int j = 0; j < 3; ++j) {
            const Vec3f v = model->vert(face[j].vertIdx());
            face_vert.emplace_back(worldToScreen(vp, v));
            face_world.emplace_back(v);
            face_uv.emplace_back(model->uv(face[j].uvIdx()));
        }
        Vec3f n = (face_world[2] - face_world[0]) ^ (face_world[1] - face_world[0]);
        n.normalize();
        const float& intensity = light_dir * n;
        if (intensity > 0) {
            triangle(face_vert.data(), face_uv.data(), zbuffer, diffuse, image, {intensity, intensity, intensity});
        }
    }

    image.flip_vertically();  // i want to have the origin at the left bottom corner of the image
    image.write_tga_file("output.tga");
    delete model;
    delete[] zbuffer;
    return 0;
}