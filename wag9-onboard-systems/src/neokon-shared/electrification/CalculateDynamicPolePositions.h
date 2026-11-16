#pragma once

#include "neokon-shared/BezierSpline.h"
#include "neokon-shared/electrification/ElectrificationContextDataTypes.h"
#include "neokon-shared/electrification/ElectrificationGeneratorDataTypes.h"

#include <UnigineMathLib.h>
#include <optional>

namespace electrification
{

struct CustomPoleInfo
{
    int customId{ -1 };
    double pathDistance{ 0.0 };
    double pathNormalDistance{ 0.0 };
    double heightAbovePath{ 0.0 };
    double angle{ 0.0 };
    std::string presetName;
    std::optional<std::string> nodeName;
    std::optional<electrification::PoleSide> poleSide;

    electrification::PoleType poleType{ electrification::PoleType::Middle };
    std::optional<electrification::TransitionalPoleType> transitionalType;
    electrification::ConsoleType consoleType{ electrification::ConsoleType::Horizontal };
    electrification::RegistrationArmType registrationArmType{ electrification::RegistrationArmType::StraightPullOff };
    std::optional<Unigine::Math::Vec3> portalDirection;
    bool isSuspension{ false };
    bool flipRegistrationArmType{ false };

    bool operator==(electrification::CustomPoleInfo const& other) const;
    bool operator!=(electrification::CustomPoleInfo const& other) const;
};
using CustomPoleInfoList = std::list<CustomPoleInfo>;

/**
 * Входящие аргументы для функции calculateDynamicPolePositions и её производных.
 */
struct CalculateDynamicPolePositionsArgs
{
    BezierSpline coveredPathSpline; //Рабочий сплайн вдоль которого рассчитываються позиции жд-опор
    PoleSide poleSide{PoleSide::LeftHand}; //Сторона установки жд-опор справа или слева от полотна
                                           //по правилу левой руки
    std::string defaultPolePreset;
    int numTensionLengthIntervals{0}; //количество пролетов анкерного участка
    int numTensionOverlapIntervals{0}; //количество пролетов сопряжения
    double straightIntervalM{0}; //Длина пролета на прямых участках пути.
    double curvedIntervalM{0}; //Длина пролета на закругленных участках.
    double maxCurveRadiusThresholdM{0}; //Максимальный радиус кривизны,
                                        //выше которого участок считаеться прямым
    double maxAllowedIntervalCorrection{0}; //Допустимая величина подгонки длины интервалов
                                            //при равномерном распределении длины участка КС
                                            //Если после корректировки интервалов от частка остался
                                            //хвост меньше длины пролета - вставляется новый пролет.
    bool isOverlap{ false }; //сопряжение или анкерный участок?
    bool flipRegistrationArms{ false };
    CustomPoleInfoList customPoles; //список пользовательских опор,
                                    //которые генератор должен добавить наравне
                                    //со сгенерированными в итоговый результат
};

/**
 * Расчитывает позиции и другую входную информацию для генераторов анкерных участков,
 * сопряжений и подвесок. Данная реализации используется при создании нового
 * электрифицированного пути. Возвращает список структур DynamicPoleGenInfo,
 * каждая из которых нужна для генерации одной опоры (или подвеса под поперечиной) на жд-путях.
 */
std::vector<DynamicPoleGenInfo>
calculateDynamicPolePositions(CalculateDynamicPolePositionsArgs const& args);

/**
 * Вариант функции расчета входных данных для генераторов, для создания
 * электрифицированных путей, чьи опоры, выровненны относительно их визави на другом пути.
 * Помимо входной описательной структуры данных принимает также исходные данные
 * пути-референса sourceData, и список  расчитанных выровненных позиций sourcePositions.
 * В функции делается предположение, что sourceData.size() == sourcePositions.size().
 * В итоговый результат копируются выровненные позиции sourcePositions и типы опор из
 * sourceData. Оставшаяся информация (кастомные опоры, типы фиксаторов, матрицы трансформации
 * заполняется на основе параметров структуры args.
 */
std::vector<DynamicPoleGenInfo>
calculateDynamicPolePositionsAligned(CalculateDynamicPolePositionsArgs const& args,
                                     std::vector<DynamicPoleGenInfo> const& sourceData,
                                     std::vector<double> const& sourcePositions);

/**
 * Данный вариант расчетной функции предназначен для работы с кешированными данными,
 * загружаемыми из electrificationSet.json, при последующих загрузках мира с уже
 * построенной электрификацией. Данный вариант только добавляет кастомные опоры и вычисляет
 * матрицы трансформации.
 */
void
calculateDynamicPolePositionsCached(CalculateDynamicPolePositionsArgs const& args,
    std::vector<DynamicPoleGenInfo>& resultPoleGenInfo);

} // namespace electrification
