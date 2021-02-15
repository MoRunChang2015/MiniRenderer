#pragma once

#include <vector>

#include "resource/material.h"
#include "resource/mesh.h"
#include "util/geometry.h"

class Model {
   public:
    Model(Mesh* mesh, Material* material) : meshRes(mesh), materialRes(material), transform(Matrix4x4::identity()){};

    Mesh* getMesh() const { return meshRes; }
    Material* getMaterial() const { return materialRes; }

    const Matrix4x4& getTransform() { return transform; }
    void setTransform(const Matrix4x4& m) { transform = m; }

   private:
    Mesh* meshRes{nullptr};
    Material* materialRes{nullptr};
    Matrix4x4 transform;
};