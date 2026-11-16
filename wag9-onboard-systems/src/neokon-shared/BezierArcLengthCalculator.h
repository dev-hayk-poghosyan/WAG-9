#pragma once

#include <glm/glm.hpp>

class BezierSegment;

class BezierArcLengthCalculator
{
public:
    BezierArcLengthCalculator(BezierSegment const& segment);

    double arcLengthFromTo(double tStart, double tEnd) const;

    double tForDistance(double distance) const;

private:
    struct Sample
    {
        double weight;
        double xMinusOneToOne;
    };

    /**
     * Represents the value f(x) and the derivative f'(x) of some unknown function, evaluated at x.
     */
    struct alignas(16) EvaluatedFunction
    {
        /**
         * f(x)
         */
        double val;

        /**
         * f'(x)
         */
        double deriv;

        EvaluatedFunction(double v = 0.0, double d = 0.0) : val(v), deriv(d) {}

        EvaluatedFunction& operator+=(EvaluatedFunction f);

        EvaluatedFunction& operator-=(EvaluatedFunction f);

        EvaluatedFunction& operator*=(double scalar);

        EvaluatedFunction& operator*=(EvaluatedFunction f);

        friend EvaluatedFunction operator+(EvaluatedFunction f1, EvaluatedFunction f2)
        {
            return f1 += f2;
        }

        friend EvaluatedFunction operator-(EvaluatedFunction f1, EvaluatedFunction f2)
        {
            return f1 -= f2;
        }

        friend EvaluatedFunction operator*(EvaluatedFunction f, double scalar)
        {
            return f *= scalar;
        }

        friend EvaluatedFunction operator*(double scalar, EvaluatedFunction f)
        {
            return f *= scalar;
        }

        friend EvaluatedFunction operator*(EvaluatedFunction f1, EvaluatedFunction f2)
        {
            return f1 *= f2;
        }
    };

    EvaluatedFunction arcLengthFromZero(double tEnd) const;

    glm::dvec3 splineGradientVector(double t) const;

    glm::tvec3<EvaluatedFunction> splineGradientVector(EvaluatedFunction t) const;

    template<typename T>
    glm::tvec3<T> controlPointWeightsForSplineGradient(T t) const;

    static EvaluatedFunction vectorLength(glm::tvec3<EvaluatedFunction> const& vec);

    static EvaluatedFunction vectorSquaredLength(glm::tvec3<EvaluatedFunction> const& vec);

    static EvaluatedFunction square(EvaluatedFunction f);

    static EvaluatedFunction sqrt(EvaluatedFunction f);

    static Sample const m_samples[13];

    /**
     * The columns of this matrix are the last 3 control points of a Bezier segment.
     * The first control point that's not included in this matrix is at the origin.
     */
    glm::dmat3 m_last3ControlPoints;
};
