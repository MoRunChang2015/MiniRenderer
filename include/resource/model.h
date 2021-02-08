#pragma once

#include <vector>

#include "util/geometry.h"

class Model {
   public:
    Model(const std::string& filename);
    ~Model();

    int nverts() const;
    int nfaces() const;

    Vec3f vert(const int& i) const;
    std::vector<int> face(const int& i) const;

   private:
    std::vector<Vec3f> verts;
    std::vector<std::vector<int>> faces;
};