#include "CalculateDynamicPolePositions.h"

#include "neokon-shared/CalcBezierSplineCurveRadius.h"
#include "neokon-shared/FloatingPointCompare.h"
#include "neokon-shared/TransformTurnaroundZ.h"

namespace electrification
{
using namespace Unigine;

bool CustomPoleInfo::operator==(electrification::CustomPoleInfo const& other) const
{
    if (pathDistance != other.pathDistance)
    {
        return false;
    }

    if (pathNormalDistance != other.pathNormalDistance)
    {
        return false;
    }

    if (heightAbovePath != other.heightAbovePath)
    {
        return false;
    }

    if (presetName != other.presetName)
    {
        return false;
    }

    if (nodeName != other.nodeName)
    {
        return false;
    }

    if (isSuspension != other.isSuspension)
    {
        return false;
    }

    if (poleType != other.poleType)
    {
        return false;
    }

    if (poleSide != other.poleSide)
    {
        return false;
    }

    if (transitionalType != other.transitionalType)
    {
        return false;
    }

    if (consoleType != other.consoleType)
    {
        return false;
    }

    if (registrationArmType != other.registrationArmType)
    {
        return false;
    }

    return true;
}

bool CustomPoleInfo::operator!=(electrification::CustomPoleInfo const& other) const
{
    return !(operator==(other));
}

DynamicPoleGenInfo fillPoleGenInfo(CalculateDynamicPolePositionsArgs const& args, double pathDistance)
{
    DynamicPoleGenInfo poleGenInfo;
    poleGenInfo.curvatureSide = CurvatureSide::None;
    poleGenInfo.pathDistanceM = pathDistance;
    poleGenInfo.preset = args.defaultPolePreset;

    return poleGenInfo;
}

/**
 * Проверяет радиус кривизны участка сплайна между текущей опорой @pole и предыдущей опорой @previousPole.
 * Если радиус кривизны по модулю оказывается ниже установленного порога args.maxCurveRadiusThresholdM,
 * производится корректировка - вторая опора сдвигается к первой на расстояние args.curvedIntervalM.
 *
 * В структуру для параметра @pole записываются:
 * 1. радиус кривизны со знаком.
 * 2. направление закругления (внешний или внутренний радиус закругления).
 * 3. новое значение дистанции от начала пути @args.coveredPathSpline
 * 4. матрица трансформации
 *
 * Функция возвращает true, если позиция опоры @pole была скорректирована и false,
 * если корректировки не было. 
 */
bool checkAndUpdatePoleCurvature(CalculateDynamicPolePositionsArgs const& args,
                        DynamicPoleGenInfo const& previousPole, DynamicPoleGenInfo& pole, bool dontShift)
{
    BezierSpline const& coveredPathSpline = args.coveredPathSpline;
    const double curveMaxRadiusThreshold = args.maxCurveRadiusThresholdM;
    const double straightSecondPoleDistance = previousPole.pathDistanceM + args.straightIntervalM;
    const double curvedSecondPoleDistance = previousPole.pathDistanceM + args.curvedIntervalM;

    if (!CmDoubleCompare::equal(straightSecondPoleDistance, pole.pathDistanceM) && !pole.customId && !dontShift)
    {
        pole.pathDistanceM = straightSecondPoleDistance;
    }

    std::optional<double> curvature = calcBezierSplineCurveRadius(
        coveredPathSpline, previousPole.pathDistanceM, pole.pathDistanceM);
    if (!curvature)
    {
        return false;
    }

    pole.signedCurveRadiusM = *curvature;

    if (CmDoubleCompare::moreGreater(std::abs(*curvature), curveMaxRadiusThreshold))
    {
        return false;
    }

    pole.curvatureSide = (*curvature) > 0 ? CurvatureSide::Outer : CurvatureSide::Inner;

    if (pole.customId || dontShift)
    {
        return false;
    }

    if (DefaultDoubleCompare::greaterEqual(curvedSecondPoleDistance, pole.pathDistanceM))
    {
        return false;
    }

    pole.pathDistanceM = curvedSecondPoleDistance;

    if (curvature = calcBezierSplineCurveRadius(
        coveredPathSpline, previousPole.pathDistanceM, pole.pathDistanceM))
    {
        pole.signedCurveRadiusM = *curvature;
        pole.curvatureSide = (*curvature) > 0 ? CurvatureSide::Outer : CurvatureSide::Inner;
    }

    return true;
}

/**
 * Одинарный проход алгоритма автоматической расстановки опор. Корректироует позиции уже расставленных
 * опор на закругленных участках. Проходит по опорам, начиная со startIdx, и проверяет радиус кривизны,
 * при необходимости производя корректировки позиций опор на закругленных участках.
 * Нулевой элемент вектора poleGenInfo не затрагивается, но на нем будут основаны последующие расчеты кривизны.
 */
void checkPolesCurvatureOnePass(CalculateDynamicPolePositionsArgs const& args,
                                std::vector<DynamicPoleGenInfo>& poleGenInfo, int startIdx, bool dontShift)
{
    if (poleGenInfo.size() < 2)
    {
        //для сканирования и расчета кривизны нужны как минимум две опоры.
        return;
    }

    if (startIdx < 1)
    {
        startIdx = 1;
    }

    int lastIdx = poleGenInfo.size() - 1;
    int preLastIdx = lastIdx - 1;
    if (startIdx > preLastIdx && preLastIdx != 0)
    {
        startIdx = preLastIdx;
    }

    for (int i = startIdx; i <= lastIdx; i++)
    {
        DynamicPoleGenInfo& firstPole = poleGenInfo[i - 1];
        DynamicPoleGenInfo& secondPole = poleGenInfo[i];

        checkAndUpdatePoleCurvature(args, firstPole, secondPole, dontShift);
    }
}

/**
 * Одинарный проход (итерация) алгоритма автоматической расстановки опор. В рамках прохода,
 * алгоритм проходит по рабочему сплайну @args.coveredPathSpline,
 * начиная с дистанции от его начала @startDistance. При этом расставленные опоры добавляются
 * в вектор @poleGenInfo.
 *
 * Возвращает true, если итерация добавила новые опоры.
 */
bool calculateAutoPositionsOnePass(CalculateDynamicPolePositionsArgs const& args,
                                   std::vector<DynamicPoleGenInfo>& poleGenInfo, double startDistance)
{
    BezierSpline const& coveredPathSpline = args.coveredPathSpline;
    const double coveredPathLength = coveredPathSpline.arcLength();
    const double straightInterval = args.straightIntervalM;

    double currentPathDistance = startDistance;
    std::size_t prevSize = poleGenInfo.size();

    while (CmDoubleCompare::moreLess(currentPathDistance, coveredPathLength))
    {
        poleGenInfo.push_back(fillPoleGenInfo(args, currentPathDistance));

        currentPathDistance = poleGenInfo.back().pathDistanceM + straightInterval;

        if (CmDoubleCompare::greaterEqual(currentPathDistance, coveredPathLength))
        {
            double overhead = std::abs(currentPathDistance - coveredPathLength);

            if (CmDoubleCompare::lessEqual(overhead, args.maxAllowedIntervalCorrection))
            {
                poleGenInfo.push_back(fillPoleGenInfo(args, coveredPathLength));
            }
        }
    } // while (currentPathDistance < coveredPathLength)

    return poleGenInfo.size() > prevSize;
};

/**
 * Алгоритм автоматической расстановки опор на перегоне. Алгоритм выполняется в два этапа:
 * 1. Многопроходная расстановка опор с последующим сдвигом позиций опор на закругленных участках.
 * 2. Распределение оставшегося неиспользованного места на сплайне между пролетами с возможной
 *    вставкой дополнительного пролета.
 *
 * Возвращает вектор данных для генерации опор DynamicPoleGenInfo.
 */
std::vector<DynamicPoleGenInfo> calculateAutoPositions(CalculateDynamicPolePositionsArgs const& args)
{
    BezierSpline const& coveredPathSpline = args.coveredPathSpline;
    const double coveredPathLength = coveredPathSpline.arcLength();
    const double straightInterval = args.straightIntervalM;
    const double maxAllowedIntervalCorrection = args.maxAllowedIntervalCorrection;

    std::vector<DynamicPoleGenInfo> resultPoleGenInfo;

    int estimatedNumPoles = static_cast<int>(std::ceil(coveredPathLength / straightInterval)) + 1;
    estimatedNumPoles += static_cast<int>(args.customPoles.size());
    resultPoleGenInfo.reserve(estimatedNumPoles * 2);

    double startPathDistance = 0.0;
    int startCheckCurvatureIdx = 0;

    //после корректировки по кривизне участка, появляется неиспользованное место за
    //последней опорой и до конца пути. Мы делаем несколько проходов, чтобы заполнить
    //опорами весь охватываемый путь.
    double restDistance = coveredPathLength - startPathDistance;
    while (CmDoubleCompare::greaterEqual(restDistance, straightInterval))
    {
        if (!calculateAutoPositionsOnePass(args, resultPoleGenInfo, startPathDistance))
        {
            break;
        }

        checkPolesCurvatureOnePass(args, resultPoleGenInfo, startCheckCurvatureIdx, /*dontShift=*/false);

        startPathDistance = resultPoleGenInfo.back().pathDistanceM;
        restDistance = coveredPathLength - startPathDistance;
        startCheckCurvatureIdx = resultPoleGenInfo.size() - 1;
        startPathDistance += straightInterval;
    }

    return resultPoleGenInfo;
}

void distributeRestSpace(CalculateDynamicPolePositionsArgs const& args, std::vector<DynamicPoleGenInfo>& poleGenInfo)
{
    if (poleGenInfo.empty())
    {
        return;
    }

    BezierSpline const& coveredPathSpline = args.coveredPathSpline;
    const double coveredPathLength = coveredPathSpline.arcLength();
    const double straightInterval = args.straightIntervalM;
    const double maxAllowedIntervalCorrection = args.maxAllowedIntervalCorrection;

    double lastPoleDistance = poleGenInfo.back().pathDistanceM;
    double restDistance = coveredPathLength - lastPoleDistance;

    if (restDistance > 0 && CmDoubleCompare::lessEqual(restDistance, straightInterval))
    {
        //если после основных проходов все еще осталось место
        //надо распределить его между пролетами или вставить новый пролет

        int actualNumIntervals = poleGenInfo.size() - 1;
        double delta = straightInterval - restDistance;
        double allowedDelta = args.maxAllowedIntervalCorrection * actualNumIntervals;

        if (CmDoubleCompare::lessEqual(restDistance, allowedDelta))
        {
            //оставшийся участок в пределах допустимой поправки
            //сдвинуть опоры вперед на величину коррекции, чтобы заполнить оставшийся участок
            const double correction = restDistance / actualNumIntervals;
            double accumulatedCorrection = 0;
            for (auto& info : poleGenInfo)
            {
                info.pathDistanceM += accumulatedCorrection;
                if (info.pathDistanceM > coveredPathLength)
                {
                    info.pathDistanceM = coveredPathLength;
                }
                accumulatedCorrection += correction;
            }
        }
        else if (CmDoubleCompare::lessEqual(delta, allowedDelta))
        {
            //немного не хватает до еще одного пролета.
            //вставить последнюю опору и ужать предыдущие интервалы, чтобы высвободить
            //недостающее место
            poleGenInfo.push_back(fillPoleGenInfo(args, coveredPathLength));
            //если мы добавили еще одну опору, то мы должны также увеличить число интервалов.
            actualNumIntervals++;

            //поправка отрицательная, потому что сдвигаем опоры назад.
            const double correction = -(delta / actualNumIntervals);
            double accumulatedCorrection = 0;
            assert(!poleGenInfo.empty());
            for (int i = 0; i < poleGenInfo.size() - 1; i++)
            {
                auto& info = poleGenInfo[i];

                info.pathDistanceM += accumulatedCorrection;
                if (CmDoubleCompare::moreGreater(info.pathDistanceM, coveredPathLength))
                {
                    info.pathDistanceM = coveredPathLength;
                }
                accumulatedCorrection += correction;
            }

            //расчитаем кривизну для последней опоры
            auto lastPoleIdx = poleGenInfo.size() - 1;
            DynamicPoleGenInfo& firstPole = poleGenInfo[lastPoleIdx - 1];
            DynamicPoleGenInfo& secondPole = poleGenInfo[lastPoleIdx];
            checkAndUpdatePoleCurvature(args, firstPole, secondPole, true);

            secondPole.transform = (*args.coveredPathSpline.endDistanceMatrix());
        }
    }
}

/**
 * Вставка кастомных опор (опор для которых позицию и настройки определил пользователь вручную).
 *
 * Записи для кастомных пор добавляються в вектор @poleGenInfo, при этом некоторые автоматически
 * сгенерированные опоры, попавшие в радиус отчуждения отбрасываются. По окончанию вставки, производится
 * сортировка вектора по дистанции от начала пути DynamicPoleGenInfo::pathDistance.
 */
void addCustomPoles(CalculateDynamicPolePositionsArgs const& args, std::vector<DynamicPoleGenInfo>& poleGenInfo)
{
    const double exclusionDistance = args.straightIntervalM * 0.5;

    boost::icl::split_interval_set<double> exclusionIntervals;

    auto poleExcluded = [&exclusionIntervals](DynamicPoleGenInfo& info)
    {
        auto poleFoundIt = exclusionIntervals.find(info.pathDistanceM);
        return poleFoundIt != exclusionIntervals.end();
    };

    // adding custom poles
    for (auto const& info : args.customPoles)
    {
        decltype(exclusionIntervals)::interval_type interval
        {
            info.pathDistance - exclusionDistance,
            info.pathDistance + exclusionDistance
        };
        exclusionIntervals += interval;
    }

    auto newEndIt = std::remove_if(poleGenInfo.begin(), poleGenInfo.end(), poleExcluded);
    poleGenInfo.erase(newEndIt, poleGenInfo.end());

    for (auto const& info : args.customPoles)
    {
        auto poleTransform =
            args.coveredPathSpline.calcSplineMatrixAtDistance(info.pathDistance, true);
        if (!poleTransform)
        {
            continue;
        }

        Math::Mat4 basePoleTransform = *poleTransform;
        PoleSide poleSide = args.poleSide;
        if (info.poleSide)
        {
            poleSide = *info.poleSide;
        }

        if (poleSide == PoleSide::LeftHand)
        {
            basePoleTransform = transformTurnaroundZ(basePoleTransform);
        }

        electrification::DynamicPoleGenInfo customPoleInfo;
        customPoleInfo.customId = info.customId;
        customPoleInfo.curvatureSide = CurvatureSide::None;
        customPoleInfo.pathDistanceM = info.pathDistance;
        customPoleInfo.preset = info.presetName;
        customPoleInfo.nodeName = info.nodeName;
        customPoleInfo.isSuspension = info.isSuspension;
        customPoleInfo.poleType = info.poleType;
        customPoleInfo.transitionalPoleSubtype = info.transitionalType;
        customPoleInfo.consoleType = info.consoleType;
        customPoleInfo.registrationArmType = info.registrationArmType;
        customPoleInfo.flipRegistrationArmType = info.flipRegistrationArmType;

        Math::Vec3 localPosition{0.0, -info.pathNormalDistance, info.heightAbovePath};
        Math::Vec3 newPosition = basePoleTransform * localPosition;

        Math::Mat4 rotate = Math::rotateZ(info.angle);
        basePoleTransform = rotate * basePoleTransform;

        basePoleTransform.setColumn(3, Math::Vec4(newPosition, 0.0));

        customPoleInfo.transform = basePoleTransform;

        if (info.isSuspension && info.portalDirection)
        {
            Math::Vec3 portalAxisY = *info.portalDirection;
            Math::Vec3 suspensionAxisY = basePoleTransform.getAxisY();
            portalAxisY.normalize();
            suspensionAxisY.normalize();
            double dot = Math::dot(portalAxisY, suspensionAxisY);

            Math::Vec3 axisY =
                DefaultDoubleCompare::greaterEqual(dot, 0.0) ? portalAxisY : -portalAxisY;
            Math::Vec3 axisZ = Math::Vec3::UP;
            Math::Vec3 axisX = Math::cross(axisY, axisZ);

            customPoleInfo.transform.setColumn(0, Math::Vec4(axisX, 0.0));
            customPoleInfo.transform.setColumn(1, Math::Vec4(axisY, 0.0));
            customPoleInfo.transform.setColumn(2, Math::Vec4(axisZ, 0.0));
        }

        poleGenInfo.push_back(customPoleInfo);
    }

    // sorting by path distance
    auto compare = [](electrification::DynamicPoleGenInfo const& A,
        electrification::DynamicPoleGenInfo const& B)
    {
        return A.pathDistanceM < B.pathDistanceM;
    };

    std::sort(poleGenInfo.begin(), poleGenInfo.end(), compare);
}

void
flipRegistrationArmsUnsafe(DynamicPoleGenInfo& info)
{
    switch (info.registrationArmType)
    {
    case RegistrationArmType::StraightPullOff:
        info.registrationArmType = RegistrationArmType::StraightPushOff;
        break;
    case RegistrationArmType::StraightPushOff:
        info.registrationArmType = RegistrationArmType::StraightPullOff;
        break;
    case RegistrationArmType::CurvePullOff:
        info.registrationArmType = RegistrationArmType::CurvePushOff;
        break;
    case RegistrationArmType::CurvePushOff:
        info.registrationArmType = RegistrationArmType::CurvePullOff;
        break;
    }
}

/**
 * Вычисляет типы фиксаторов на основе радиуса и направления кривизны.
 * Если для опоры установлена ориентация по правилу левой руки (@poleSide == PoleSide::LeftHand),
 * разворацияет опору на 180 градусов относительно пивота вокруг  оси Z. Таким образом опора оказывается
 * с противоположной стороны путей.
 */
void calculateRegistrationArmType(std::vector<DynamicPoleGenInfo>& poleGenInfo, PoleSide poleSide)
{
    bool pullOff = true;
    RegistrationArmType prevArmType = RegistrationArmType::StraightPullOff;

    for (auto& info : poleGenInfo)
    {
        if (info.poleType == PoleType::Transitional
            && ((*info.transitionalPoleSubtype) == TransitionalPoleType::TransitionalC
                || (*info.transitionalPoleSubtype) == TransitionalPoleType::TransitionalCInsulated))
        {
            info.registrationArmType = prevArmType;
        }
        else if (info.curvatureSide == CurvatureSide::Inner)
        {
            info.registrationArmType = poleSide == PoleSide::LeftHand
                ? RegistrationArmType::CurvePushOff : RegistrationArmType::CurvePullOff;
            pullOff = true;
        }
        else if (info.curvatureSide == CurvatureSide::Outer)
        {
            info.registrationArmType = poleSide == PoleSide::LeftHand ?
                RegistrationArmType::CurvePullOff : RegistrationArmType::CurvePushOff;
            pullOff = false;
        }
        else
        {
            info.registrationArmType = pullOff
                ? RegistrationArmType::StraightPullOff
                : RegistrationArmType::StraightPushOff;
            pullOff = !pullOff;
        }

        prevArmType = info.registrationArmType;

        if (info.customId && info.flipRegistrationArmType)
        {
            flipRegistrationArmsUnsafe(info);
        }
    }//for (auto& info : poleGenInfo)

    //check first pole
    if (poleGenInfo.size() < 2)
    {
        return;
    }

    DynamicPoleGenInfo const& secondPole = poleGenInfo[1];
    DynamicPoleGenInfo& firstPole = poleGenInfo[0];
    if (secondPole.curvatureSide != CurvatureSide::None)
    {
        firstPole.registrationArmType = secondPole.registrationArmType;
    }
}

void flipRegistrationArms(std::vector<DynamicPoleGenInfo>& poleGenInfo)
{
    for (auto& info : poleGenInfo)
    {
        if (info.registrationArmType == RegistrationArmType::StraightPullOff)
        {
            info.registrationArmType = RegistrationArmType::StraightPushOff;
        }
        else if (info.registrationArmType == RegistrationArmType::StraightPushOff)
        {
            info.registrationArmType = RegistrationArmType::StraightPullOff;
        }
    }
}

void calculatePoleTypes(CalculateDynamicPolePositionsArgs const& args, std::vector<DynamicPoleGenInfo>& poleGenInfo)
{
    const int commonNumPoles = args.numTensionLengthIntervals + args.numTensionOverlapIntervals;
    const int overlapAPoleIndex = args.numTensionLengthIntervals + 1;
    const int overlapBPoleIndex = args.numTensionLengthIntervals + args.numTensionOverlapIntervals - 1;

    for (int i = 0; i < poleGenInfo.size(); i++)
    {
        if (args.numTensionOverlapIntervals <= 0)
        {
            poleGenInfo[i].poleType = PoleType::Middle;
            continue;
        }

        int localPoleIdx = i % commonNumPoles;

        if (localPoleIdx > overlapAPoleIndex && localPoleIdx < overlapBPoleIndex)
        {
            poleGenInfo[i].poleType = PoleType::Transitional;
            poleGenInfo[i].transitionalPoleSubtype = TransitionalPoleType::TransitionalC;
        }
        else if (localPoleIdx == overlapAPoleIndex)
        {
            poleGenInfo[i].poleType = PoleType::Transitional;
            poleGenInfo[i].transitionalPoleSubtype = TransitionalPoleType::TransitionalA;
        }
        else if (localPoleIdx == overlapBPoleIndex)
        {
            poleGenInfo[i].poleType = PoleType::Transitional;
            poleGenInfo[i].transitionalPoleSubtype = TransitionalPoleType::TransitionalB;
        }
        else
        {
            poleGenInfo[i].poleType = PoleType::Middle;
        }
    }//for (int i = 0; i < poleGenInfo.size(); i++)
}

void calculatePoleTransforms(CalculateDynamicPolePositionsArgs const& args,
    std::vector<DynamicPoleGenInfo>& poleGenInfo)
{
    BezierSpline const& coveredPathSpline = args.coveredPathSpline;

    for (auto& info : poleGenInfo)
    {
        if (info.customId)
        {
            continue;
        }

        auto const transform = coveredPathSpline.calcSplineMatrixAtDistance(info.pathDistanceM,
            /*clampDistance=*/true);
        if (transform)
        {
            info.transform = (*transform);
        }

        if (args.poleSide == PoleSide::LeftHand)
        {
            info.transform = transformTurnaroundZ(info.transform);
        }
    }//for (auto& info : poleGenInfo)
}

std::vector<DynamicPoleGenInfo>
calculateDynamicPolePositions(CalculateDynamicPolePositionsArgs const& args)
{
    auto resultPoleGenInfo = calculateAutoPositions(args);

    addCustomPoles(args, resultPoleGenInfo);
    checkPolesCurvatureOnePass(args, resultPoleGenInfo, 1, false);
    distributeRestSpace(args, resultPoleGenInfo);
    calculatePoleTransforms(args, resultPoleGenInfo);
    calculatePoleTypes(args, resultPoleGenInfo);
    calculateRegistrationArmType(resultPoleGenInfo, args.poleSide);

    if (args.flipRegistrationArms)
    {
        flipRegistrationArms(resultPoleGenInfo);
    }

    return resultPoleGenInfo;
}

std::vector<DynamicPoleGenInfo>
calculateDynamicPolePositionsAligned(CalculateDynamicPolePositionsArgs const& args,
                                            std::vector<DynamicPoleGenInfo> const& sourceData,
                                            std::vector<double> const& sourcePositions)
{
    std::vector<DynamicPoleGenInfo> resultPoleGenInfo;
    resultPoleGenInfo.reserve(sourcePositions.size());

    assert(sourceData.size() == sourcePositions.size());
    for (int i = 0; i < sourcePositions.size() && i < sourceData.size(); i++)
    {
        resultPoleGenInfo.push_back(fillPoleGenInfo(args, sourcePositions[i]));
        resultPoleGenInfo.back().poleType = sourceData[i].poleType;
        resultPoleGenInfo.back().transitionalPoleSubtype = sourceData[i].transitionalPoleSubtype;
    }

    addCustomPoles(args, resultPoleGenInfo);
    checkPolesCurvatureOnePass(args, resultPoleGenInfo, 1, true);
    calculatePoleTransforms(args, resultPoleGenInfo);
    calculateRegistrationArmType(resultPoleGenInfo, args.poleSide);

    if (args.flipRegistrationArms)
    {
        flipRegistrationArms(resultPoleGenInfo);
    }

    return resultPoleGenInfo;
}

void
calculateDynamicPolePositionsCached(CalculateDynamicPolePositionsArgs const& args,
    std::vector<DynamicPoleGenInfo>& cache)
{
    for (auto& cachedGenInfo : cache)
    {
        cachedGenInfo.preset = args.defaultPolePreset;
    }

    addCustomPoles(args, cache);
    calculatePoleTransforms(args, cache);
    calculatePoleTypes(args, cache);
}

} // namespace electrification

