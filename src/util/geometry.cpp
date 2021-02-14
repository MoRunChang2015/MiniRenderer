#include "util/geometry.h"
#include <cassert>

Matrix::Matrix(const int& n_rows, const int& n_cols)
    : raws(n_rows, std::vector<float>(n_cols, 0.f)), rows(n_rows), cols(n_cols){};

Matrix Matrix::identity(const int& dim) {
    Matrix m(dim, dim);
    for (int i = 0; i < dim; ++i) m[i][i] = 1;
    return m;
}

Matrix Matrix::transpose() const {
    Matrix result(cols, rows);
    for (int i = 0; i < rows; i++)
        for (int j = 0; j < cols; j++) result[j][i] = raws[i][j];
    return result;
}

Matrix Matrix::inverse() const {
    assert(rows == cols);
    // augmenting the square matrix with the identity matrix of the same dimensions a => [ai]
    Matrix result(rows, cols * 2);
    for (int i = 0; i < rows; i++)
        for (int j = 0; j < cols; j++) result[i][j] = raws[i][j];
    for (int i = 0; i < rows; i++) result[i][i + cols] = 1;
    // first pass
    for (int i = 0; i < rows - 1; i++) {
        // normalize the first row
        for (int j = result.cols - 1; j >= 0; j--) result[i][j] /= result[i][i];
        for (int k = i + 1; k < rows; k++) {
            float coeff = result[k][i];
            for (int j = 0; j < result.cols; j++) {
                result[k][j] -= result[i][j] * coeff;
            }
        }
    }
    // normalize the last row
    for (int j = result.cols - 1; j >= rows - 1; j--) result[rows - 1][j] /= result[rows - 1][rows - 1];
    // second pass
    for (int i = rows - 1; i > 0; i--) {
        for (int k = i - 1; k >= 0; k--) {
            float coeff = result[k][i];
            for (int j = 0; j < result.cols; j++) {
                result[k][j] -= result[i][j] * coeff;
            }
        }
    }
    // cut the identity matrix back
    Matrix truncate(rows, cols);
    for (int i = 0; i < rows; i++)
        for (int j = 0; j < cols; j++) truncate[i][j] = result[i][j + cols];
    return truncate;
}
Matrix Matrix::operator*(const Matrix& other) const {
    assert(cols == other.rows);
    Matrix result(rows, other.cols);
    for (int i = 0; i < rows; i++) {
        for (int j = 0; j < other.cols; j++) {
            result[i][j] = 0.f;
            for (int k = 0; k < cols; k++) {
                result[i][j] += raws[i][k] * other[k][j];
            }
        }
    }
    return result;
}

std::ostream& operator<<(std::ostream& s, Matrix& m) {
    for (int i = 0; i < m.nrows(); i++) {
        for (int j = 0; j < m.ncols(); j++) {
            s << m[i][j];
            if (j < m.ncols() - 1) s << "\t";
        }
        s << "\n";
    }
    return s;
}
