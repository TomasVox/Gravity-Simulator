#include "vec.h"
#include "mat.h"
#include <cmath>


template <size_t Size>
inline vec<Size> normalize(const vec<Size>& vector)
{
    double sum = 0.0f;
    for (size_t i = 0; i<Size; i++)
        sum += pow(vector[i], 2);
    double magnitude = sqrt(sum);

    vec<Size> result;
    for (size_t i = 0; i<Size; i++)
        result[i] = vector[i] / magnitude;
    
    return result;
}

inline matrix identity (matrix& mat)
{
    if (mat.Rows() != mat.Columns())
        throw std::runtime_error("Matrix couldn't be transformed to identity matrix due its size");

    matrix result(mat.Rows(), mat.Columns());
    for (size_t i = 0; i<mat.Rows(); i++)
        result(i, i) = 1;
    return result;
}

inline matrix rotate (matrix mat, double degrees, vec<3> axis)
{
    const double PI = 3.141592653;
    const double rads = degrees * PI / 180;
    
    // Normalizacion de eje de rotación
    axis = normalize(axis);

    matrix I(4, 4); // identity matrix
    I = identity(I);
    
    matrix K(4, 4); // anti-simetric matrix
    K(0, 1) = -axis[2];
    K(0, 2) = axis[1];
    K(1, 0) = axis[2];
    K(1, 2) = -axis[0];
    K(2, 0) = -axis[1];
    K(2, 1) = axis[0];

    matrix rotationMatrix(4, 4);
    rotationMatrix = I + K * std::sin(rads) + (K*K) * (1 - std::cos(rads)); // Formula de rodriguez

    matrix result(4,4);
    result = mat * rotationMatrix;

    return result;
}

inline matrix perspective(float fov, float aspect, float near, float far)
{
    const double PI = 3.141592653;
    const double rads = fov * PI / 180.0f;

    const double f = 1 / tan(rads/2.0f);

    matrix P(4, 4);
    P(0,0) = f/aspect;
    P(1, 1) = f;
    P(2, 2) = -((far + near) / (far - near));
    P(2, 3) = -((2.0 * far * near) / (far - near));
    P(3, 2) = -1;

    return P;
}

inline matrix lookAt(const vec<3>& cameraPos, const vec<3>& target, const vec<3>& up)
{
    vec<3> container;

    vec<3> z = normalize(cameraPos - target);
    vec<3> x = normalize(cross(up, z));
    vec<3> y = normalize(cross(z, x));

    matrix A(4,4);
    matrix B(4,4);
    B = identity(B);

    A(0,0) = x[0]; A(0,1) = x[1]; A(0,2) = x[2];
    A(1,0) = y[0]; A(1,1) = y[1]; A(1,2) = y[2];
    A(2,0) = z[0]; A(2,1) = z[1]; A(2,2) = z[2];

    B(0, 3) = -cameraPos[0];
    B(1, 3) = -cameraPos[1];
    B(2, 3) = -cameraPos[2];

    return A * B;
}

std::vector<vec<3>> drawSphere(unsigned int stacks, unsigned int sectors)
{
    std::vector<vec<3>> vertices;
    std::vector<vec<3>> indices;
    const float PI = 3.14159265359f;

    for (unsigned int i = 0; i <= stacks; ++i) {
        float phi = PI * i / stacks;
        for (unsigned int j = 0; j <= sectors; ++j) {
            float theta = 2.0f * PI * j / sectors;

            float x = sinf(phi) * cosf(theta);
            float y = sinf(phi) * sinf(theta);
            float z = cosf(phi);

            vertices.push_back({x, y, z});
        }
    }
    for (unsigned int i = 0; i < stacks; ++i)
    {
        size_t f_index = i * (sectors + 1);
        size_t s_index = f_index + sectors + 1;
        for (unsigned int j = 0; j < sectors; ++j, ++f_index, ++s_index)
        {
            if (i != 0) {
                indices.push_back(vertices[f_index]);
                indices.push_back(vertices[s_index]);
                indices.push_back(vertices[f_index + 1]);
            }

            // Excluir el segundo triángulo en los polos (degenerados)
            if (i != (stacks - 1)) {
                indices.push_back(vertices[f_index + 1]);
                indices.push_back(vertices[s_index]);
                indices.push_back(vertices[s_index + 1]);
            }
        }
    }

    return indices;
}