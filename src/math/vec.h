#pragma once
#include <cstddef>
#include <initializer_list>
#include "mat.h"

template <size_t Size>
class vec {
    private:
        float data[Size] = {};
    public:
        float* ptr() { return data; }
        const float* ptr() const { return data; }

        explicit vec(float element)
        {
            for (size_t i = 0; i<Size; i++)
            {
                data[i] = element;
            }
        }

        vec(std::initializer_list<float> list)
        {
            size_t i = 0;
            for (auto& item : list)
            {
                if (i < Size) data[i++] = item;
            }
        }

        const float& operator[](size_t i) const { return data[i]; }
        float& operator[](size_t i) { return data[i]; }

        vec() = default;

        vec& operator+=(const vec& other)
        {
            for (size_t i = 0; i<Size; i++)
            {
                data[i] += other[i];
            }
            return *this;
        }
        vec operator+(const vec& other) const {
            vec result = *this;
            result += other;
            return result;
        }

        vec& operator-=(const vec& other) {
            for (size_t i = 0; i<Size; i++)
            {
                data[i] -= other[i];
            }
            return *this;
        }
        vec operator-(const vec& other) const {
            vec result = *this;
            result -= other;
            return result;
        }

        vec& operator*=(const float lambda)
        {
            for (size_t i = 0; i<Size; i++)
            {
                data[i] *= lambda;
            }
            return *this;
        }
        vec operator*(const float lambda) const
        {
            vec result = *this;
            result *= lambda;
            return result;
        }
        vec operator*(const matrix& other) const 
        {
            /*if (other.Rows() != Size)
                throw std::runtime_error("Not compatible matrix and vector to perfom multiplication");

            vec<Size> result(0.0f);

            for (size_t i = 0; i<other.Columns(); i++)
            {
                float sum = 0.0f;
                for (size_t j = 0; j<Size; j++)
                {
                    sum += data[j] * other(j, i);
                }
                result[i] = sum;
            }
            return result;
        }*/
            if (other.Columns() != Size)
                throw std::runtime_error("Dimensiones incompatibles: A.cols debe ser igual a x.size");

            // El resultado tiene tantas filas como la matriz (m)
            vec<Size> result(0.0f); //cambiar dimension del vector ⚠️

            for (size_t i = 0; i < other.Rows(); i++) {
                float sum = 0.0f;
                for (size_t j = 0; j < Size; j++) {
                    // A_{ij} * x_j
                    sum += other(i, j) * data[j];
                }
                result[i] = sum;
            }
            return result;
        }
};

template <size_t Size>
vec<Size> cross(const vec<Size>& a, const vec<Size>& b) = delete;

template <>
inline vec<3> cross(const vec<3>& a, const vec<3>& b)
{
    return {
        a[1] * b[2] - a[2] * b[1],
        a[2] * b[0] - a[0] * b[2],
        a[0] * b[1] - a[1] * b[0]
    };
}