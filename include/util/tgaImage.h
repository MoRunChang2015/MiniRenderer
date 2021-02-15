#ifndef __IMAGE_H__
#define __IMAGE_H__

#include <fstream>
#include "geometry.h"

#pragma pack(push, 1)
struct TGA_Header {
    char idlength;
    char colormaptype;
    char datatypecode;
    short colormaporigin;
    short colormaplength;
    char colormapdepth;
    short x_origin;
    short y_origin;
    short width;
    short height;
    char bitsperpixel;
    char imagedescriptor;
};
#pragma pack(pop)

struct TGAColor {
    union {
        struct {
            unsigned char b, g, r, a;
        };
        unsigned char raw[4];
        unsigned int val;
    };
    int bytespp;

    TGAColor() : val(0), bytespp(1) {}

    TGAColor(unsigned char R, unsigned char G, unsigned char B, unsigned char A) : b(B), g(G), r(R), a(A), bytespp(4) {}

    TGAColor(int v, int bpp) : val(v), bytespp(bpp) {}

    TGAColor(const TGAColor &c) : val(c.val), bytespp(c.bytespp) {}

    TGAColor(const unsigned char *p, int bpp) : val(0), bytespp(bpp) {
        for (int i = 0; i < bpp; i++) {
            raw[i] = p[i];
        }
    }

    TGAColor &operator=(const TGAColor &c) {
        if (this != &c) {
            bytespp = c.bytespp;
            val = c.val;
        }
        return *this;
    }

    TGAColor operator*(const Vec3f &other) {
        return {static_cast<unsigned char>(r * other.x), static_cast<unsigned char>(g * other.y),
                static_cast<unsigned char>(b * other.z), a};
    }

    TGAColor operator*(const float &other) {
        return {static_cast<unsigned char>(r * other), static_cast<unsigned char>(g * other),
                static_cast<unsigned char>(b * other), static_cast<unsigned char>(a * other)};
    }

    unsigned char &operator[](const size_t &idx) { return raw[idx]; }
    const unsigned char &operator[](const size_t &idx) const { return raw[idx]; }
};

class TGAImage {
   protected:
    unsigned char *data;
    int width;
    int height;
    int bytespp;

    bool load_rle_data(std::ifstream &in);
    bool unload_rle_data(std::ofstream &out);

   public:
    enum Format { GRAYSCALE = 1, RGB = 3, RGBA = 4 };

    TGAImage();
    TGAImage(int w, int h, int bpp);
    TGAImage(const TGAImage &img);
    bool read_tga_file(const char *filename);
    bool write_tga_file(const char *filename, bool rle = true);
    bool flip_horizontally();
    bool flip_vertically();
    bool scale(int w, int h);
    TGAColor get(int x, int y) const;
    bool set(int x, int y, const TGAColor &c);
    ~TGAImage();
    TGAImage &operator=(const TGAImage &img);
    int get_width() const;
    int get_height() const;
    int get_bytespp() const;
    unsigned char *buffer();
    void clear();
};

#endif  //__IMAGE_H__