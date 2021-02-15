#pragma once
#include "util/geometry.h"
#include "util/tgaImage.h"

class Material {
   public:
    Material(const std::string& diffuseFile, const std::string& normalFile = "", const std::string& specularFile = "");
    ~Material() = default;

    TGAColor diffuse(Vec2f uv);
    Vec3f normal(Vec2f uv);
    float specular(Vec2f uv);

   private:
    TGAImage diffuseMap;
    TGAImage normalMap;
    TGAImage specularMap;
};