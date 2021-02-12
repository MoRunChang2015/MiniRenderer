#include "resource/model.h"

#include <fstream>
#include <sstream>
#include <string>

Model::Model(const std::string& filename) {
    std::ifstream in;
    in.open(filename, std::ios::in);
    if (in.fail()) return;
    std::string line;
    while (!in.eof()) {
        std::getline(in, line);
        std::istringstream iss(line);
        char trash;
        if (!line.compare(0, 2, "v ")) {
            iss >> trash;
            Vec3f v;
            for (float& i : v.raw) {
                iss >> i;
            }
            verts.emplace_back(v);
        } else if (!line.compare(0, 2, "f ")) {
            std::vector<Vertex> face;
            int itrash, idx, uv_idx, normal_idx;
            iss >> trash;
            while (iss >> idx >> trash >> uv_idx >> trash >> normal_idx) {
                --idx;  // start from 1 not 0
                --uv_idx;
                --normal_idx;
                face.emplace_back(idx, uv_idx, normal_idx);
            }
            faces.push_back(std::move(face));
        } else if (!line.compare(0, 2, "vt")) {
            iss >> trash >> trash;
            Vec3f v;
            for (float& i : v.raw) {
                iss >> i;
            }
            uvs.emplace_back(v);
        } else if (!line.compare(0, 2, "vn")) {
            iss >> trash >> trash;
            Vec3f v;
            for (float& i : v.raw) {
                iss >> i;
            }
            normals.emplace_back(v);
        }
    }
}

Model::~Model() = default;

int Model::nverts() const { return static_cast<int>(verts.size()); }

int Model::nfaces() const { return static_cast<int>(faces.size()); }

Vec3f Model::vert(const int& idx) const { return verts[idx]; }

Vec3f Model::normal(const int& idx) const { return normals[idx]; }
Vec3f Model::uv(const int& idx) const { return uvs[idx]; }

std::vector<int> Model::face_vertex(const int& idx) const {
    std::vector<int> ans;
    for (const auto& vertex : face(idx)) {
        ans.emplace_back(vertex.vertIdx());
    }
    return ans;
}
std::vector<int> Model::face_uv(const int& idx) const {
    std::vector<int> ans;
    for (const auto& vertex : face(idx)) {
        ans.emplace_back(vertex.uvIdx());
    }
    return ans;
}
std::vector<int> Model::face_normal(const int& idx) const {
    std::vector<int> ans;
    for (const auto& vertex : face(idx)) {
        ans.emplace_back(vertex.normalIdx());
    }
    return ans;
}

const std::vector<Model::Vertex>& Model::face(const int& idx) const { return faces[idx]; }
