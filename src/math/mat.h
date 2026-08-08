#pragma once

#include <cstddef>
#include <initializer_list>
#include <vector>
#include <stdexcept>
#include <variant>
#include "vec.h"

class matrix {
    private:
        size_t rows = 0;
        size_t columns = 0;
        std::vector<float> data;
    public:
        matrix() {}
        matrix(size_t r, size_t c) : rows(r), columns(c), data(r * c, 0) {}
        matrix(size_t r, size_t c, float value) : rows(r), columns(c), data(r * c, value) {}
        matrix(std::initializer_list<std::initializer_list<float>> values)
        {
            rows = values.size();
            columns = rows > 0 ? values.begin()->size() : 0;

            data.reserve(rows * columns);

            for(const auto& row : values)
            {
                if (row.size() != columns)
                    throw std::runtime_error("Rows have different sizes");
                
                for (float value : row)
                    data.push_back(value);
            }
        }
        matrix(const matrix& other) = default;
        matrix(matrix&& other) = default;

        float& operator()(size_t row, size_t col)
        {
            return data[row * columns + col];
        }
        const float& operator()(size_t row, size_t col) const
        {
            return data[row * columns + col];
        }

        /*
            ⚠️ Puede cambiar el orden de multiplicacion de matrices.
        */ 
        matrix operator*(const matrix& other) const
        {
            if (columns != other.rows)
                throw std::runtime_error("Invalid matrix dimensions");               
            
            matrix result(rows, other.columns);

            for (size_t i = 0; i<rows; i++)
            {
                for (size_t j = 0; j<other.columns; j++)
                {
                    float sum = 0.0f;

                    for (size_t k = 0; k<columns; k++)
                    {
                        sum += (*this)(i, k) * other(k, j);
                    }
                    result(i, j) = sum;
                }
            }
            return result;
        }
        matrix operator*(const double lambda)
        {
            matrix result = *this;

            for (size_t i = 0; i<rows; i++)
                for (size_t j = 0; j<columns; j++)
                    result(i, j) *= lambda;
            return result;
        }

        matrix operator+(const matrix& other)
        {
            if (other.columns != columns || other.rows != rows)
                throw std::runtime_error("Not compatible dimensions to perform addition.");
            matrix result = *this;
            
            for (size_t i = 0; i<rows; i++)
                for (size_t j = 0; j<columns; j++)
                    result(i, j) += other(i, j);
            return result;
        }

        matrix operator=(const matrix& other)
        {
            data.reserve(other.columns * other.rows);
            data = other.data;

            return *this;
        }
        matrix& operator=(matrix&& other) noexcept {
            if (this != &other) {
                // Intercambiamos los datos directamente. 
                // El temporal se lleva la "basura" y nosotros los datos reales.
                rows = other.rows;
                columns = other.columns;
                data = std::move(other.data); 
                
                // Opcional: resetear el otro para limpieza
                other.rows = 0;
                other.columns = 0;
            }
            return *this;
        }

        size_t Rows() const
        {
            return this->rows;
        }
        size_t Columns() const
        {
            return this->columns;
        }

        const float* data_ptr() const {
            return this->data.data();
        }

        size_t size_bytes() const {
            return this->data.size() * sizeof(float);
        }
};