#pragma once

#include <cassert>
#include <vector>
#include <unordered_map>

namespace simplex_solver
{

class Matrix
{
public:
    Matrix(int rowCount, int columnCount)
        : m_rowCount(rowCount)
        , m_columnCount(columnCount)
    {
        m_data.resize(m_columnCount * m_rowCount);
        for (size_t index = 0; index < m_data.size(); ++index)
        {
            m_data[index] = 0.0;
        }
    }

    double at(int row, int column) const
    {
        assert(row < m_rowCount&& column < m_columnCount);
        return m_data[row * m_columnCount + column];
    }

    void set(int row, int column, double value)
    {
        m_data[row * m_columnCount + column] = value;
    }

    int rowCount() const { return m_rowCount; }
    int columnCount() const { return m_columnCount; }
    const double* data() const { return m_data.data(); }

private:
    int m_rowCount{ 0 }, m_columnCount{ 0 };

    std::vector<double> m_data;
};

struct MatrixRow
{
    Matrix& matrix;
    int rowIndex = -1;

    MatrixRow(Matrix& mtrx, int row)
        : matrix(mtrx), rowIndex(row)
    {
    }

    void clear()
    {
        for (int columnIndex = 0; columnIndex < matrix.columnCount(); ++columnIndex)
        {
            matrix.set(rowIndex, columnIndex, 0.0);
        }
    }
};

class SimplexSolver
{
public:
    struct Input
    {
        std::vector<double> objectiveFuncionCoefficients; // variables c^T

        Matrix constraintMatrix; // A TODO: replace with SparseMatrix

        std::vector<double> constraintVector; // b

        Input(std::vector<double> const& objectiveFunction,
            Matrix const& constraintMatrix,
            std::vector<double> const& constraintVector)
            : objectiveFuncionCoefficients(objectiveFunction)
            , constraintMatrix(constraintMatrix)
            , constraintVector(constraintVector)
        {}
    };

    /**
     * Solves the Linear programming problem:
     * c^T * x -> max, Ax <= b, x >= 0, b >= 0
     *
     * and returns a vector of the resulting variables 'x' in the same order as the initial
     * variables 'c'
     *
     */
    std::vector<double> solve(const Input& input);
};

}
