#pragma once
#include <cmath>
#include <iostream>
#include <vector>

template <typename T>
struct Vector2 {
    union {
        struct {
            T u, v;
        };
        struct {
            T x, y;
        };
        T raw[2];
    };

    Vector2() : x(0), y(0){};
    Vector2(T u, T v) : x(u), y(v){};
    inline Vector2<T> operator+(const Vector2<T>& other) const { return Vector2<T>(x + other.x, y + other.y); }
    inline Vector2<T> operator-(const Vector2<T>& other) const { return Vector2<T>(x - other.x, y - other.y); }
    inline Vector2<T> operator*(const float& s) const { return Vector2<T>(x * s, y * s); }
    inline T& operator[](int idx) { return raw[idx]; }
    inline const T& operator[](int idx) const { return raw[idx]; }

    template <typename>
    friend std::ostream& operator<<(std::ostream& s, Vector2<T>& v);
};

template <typename T>
struct Vector3 {
    union {
        struct {
            T x, y, z;
        };
        struct {
            T ivert, iuv, inorm;
        };
        T raw[3];
    };

    Vector3() : x(0), y(0), z(0){};
    Vector3(T u, T v, T w) : x(u), y(v), z(w){};
    inline Vector3<T> operator+(const Vector3<T>& other) const {
        return Vector3<T>(x + other.x, y + other.y, z + other.z);
    }
    inline Vector3<T> operator-(const Vector3<T>& other) const {
        return Vector3<T>(x - other.x, y - other.y, z - other.z);
    }
    inline Vector3<T> operator*(const float& s) const { return Vector3<T>(x * s, y * s, z * s); }

    inline T operator*(const Vector3<T>& other) const { return x * other.x + y * other.y + z * other.z; }

    inline T& operator[](int idx) { return raw[idx]; }
    inline const T& operator[](int idx) const { return raw[idx]; }

    inline Vector3<T> operator^(const Vector3<T>& other) const {
        return Vector3<T>(y * other.z - z * other.y, z * other.x - x * other.z, x * other.y - y * other.x);
    }

    float norm() const { return std::sqrt(x * x + y * y + z * z); }
    Vector3<T>& normalize(T l = 1) {
        *this = (*this) * (1 / norm());
        return *this;
    }

    Vector3<T> round() const { return Vector3<T>(std::round(x), std::round(y), std::round(z)); }

    template <typename>
    friend std::ostream& operator<<(std::ostream& s, Vector2<T>& v);
};

template <typename T>
std::ostream& operator<<(std::ostream& s, Vector2<T>& v) {
    s << "(" << v.x << ", " << v.y << ")" << std::endl;
    return s;
}

template <typename T>
std::ostream& operator<<(std::ostream& s, Vector3<T>& v) {
    s << "(" << v.x << ", " << v.y << ", " << v.z << ")" << std::endl;
    return s;
}

using Vec2f = Vector2<float>;
using Vec2i = Vector2<int>;
using Vec3f = Vector3<float>;
using Vec3i = Vector3<int>;

class Matrix {
   public:
    Matrix(const int& n_rows, const int& n_cols);
    ~Matrix() = default;
    inline int nrows() const { return rows; }
    inline int ncols() const { return cols; }
    Matrix operator*(const Matrix& other) const;
    std::vector<float>& operator[](const int& idx) { return raws[idx]; }
    const std::vector<float>& operator[](const int& idx) const { return raws[idx]; };
    Matrix transpose() const;
    Matrix inverse() const;

    static Matrix identity(const int& dim);

   private:
    std::vector<std::vector<float>> raws;
    int rows;
    int cols;
};

std::ostream& operator<<(std::ostream& s, Matrix& m);