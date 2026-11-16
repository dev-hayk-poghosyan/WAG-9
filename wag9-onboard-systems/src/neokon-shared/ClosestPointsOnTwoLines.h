#pragma once

#include <utility>

/**
 * Finds a point on each of the given lines, so that these two points are as close
 * as possible to each other.
 *
 * Our lines are given as (origin, vector) pairs. The origin may be any point on the line
 * and the vector may be any non-zero vector, not necessarily a unit length one.
 *
 * This function returns a tuple of scalars u and v, such as:
 * @code
 * p1 = line1Origin + u * line1Vector;
 * p2 = line2Origin + v * line2Vector;
 * // The pair of points (p1, p2) has the minimum euclidean distance among all possible pairs.
 * @endcode
 *
 * The return value is the (u, v) std::pair.
 *
 * @note If the two lines are parralel or nearly parallel, the return values are undefined.
 *
 * @note This function works with both GLM and Unigine::Math vector types.
 */
template<typename Vec>
auto closestPointsOnTwoLines(
    Vec const& line1Origin, Vec const& line1Vector,
    Vec const& line2Origin, Vec const& line2Vector)
{
    /*
    Let's denote the following:
    a = line1Origin
    b = line1Vector
    c = line2Origin
    d = line2Vector

    The scalars u and v define the points on each line:

    pointOnLine1(u) = a + u * b
    pointOnLine2(v) = c + v * d

    So, we have to minimise the following function with respect to u and v:

    F = ||a + u * b - (c + v * d)||^2

    Minimising the squared distance gives us the same u and v as if we minimised the
    actual distance.

    Assuming 3D vectors, our function to minimise becomes:

    F = (a_x + u * b_x - c_x - v * d_x)^2 + <same for y> + <same for z>

    Taking partial derivatives with respect to u and v:

    dF/du = 2*(a_x + u * b_x - c_x - v * d_x) * b_x + <same for y> + <same for z>
    dF/dv = 2*(a_x + u * b_x - c_x - v * d_x) * (-d_x) + <same for y> <same for z>

    // Opening parentheses:

    dF/du = 2*a_x*b_x + 2*u*b_x*b_x - 2*c_x*b_x - 2*v*d_x*b_x + <same for y> + <same for z>
    dF/dv = -2*a_x*d_x - 2*u*b_x*d_x + 2*c_x*d_x + 2*v*d_x*d_x + <same for y> + <same for z>

    Setting partial derivatives to zero and dividing both sides by 2:

    0 = a_x*b_x + u*b_x*b_x - c_x*b_x - v*d_x*b_x + <same for y> + <same for z>
    0 = -a_x*d_x - u*b_x*d_x + c_x*d_x + v*d_x*d_x + <same for y> + <same for z>

    Factoring out u and v:

    u*b_x*b_x - v*d_x*b_x + <same for y> + <same for z> = c_x*b_x - a_x*b_x + <same for y> + <same for z>
    v*d_x*d_x - u*b_x*d_x + <same for y> + <same for z> = a*x*d_x - c_x*d_x + <same for y> + <same for z>

    So we've got a linear system to solve:

    | dot(b, b)  -dot(b, d) | * | u | = | dot(c, b) - dot(a, b) |
    | -dot(b, d)  dot(d, d) |   | v |   | dot(a, d) - dot(c, d) |

    Note that the left-hand-side matrix is symmetric.
    */

    auto const lhs00 = dot(line1Vector, line1Vector);
    auto const lhs11 = dot(line2Vector, line2Vector);
    auto const lhsDiag = -dot(line1Vector, line2Vector);

    auto const rhs0 = dot(line2Origin, line1Vector) - dot(line1Origin, line1Vector);
    auto const rhs1 = dot(line1Origin, line2Vector) - dot(line2Origin, line2Vector);

    // Solving by Cramer's rule:
    auto const detLHS = lhs00 * lhs11 - lhsDiag * lhsDiag;
    auto const detU = rhs0 * lhs11 - rhs1 * lhsDiag;
    auto const detV = lhs00 * rhs1 - lhsDiag * rhs0;

    return std::make_pair(detU / detLHS, detV / detLHS);
}
