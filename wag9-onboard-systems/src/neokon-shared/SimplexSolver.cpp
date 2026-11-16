#include "SimplexSolver.h"

#include "neokon-shared/Profiler.h"

#include <algorithm>

//#define ENABLE_DEBUG_PRINT

namespace
{
// zero tolerances
static const double epsilon1 = 0.00001;
static const double epsilon2 = 0.00000001;

// number of constraints
//static size_t m;

// number of variables
//static size_t n;

// a structure that represents a basic variable
// containing its label (ranges from 0 to m + n - 1) and its current value
struct variable {
    size_t label;
    double value;
};

// a structure that represents an eta matrix
// containing the column that's different from an identity matrix (ranges from 1 to m - 1)
// and the values in that column
struct eta {
    size_t col;
    std::vector<double> values;
};

// print out the A matrix
void printMatrix(const double* matrix, size_t r, size_t c, size_t m, size_t n) {
    for (size_t row = 0; row < r; ++row) {
        for (size_t col = 0; col < c; ++col) {
            printf("%10.3f ", matrix[row * (m + n) + col]);
        }
        printf("\n");
    }
}

// print out the information of the LP, which includes
// number of variables, constraints, coefficients in the objective function,
// initial values of the basic variables and the A matrix
void printLPInfo(const double objFuncCoeff[], const variable b[], const double* matrix, size_t m,
    size_t n)
{
    printf("m = %zu ", m);
    printf("n = %zu \n", n);

    printf("c = ");

    for (size_t i = 0; i < m + n; ++i) {
        printf("%10.3f ", objFuncCoeff[i]);
    }

    printf("\nb = ");

    for (size_t i = 0; i < m; ++i) {
        printf("%10.3f ", b[i].value);
    }

    printf("\nA = \n");

    printMatrix(matrix, m, n + m, m, n);
};

// print out basic and non-basic variables for each iteration
void printVariables(size_t nonbasic[], variable b[], size_t m, size_t n) {
    printf("N = { ");

    for (size_t i = 0; i < n; ++i) {
        printf("x%zu ", nonbasic[i] + 1);
    }

    printf("} B = { ");

    // basic variables
    for (size_t i = 0; i < m; ++i) {
        printf("x%zu ", b[i].label + 1);
    }

    printf("}\n");
};

// print out the values of basic variables for each iteration
void printBbar(variable b[], size_t m) {
    printf("bbar = ");

    for (size_t i = 0; i < m; ++i) {
        printf("%10.3f ", b[i].value);
    }

    printf("\n");
}

// print out the final values of decision variables and slack variables
// when the optimal solution is reached
std::vector<double>
printFinalVariablesAndCalcSolution(variable b[], size_t nonbasic[], size_t m, size_t n)
{
    std::vector<double> solution(n);

    std::vector<double> varValues(m + n);

    for (size_t row = 0; row < m; ++row) {
        varValues[b[row].label] = b[row].value;
    }

    for (size_t col = 0; col < n; ++col) {
        varValues[nonbasic[col]] = 0.0;
    }
#ifdef ENABLE_DEBUG_PRINT
    // decision variables
    printf("Decision variables: ");
#endif // ENABLE_DEBUG_PRINT


    for (size_t i = 0; i < n; ++i) {
#ifdef ENABLE_DEBUG_PRINT
        printf("x%zu = %5.3f ", i + 1, varValues[i]);
#endif // ENABLE_DEBUG_PRINT
        solution[i] = varValues[i];
    }

#ifdef ENABLE_DEBUG_PRINT
    // slack variables
    printf("\nSlack variables: ");

    for (size_t i = n; i < m + n; ++i) {
        printf("x%zu = %5.3f ", i + 1, varValues[i]);
    }

    printf("\n");
#endif
    return solution;
}

// function to print out the family of solutions when the LP is found to be unbounded
void printFamilyOfSolutions(variable b[], size_t nonbasic[], std::vector<double> d,
    double largestCoeff, size_t enteringLabel, double z, size_t m, size_t n) {
    // in order to print out variable information in their natural order
    // we need to remember which row a basic variable locates in the basis
    // as to correspond it to the correct row in the entering variable's column in the dictionary
    struct varInfo {
        size_t label;
        double value;
        size_t row_in_basis;
        bool isBasic;
    };

    std::vector<varInfo> info(m + n);

    for (size_t row = 0; row < m; ++row) {
        variable v = b[row];
        info[v.label] = { v.label, v.value, row, true };
    }

    for (size_t col = 0; col < n; ++col) {
        info[nonbasic[col]] = { nonbasic[col], 0.0, 0, false };
    }

    // decision variables
    printf("Decision variables: ");

    for (size_t i = 0; i < n; ++i) {
        double solutionI = info[i].value;
        printf("x%zu = %5.3f ", i + 1, info[i].value);
        if (info[i].isBasic) {
            printf("+ %5.3fx%zu ", d[info[i].row_in_basis] * (-1.0), enteringLabel + 1);
            solutionI -= d[info[i].row_in_basis];
        }
    }

    // slack variables
    printf("\nSlack variables: ");

    for (size_t i = n; i < m + n; ++i) {
        printf("x%zu = %5.3f ", i + 1, info[i].value);
        if (info[i].isBasic) {
            printf("+ %5.3fx%zu ", d[info[i].row_in_basis] * (-1.0), enteringLabel + 1);
        }
    }

    // objective function
    printf("\nZ = %5.3f + %5.3fx%zu, ", z, largestCoeff, enteringLabel + 1);

    printf("with x%zu >= 0\n", enteringLabel + 1);
}


// a function for sorting variables in a vector
// into descending order
bool mComparator(variable v1, variable v2) {
    return v1.value > v2.value;
}

} // namespace

namespace simplex_solver
{

std::vector<double> SimplexSolver::solve(const Input& input)
{
    PROFILER_FUNCTION;

    // based on https://github.com/pakwah/Revised-Simplex-Method/blob/master/RevisedSimplex/main.cpp

    // Read in the number of constraints and
    // the number of variables
    //cin >> m >> n;

    size_t constraintCount = input.constraintVector.size(); // m
    size_t variableCount = input.objectiveFuncionCoefficients.size(); // n

    // Coefficients in the objective function
    std::vector<double> objFuncCoeff(input.objectiveFuncionCoefficients.size() +
        input.constraintVector.size());

    //double objFuncCoeff[n + m];

    // Read in the coefficients in the objective function
    for (size_t col = 0; col < input.objectiveFuncionCoefficients.size(); ++col)
    {
        //cin >> objFuncCoeff[col];
        objFuncCoeff[col] = input.objectiveFuncionCoefficients[col];
    }

    // Populate the rest of C to be 0
    for (size_t col = variableCount; col < constraintCount + variableCount; ++col)
    {
        objFuncCoeff[col] = 0.0;
    }

    // Matrix A represented as a 1-D array
    // with rows stacked next to each other
    //double A[m * (n + m)];
    Matrix A(input.constraintMatrix.rowCount(),
        input.constraintMatrix.columnCount() + input.constraintMatrix.rowCount());

    // Column of labels and values of the basic variables in the basic feasible solution
    //variable b[m];
    std::vector<variable> b(constraintCount);

    // Column of variable (only with labels, from 0 to m + n - 1) of the nonbasic variables
    std::vector<size_t> nonbasic(variableCount);

    // Initialize columns for decision variables in matrix A
    // Initialize b, the array of basic variables
    for (size_t row = 0; row < constraintCount; ++row) {
        for (size_t col = 0; col <= variableCount; ++col) {
            if (col == variableCount) {
                variable bVar = { variableCount + row, input.constraintVector[row] };
                b[row] = bVar;
            }
            else {
                A.set(row, col, input.constraintMatrix.at(row, col));
            }
        }
    }

    // Initialize columns for slack variables in matrix A
    for (size_t row = 0; row < constraintCount; ++row) {
        for (size_t col = 0; col < constraintCount; ++col) {
            if (col != row)
            {
                A.set(row, col + variableCount, 0.0); //base + col
            }
            else
            {
                A.set(row, col + variableCount, 1.0); //base + col
            }
        }
    }

    // Initialize the nonbasic variables' labels to be 0 through (n - 1)
    for (size_t i = 0; i < variableCount; ++i) {
        nonbasic[i] = i;
    }

#ifdef ENABLE_DEBUG_PRINT
    // Print out initial input values
    printLPInfo(objFuncCoeff.data(), b.data(), A.data(), constraintCount, variableCount);

    printf("\n\n");

    // Print out initial nonbasic and basic variables
    printVariables(nonbasic.data(), b.data(), constraintCount, variableCount);

    // Print out initial values of basic variables
    printBbar(b.data(), constraintCount);

    printf("\n");
#endif // ENABLE_DEBUG_PRINT

    // Check initial feasibility
    for (size_t row = 0; row < constraintCount; ++row) {
        if (b[row].value < 0.0) {
#ifdef ENABLE_DEBUG_PRINT
            printf("The given linear program is infeasible, exiting the program.\n");
#endif // ENABLE_DEBUG_PRINT
            return {};
        }
    }

    // Initial basic solution is feasible, now proceed with the Simplex Method

    // A counter to remember the current iteration number
    size_t counter = 1;

    // An array of eta matrices representing previous pivots
    std::vector<eta> pivots{};

    // Initial value of objective function
    double z = 0.0;

    // Revised Simplex Method
    while (true)
    {
#ifdef ENABLE_DEBUG_PRINT
        printf("Iteration%zu\n------------\n", counter);
#endif // ENABLE_DEBUG_PRINT
        // compute y using eta matrices (yB = Cb)
        std::vector<double> y(constraintCount);

        // initialize y to be Cb
        for (size_t row = 0; row < constraintCount; ++row)
        {
            variable v = b[row];
            y[row] = objFuncCoeff[v.label];
        }

        // solving y in yB = Cb
        for (auto rIter = pivots.crbegin(); rIter != pivots.crend(); ++rIter)
        {
            eta pivot = *rIter;
            size_t colToChange = pivot.col;
            double yOriginal = y[colToChange];

            for (size_t row = 0; row < pivot.values.size(); ++row) {
                if (row != colToChange) {
                    yOriginal -= pivot.values[row] * y[row];
                }
            }

            double yNew = yOriginal / pivot.values[colToChange];
            y[colToChange] = yNew;
        }

#ifdef ENABLE_DEBUG_PRINT
        // print out solved y
        printf("y = ");

        for (auto iter = y.cbegin(); iter != y.cend(); ++iter) {
            printf("%10.3f ", *iter);
        }

        printf("\n");
#endif // ENABLE_DEBUG_PRINT

        // choose an entering column
        // using the condition Cn > ya, where "a" is a column of An

        // a vector to keep track of the variables
        // whose coefficients in the objective function in this iteration are positive
        std::vector<variable> cnbars;

        size_t enteringLabel = nonbasic[0];
        double largestCoeff = -1.0;

#ifdef ENABLE_DEBUG_PRINT
        // print cnbar
        printf("cnbar: ");
#endif
        for (size_t i = 0; i < variableCount; ++i)
        {
            size_t varLabel = nonbasic[i];
            double cni = objFuncCoeff[varLabel];
            double yai = 0.0;

            for (size_t yIndex = 0; yIndex < constraintCount; ++yIndex)
            {
                yai += y[yIndex] * A.at(yIndex, varLabel);
            }

            double cnbar = cni - yai;
#ifdef ENABLE_DEBUG_PRINT
            printf("x%zu %5.3f ", varLabel + 1, cnbar);
#endif // ENABLE_DEBUG_PRINT
            if (cnbar > epsilon1) {
                variable v = { varLabel, cnbar };

                cnbars.push_back(v);

                if (cnbar > largestCoeff) {
                    largestCoeff = cnbar;
                    enteringLabel = varLabel;
                }
            }
        }

        // sort the variables into descending order
        // based on their coefficients in the objective function
        sort(cnbars.begin(), cnbars.end(), mComparator);

#ifdef ENABLE_DEBUG_PRINT
        printf("\n");
#endif // ENABLE_DEBUG_PRINT

        // If the vector cnbars is empty, then there are no candidates for the entering variable
        if (cnbars.size() == 0)
        {
#ifdef ENABLE_DEBUG_PRINT
            printf("\nNo entering var. Optimal value of %5.3f has been reached.\n", z);
#endif // ENABLE_DEBUG_PRINT
            return printFinalVariablesAndCalcSolution(b.data(), nonbasic.data(), constraintCount,
                variableCount);
        }
        else
        {
#ifdef ENABLE_DEBUG_PRINT
            printf("Entering variable is x%zu \n", enteringLabel + 1);
#endif // ENABLE_DEBUG_PRINT
        }

        size_t enteringVariable_index = 0;

        // compute the column d in Anbar
        // for the entering variable
        // using eta matrices (Bd = a)
        std::vector<double> d(constraintCount);

        size_t leavingLabel;
        size_t leavingRow;
        double smallest_t;

        while (true)
        {
            leavingLabel = -1;
            leavingRow = -1;
            smallest_t = -1;

#ifdef ENABLE_DEBUG_PRINT
            if (enteringVariable_index > 0)
            {
                printf("\n\nRechoosing entering variable since the diagonal element in the eta column is close to zero.\n");
            }
#endif // ENABLE_DEBUG_PRINT
            if (enteringVariable_index < cnbars.size()) {
                enteringLabel = cnbars[enteringVariable_index].label;

#ifdef ENABLE_DEBUG_PRINT
                if (enteringVariable_index > 0) {
                    printf("Entering variable is x%zu \n", enteringLabel + 1);
                }
#endif // ENABLE_DEBUG_PRINT
            }
            else {
#ifdef ENABLE_DEBUG_PRINT
                printf("\nNo entering var. Optimal value of %5.3f has been reached.\n", z);
#endif // ENABLE_DEBUG_PRINT
                return printFinalVariablesAndCalcSolution(b.data(), nonbasic.data(), constraintCount,
                    variableCount);
            }

            // initialize d to be the entering column a
            for (size_t row = 0; row < constraintCount; ++row)
            {
                d[row] = A.at(row, enteringLabel);
            }

            // Go through eta matrices from pivot 1 to pivot k
            for (auto iter = pivots.cbegin(); iter != pivots.cend(); ++iter)
            {
                eta pivot = *iter;
                size_t rowToChange = pivot.col;
                double dOriginal = d[rowToChange];

                d[rowToChange] = dOriginal / pivot.values[rowToChange];

                for (size_t row = 0; row < d.size(); ++row) {
                    if (row != rowToChange) {
                        d[row] = d[row] - pivot.values[row] * d[rowToChange];
                    }
                }
            }

#ifdef ENABLE_DEBUG_PRINT
            // print out d (abarj)
            printf("d = ");

            for (auto iter = d.cbegin(); iter != d.cend(); ++iter) {
                printf("%5.3f ", *iter);
            }

            printf("\n");
#endif // ENABLE_DEBUG_PRINT

            // compute t for each b[i].value / d[i]
            // where d[i] > 0
            // choose the corresponding i for the smallest ratio
            // as the leaving variable

            // initialize smallest_t to be the first ratio where
            // the coefficient of the entering variable in that row is negative
            for (size_t row = 0; row < d.size(); ++row)
            {
                if (d[row] > 0.0)
                {
                    leavingLabel = b[row].label;
                    leavingRow = row;
                    smallest_t = b[row].value / d[row];
                }
            }

            // if no ratio is computed, then the LP is unbounded
            if (leavingLabel == -1)
            {
#ifdef ENABLE_DEBUG_PRINT
                printf("\nThe given LP is unbounded. The family of solutions is:\n");
                printFamilyOfSolutions(b.data(), nonbasic.data(), d, largestCoeff,
                    enteringLabel, z, constraintCount, variableCount);
#endif // ENABLE_DEBUG_PRINT
                return {};
            }

            // there is at least one ratio computed, print out the ratio(s)
            // and choose the row corresponding to the smallest ratio to leave
#ifdef ENABLE_DEBUG_PRINT
            printf("ratio: ");
#endif // ENABLE_DEBUG_PRINT
            for (size_t row = 0; row < d.size(); ++row) {
                if (d[row] < 0.0) {
                    continue;
                }

                double t_row = b[row].value / d[row];

#ifdef ENABLE_DEBUG_PRINT
                if (t_row >= 0.0) {
                    printf("x%zu %5.3f ", b[row].label + 1, t_row);
                }
#endif // ENABLE_DEBUG_PRINT

                if (t_row < smallest_t) {
                    leavingLabel = b[row].label;
                    leavingRow = row;
                    smallest_t = t_row;
                }
            }

            // check the diagonal element in the eta column
            // to see if the current choice of entering variable has to be rejected
            if (d[leavingRow] > epsilon2) {
#ifdef ENABLE_DEBUG_PRINT
                printf("\nLeaving variable is x%zu\n", leavingLabel + 1);
#endif // ENABLE_DEBUG_PRINT
                break;
            }
            else {
                enteringVariable_index++;
                continue;
            }
        }

        // At this point we have a pair of entering and leaving variables
        // so that the entering variable is positive and the diagonal entry in the eta column
        // of the eta matrix is fairly far from zero

        // set the value of the entering varaible at t
        // modify b (change leaving variable to entering variable, change values of other basic vars)
        variable enteringVar = { enteringLabel, smallest_t };
        b[leavingRow] = enteringVar;

        for (size_t row = 0; row < b.size(); ++row) {
            if (row != leavingRow) {
                b[row].value -= d[row] * smallest_t;
            }
        }

        // push a new eta matrix onto the vector
        eta pivot = { leavingRow, d };
        pivots.push_back(pivot);

#ifdef ENABLE_DEBUG_PRINT
        // print out the eta matrix representing the pivot at this iteration
        printf("E%zu = column %zu: ", counter, leavingRow);

        for (auto iter = d.cbegin(); iter != d.cend(); ++iter) {
            printf("%5.3f ", *iter);
        }

        printf("\n");
#endif // ENABLE_DEBUG_PRINT
        // original (wrong) code to put variable out of list of the basis variables:
        //nonbasic[enteringLabel] = leavingLabel;

        bool variablePutOutOfBasis = false;
        for (auto& nonBasicLabel : nonbasic)
        {
            if (nonBasicLabel == enteringLabel)
            {
                nonBasicLabel = leavingLabel;

                variablePutOutOfBasis = true;
            }
        }
        assert(variablePutOutOfBasis);

#ifdef ENABLE_DEBUG_PRINT
        // print out nonbasic and basic variable set after the pivot
        printVariables(nonbasic.data(), b.data(), constraintCount, variableCount);

        // print out the new values of the basic variables
        printBbar(b.data(), constraintCount);

        // print out the coefficient of the entering variable and the amount the entering variable has been increased
        printf("\nCoefficient of entering variable: %5.3f\nAmount increased for the entering variable is: %5.3f\n",
            largestCoeff, smallest_t);
#endif // ENABLE_DEBUG_PRINT

        // increase the value of the objective function
        double increasedValue = largestCoeff * smallest_t;

#ifdef ENABLE_DEBUG_PRINT
        // print out the update to the objective function value
        printf("Increased value: %5.3f\n", increasedValue);
#endif // ENABLE_DEBUG_PRINT

        double originalZ = z;

        z += increasedValue;

#ifdef ENABLE_DEBUG_PRINT
        printf("Value of the objective function changed from %5.3f to %5.3f\n\n\n", originalZ, z);
#endif // ENABLE_DEBUG_PRINT

        counter++;
    }
    return {};
}

} // namespace simplex_solver
