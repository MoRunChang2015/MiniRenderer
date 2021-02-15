#pragma once

#include <vector>

#include "util/geometry.h"

class Mesh {
   public:
    Mesh(const std::string& filename);
    ~Mesh();

    struct Vertex {
        int raw[3];

        Vertex(int vIdx, int uvIdx, int nIdx) : raw{vIdx, uvIdx, nIdx} {}
        int vertIdx() const { return raw[0]; }
        int uvIdx() const { return raw[1]; }
        int normalIdx() const { return raw[2]; }
    };

    size_t nverts() const;
    size_t nfaces() const;

    Vec3f vert(const int& idx) const;
    Vec3f normal(const int& idx) const;
    Vec2f uv(const int& idx) const;
    std::vector<int> face_vertex(const int& idx) const;
    std::vector<int> face_uv(const int& idx) const;
    std::vector<int> face_normal(const int& idx) const;
    const std::vector<Vertex>& face(const int& idx) const;

   private:
    std::vector<Vec3f> verts;
    std::vector<Vec3f> uvs;
    std::vector<Vec3f> normals;
    std::vector<std::vector<Vertex>> faces;
};