#include <algorithm>

#include "graphics.h"
#include "resource/material.h"
#include "resource/mesh.h"
#include "resource/model.h"
#include "util/tgaImage.h"

constexpr int width = 2048;
constexpr int height = 2048;
const Model* model = nullptr;  // current rendering model
float* zbuffer = nullptr;
float* shadowMap = nullptr;
Vec3f light_dir{1.f, 1.f, 1.0f};
const Vec3f eye_pos{1.f, 1.0f, 4.f};
const Vec3f center{0.f, 0.f, 0.f};
const Vec3f up{0.f, 1.f, 0.f};

struct DepthShader : IShader {
    Matrix4x4 uniform_M;  // Projection * ModelView;

    DepthShader(const Matrix4x4& M) : uniform_M(M){};

    virtual Vec4f vertex(const int& faceIdx, const int& nthvert) {
        const Vec4f vertex =
            uniform_M * embed<4>(model->getMesh()->vert(model->getMesh()->face_vertex(faceIdx)[nthvert]));
        return vertex;
    }
    virtual bool fragment(const Vec3f& viewCoord, const Vec3f bar, TGAColor& outColor) const {
        outColor = TGAColor(255, 255, 255, 255) * (viewCoord.z / depth);
        return false;
    }
};

struct Shader : IShader {
    Matrix4x4 uniform_M;              // Projection * ModelView;
    Matrix4x4 uniform_shadow;         // ShadowMapVPM * (Viewport * Projection * ModelView).invert
    Vec3f uniform_light_dir;          // uniform_M * lightdir
    Matrix<2, 3, float> vary_uv;      // triangle uv coordinates, set by vs, read by ps
    Matrix<3, 3, float> vary_normal;  // trangle noraml vector, set by vs, read by ps
    Matrix<3, 3, float> vary_tri;     // triangle coordinates before viewport transform, set by vs, read by ps

    Shader(const Matrix4x4& M, const Matrix4x4& MS)
        : uniform_M(M),
          uniform_shadow(MS),
          uniform_light_dir(projection<3>(uniform_M * embed<4>(light_dir)).normalize()),
          vary_uv(),
          vary_tri() {}

    virtual Vec4f vertex(const int& faceIdx, const int& nthvert) {
        vary_uv.set_column(nthvert, model->getMesh()->uv(model->getMesh()->face_uv(faceIdx)[nthvert]));
        vary_normal.set_column(nthvert, model->getMesh()->normal(model->getMesh()->face_normal(faceIdx)[nthvert]));
        Vec4f vertex = uniform_M * embed<4>(model->getMesh()->vert(model->getMesh()->face_vertex(faceIdx)[nthvert]));
        vary_tri.set_column(nthvert, projection<3>(vertex / vertex[3]));
        return vertex;
    }

    virtual bool fragment(const Vec3f& viewCoord, const Vec3f bar, TGAColor& outColor) const {
        const Vec2f uv = vary_uv * bar;
        const Vec3f bn = (vary_normal * bar).normalize();

        Matrix<3, 3, float> A;
        A[0] = vary_tri.column(1) - vary_tri.column(0);
        A[1] = vary_tri.column(2) - vary_tri.column(0);
        A[2] = bn;

        const Matrix<3, 3, float> AI = A.invert();
        const Vec3f i = AI * Vec3f(vary_uv[0][1] - vary_uv[0][0], vary_uv[0][2] - vary_uv[0][0], 0.f);
        const Vec3f j = AI * Vec3f(vary_uv[1][1] - vary_uv[1][0], vary_uv[1][2] - vary_uv[1][0], 0.f);
        Matrix<3, 3, float> B;
        B.set_column(0, i);
        B.set_column(1, j);
        B.set_column(2, bn);

        const Vec3f n = (B * model->getMaterial()->normal(uv)).normalize();

        Vec4f sm_p = uniform_shadow * embed<4>(viewCoord);
        sm_p = sm_p / sm_p[3];
        const int shadowPos = static_cast<int>(sm_p[0] + 0.5) + static_cast<int>(sm_p[1] + 0.5) * width;
        const float shadow = 0.3f + 0.7f * (shadowMap[shadowPos] < sm_p[2] + 12.21f);  // magic coeff to avoid z-fighting

        const Vec3f r = n * (uniform_light_dir * n) * 2 - uniform_light_dir;
        const float spec = std::pow(std::max(r.z, 0.f), model->getMaterial()->specular(uv));
        const float diff = std::max(0.f, uniform_light_dir * n);
        outColor = model->getMaterial()->diffuse(uv);
        for (size_t i = 0; i < 3; ++i)
            outColor[i] =
                static_cast<unsigned char>(std::min(5.f + outColor[i] * shadow * (1.2f * diff + 0.6f * spec), 255.f));
        return false;
    }
};

float max_elevation_angle(float* zbuffer, Vec2f p, Vec2f dir) {
    float maxangle = 0;
    for (float t = 0.; t < 1000.; t += 1.) {
        Vec2f cur = p + dir * t;
        if (cur.x >= width || cur.y >= height || cur.x < 0 || cur.y < 0) return maxangle;

        float distance = (p - cur).norm();
        if (distance < 1.f) continue;
        float elevation = zbuffer[int(cur.x) + int(cur.y) * width] - zbuffer[int(p.x) + int(p.y) * width];
        maxangle = std::max(maxangle, atanf(elevation / distance));
    }
    return maxangle;
}

int main(int argc, char** argv) {
    std::vector<std::string> modelsFilename{
        {"../resource/boggie/body"},
        {"../resource/boggie/eyes"},
        {"../resource/boggie/head"},
    };
    std::vector<Mesh> meshs;
    std::vector<Material> materials;
    std::vector<Model> models;
    meshs.reserve(modelsFilename.size());
    materials.reserve(modelsFilename.size());
    models.reserve(modelsFilename.size());
    for (const std::string& filename : modelsFilename) {
        meshs.emplace_back(filename + ".obj");
        materials.emplace_back(filename + "_diffuse.tga", filename + "_nm_tangent.tga", filename + "_spec.tga");
        models.emplace_back(&meshs.back(), &materials.back());
    }

    light_dir.norm();

    zbuffer = new float[width * height + 1];
    for (int i = 0; i < width * height; ++i) zbuffer[i] = -std::numeric_limits<float>::max();

    shadowMap = new float[width * height + 1];
    for (int i = 0; i < width * height; ++i) zbuffer[i] = -std::numeric_limits<float>::max();

    {
        // shadowmap
        TGAImage depthOutput(width, height, TGAImage::RGB);
        lookat(light_dir, center, up);
        viewport(width / 8, height / 8, width * 3 / 4, height * 3 / 4);
        projection(0);
        for (const Model& m : models) {
            model = &m;
            Matrix4x4 ModelView = View * model->getTransform();
            DepthShader depthShader(ModelView);
            Vec4f screen_coord[3];
            for (size_t i = 0; i < model->getMesh()->nfaces(); ++i) {
                for (size_t j = 0; j < 3; ++j) {
                    screen_coord[j] = depthShader.vertex(i, j);
                }
                triangle(screen_coord, depthShader, depthOutput, shadowMap);
            }
        }
        depthOutput.flip_vertically();
        depthOutput.write_tga_file("depthOutput.tga");
    }

    const Matrix4x4 shadowMapM = Viewport * Projection * View;

    // renderring
    TGAImage output(width, height, TGAImage::RGB);
    lookat(eye_pos, center, up);
    viewport(width / 8, height / 8, width * 3 / 4, height * 3 / 4);
    projection(-1.f / (eye_pos - center).norm());
    for (const Model& m : models) {
        model = &m;
        Matrix4x4 ModelView = View * model->getTransform();
        Shader shader(Projection * ModelView,
                      shadowMapM * model->getTransform() * (Viewport * Projection * ModelView).invert());
        Vec4f screen_coord[3];
        for (size_t i = 0; i < model->getMesh()->nfaces(); ++i) {
            for (size_t j = 0; j < 3; ++j) {
                screen_coord[j] = shader.vertex(i, j);
            }
            triangle(screen_coord, shader, output, zbuffer);
        }
    }
    output.flip_vertically();
    output.write_tga_file("output.tga");

    // Post process
    // SSAO
    // for (int x = 0; x < width; x++) {
    //    for (int y = 0; y < height; y++) {
    //        if (zbuffer[x + y * width] < -1e5) continue;
    //        float total = 0;
    //        for (float a = 0; a < PI * 2 - 1e-4; a += PI / 4) {
    //            total += PI / 2 - max_elevation_angle(zbuffer, Vec2f(x, y), Vec2f(cos(a), sin(a)));
    //        }
    //        total /= (PI / 2) * 8;
    //        total = pow(total, 100.f);
    //        output.set(x, y,
    //                   TGAColor(static_cast<unsigned char>(total * 255), static_cast<unsigned char>(total * 255),
    //                            static_cast<unsigned char>(total * 255), 255));
    //    }
    //}
    delete[] zbuffer;
    delete[] shadowMap;
    return 0;
}