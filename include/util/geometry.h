#pragma once
#include <cmath>
#include <iostream>

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
    Vector3<T>& noramlize(T l = 1) {
        *this = (*this) * (1 / norm());
        return *this;
    }

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