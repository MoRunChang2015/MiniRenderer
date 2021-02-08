#include "resource/model.h"
#include "util/tgaImage.h"

const TGAColor white = TGAColor(255, 255, 255, 255);
const TGAColor red = TGAColor(255, 0, 0, 255);

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

    for (int i = 0; i < model->nfaces(); ++i) {
        const std::vector<int>& face = model->face(i);
        for (int j = 0; j < 3; ++j) {
            const Vec3f v0 = model->vert(face[j]);
            const Vec3f v1 = model->vert(face[(j + 1) % 3]);
            const int x0 = static_cast<int>((v0.x + 1.0) * width / 2.0);
            const int y0 = static_cast<int>((v0.y + 1.0) * height / 2.0);
            const int x1 = static_cast<int>((v1.x + 1.0) * width / 2.0);
            const int y1 = static_cast<int>((v1.y + 1.0) * height / 2.0);

            line(x0, y0, x1, y1, image, white);
        }
    }

    image.flip_vertically();  // i want to have the origin at the left bottom corner of the image
    image.write_tga_file("output.tga");
    return 0;
}