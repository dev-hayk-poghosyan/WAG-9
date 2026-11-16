#pragma once

#include "BezierSpline.h"

/**
 * Вычисляет радиус кривизы участка сплайна от @distanceFrom до @distanceTo.
 * В случае успеха возвращает величину со знаком (signed). Модуль результата -
 * радиус кривизны в метрах, знак показывает отклонение пути при движении от
 * от @distanceFrom до @distanceTo. Положительный знак означает отклонение вправо,
 * отрицательный влево.
 *
 * Функция возвращает std::nullopt, если параметры distanceFrom и distanceTo
 * выходят за пределы длины сплайна, и max(double), в случае идеально прямого пути.
 */
std::optional<double>
calcBezierSplineCurveRadius(BezierSpline const& spline, double distanceFrom, double distanceTo);
