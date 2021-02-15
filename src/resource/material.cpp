#include "resource/material.h"
#include <iostream>
#include <string>

void load_texture(const std::string& filename, TGAImage& out) {
    std::cout << "Texture file " << filename << " loading " << (out.read_tga_file(filename.c_str()) ? " ok " : " fail ")
              << std::endl;
}

Material::Material(const std::string& diffuseFile, const std::string& normalFile, const std::string& specularFile) {
    load_texture(diffuseFile, diffuseMap);
    diffuseMap.flip_vertically();
    if (!normalFile.empty()) {
        load_texture(normalFile, normalMap);
        normalMap.flip_vertically();
    }
    if (!specularFile.empty()) {
        load_texture(specularFile, specularMap);
        specularMap.flip_vertically();
    }
}

TGAColor Material::diffuse(Vec2f uv) {
    Vec2i uvi(static_cast<int>(uv[0] * diffuseMap.get_width()), static_cast<int>(uv[1] * diffuseMap.get_height()));
    return diffuseMap.get(uvi[0], uvi[1]);
}
Vec3f Material::normal(Vec2f uv) {
    Vec2i uvi(static_cast<int>(uv[0] * normalMap.get_width()), static_cast<int>(uv[1] * normalMap.get_height()));
    TGAColor c = normalMap.get(uvi[0], uvi[1]);
    Vec3f ret;
    for (size_t i = 0; i < 3; ++i) {
        ret[2 - i] = static_cast<float>(c[i] / 255.0f * 2.f - 1.f);
    }
    return ret;
}
float Material::specular(Vec2f uv) {
    Vec2i uvi(static_cast<int>(uv[0] * specularMap.get_width()), static_cast<int>(uv[1] * specularMap.get_height()));
    return specularMap.get(uvi[0], uvi[1])[0] / 1.0f;
}