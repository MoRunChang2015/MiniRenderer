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
            std::vector<int> face;
            int itrash, idx;
            iss >> trash;
            while (iss >> idx >> trash >> itrash >> trash >> itrash) {
                --idx;  // start from 1 not 0
                face.push_back(idx);
            }
            faces.push_back(std::move(face));
        }
    }
}

Model::~Model() = default;

int Model::nverts() const { return static_cast<int>(verts.size()); }

int Model::nfaces() const { return static_cast<int>(faces.size()); }

std::vector<int> Model::face(const int& idx) const { return faces[idx]; }

Vec3f Model::vert(const int& idx) const { return verts[idx]; }
