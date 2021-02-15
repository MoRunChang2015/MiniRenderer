#include <algorithm>

#include "graphics.h"
#include "resource/material.h"
#include "resource/mesh.h"
#include "resource/model.h"
#include "util/tgaImage.h"

constexpr int width = 800;
constexpr int height = 800;
Model* model = nullptr;
Material* material = nullptr;
Mesh* mesh = nullptr;
float* zbuffer = nullptr;
Vec3f light_dir{1.f, 1.f, 0.0f};
const Vec3f eye_pos{1.f, 1.0f, 4.f};
const Vec3f center{0.f, 0.f, 0.f};
const Vec3f up{0.f, 1.f, 0.f};

struct Shader : IShader {
    Matrix4x4 uniform_M;              // Projection * ModelView;
    Matrix4x4 uniform_MIT;            // (Projection * ModelView).invert_transpose
    Vec3f uniform_light_dir;          // uniform_M * lightdir
    Matrix<2, 3, float> vary_uv;      // triangle uv coordinates, set by vs, read by ps
    Matrix<3, 3, float> vary_normal;  // trangle noraml vector, set by vs, read by ps
    Matrix<3, 3, float> vary_tri;     // triangle coordinates before viewport transform, set by vs, read by ps

    Shader(const Matrix4x4& M, const Matrix4x4& MIT)
        : uniform_M(M),
          uniform_MIT(MIT),
          uniform_light_dir(projection<3>(uniform_M * embed<4>(light_dir)).normalize()),
          vary_uv(),
          vary_tri() {}

    virtual Vec4f vertex(const int& faceIdx, const int& nthvert) {
        vary_uv.set_column(nthvert, model->getMesh()->uv(model->getMesh()->face_uv(faceIdx)[nthvert]));
        vary_normal.set_column(nthvert, model->getMesh()->normal(model->getMesh()->face_normal(faceIdx)[nthvert]));
        Vec4f vertex = uniform_M * embed<4>(model->getMesh()->vert(model->getMesh()->face_vertex(faceIdx)[nthvert]));
        vary_tri.set_column(nthvert, projection<3>(vertex / vertex[3]));
        vertex = Viewport * vertex;
        return vertex;
    }

    virtual bool fragment(const Vec3f bar, TGAColor& outColor) const {
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
        const Vec3f r = n * (uniform_light_dir * n) * 2 - uniform_light_dir;
        //const float spec = std::pow(std::max(r.z, 0.f), model->getMaterial()->specular(uv));
        const float spec = 0.f;
        const float diff = std::max(0.f, uniform_light_dir * n);
        outColor = model->getMaterial()->diffuse(uv);
        for (size_t i = 0; i < 3; ++i)
            outColor[i] = static_cast<unsigned char>(std::min(5.f + outColor[i] * (1.2f * diff + 0.6f * spec), 255.f));
        return false;
    }
};

int main(int argc, char** argv) {
    if (argc == 3) {
        mesh = new Mesh(argv[0]);
        material = new Material(argv[1]);
    } else {
        std::string filename = "../resource/african_head/african_head.obj";
        mesh = new Mesh(filename);
        size_t dot = filename.find_last_of(".");
        if (dot != std::string::npos) {
            filename = filename.substr(0, dot);
            material = new Material(filename + "_diffuse.tga", filename + "_nm_tangent.tga", filename + "_spec.tga");
        }
    }
    if (material == nullptr) return 0;
    model = new Model(mesh, material);
    light_dir.norm();

    zbuffer = new float[width * height + 1];
    for (int i = 0; i < width * height; ++i) zbuffer[i] = -std::numeric_limits<float>::max();

    TGAImage output(width, height, TGAImage::RGB);
    lookat(eye_pos, center, up);
    viewport(width / 8, height / 8, width * 3 / 4, height * 3 / 4);
    projection(-1.f / (eye_pos - center).norm());
    Matrix4x4 ModelView = View * model->getTransform();
    Shader shader(Projection * ModelView, (Projection * ModelView).invert_transpose());
    Vec4f screen_coord[3];
    for (size_t i = 0; i < model->getMesh()->nfaces(); ++i) {
        for (size_t j = 0; j < 3; ++j) {
            screen_coord[j] = shader.vertex(i, j);
        }
        triangle(screen_coord, shader, output, zbuffer);
    }
    output.flip_vertically();
    output.write_tga_file("output.tga");

    delete model;
    delete material;
    delete mesh;
    delete[] zbuffer;
    return 0;
}