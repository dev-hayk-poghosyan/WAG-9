#pragma once

#include <string>
#include <memory>
#include <list>
#include <UnigineMesh.h>
#include <UnigineMath2d.h>

#include "neokon-shared/BezierSegment.h"

/**
 * Функция generateMeshFromSpline создает меш на основе набора направляющих сплайнов и контура профиля.
 * Результат схож с работой модификатора Bevel Profile в пакете моделирования 3DS max.
 */
struct ProfileScaleKey
{
    double normalizedPathOffset{ 0.0 }; //[0: 1]
    Unigine::Math::Vec2 profileAxisScaleKey; //y and z
};


/**
 * Входные параметры функции generateMeshFromSpline:
 * 
 * @param dstMesh - выходной меш в который помещаеться результат. Меш не перезаписываеться полностью,
 *                  функция добавляет к нему от одной до нескольких поверхностей с именами 
 *                  surfaceName_[idx], по одной для каждого сплайнового сегмента.
 * @param surfaceName - имя добавляемой поверхности (или поверхностей).
 * @param splineList - направляющий путь из сплайнов, на который "натягивается" профиль из 
 *                     точек profilePointPositions.
 * @param profilePointPositions - замкнутый контур из точек, образующих некий поперечный срез 
 *                                создаваемого меша.
 * @param profilePointVCoords - текстурные V-компоненты текстурных координат точек профиля. 
 *                              Количество точек profilePointVCoords должно равняться таковому 
 *                              в profilePointPositions. U-компоненты координат для текстур 
 *                              генерируються в процессе построения.
 * @param numSegmentsPerUnit - количество сегментов на отрезок пути длиной 1 целую единицу.
 * @param uCoordStepPerUnit - приращение компоненты U текстурных коодина на 1 единицу пути.
 * @param localizeSplines - нужен перевод в локальные координаты опорных точек сплайна.
 *                          Если вызывающая программа позаботилась о локальных координатах сама,
 *                          то этот параметр должен быть false.
 * @param minOffset - начальное смещение для рассчета текстурной V координаты
 * @param profileOffsetY - боковое смещение относительно сплайна
 * @param profileMirrorY - зеркальное отражение точек профиля относительно сплайна
 * @param cropLengthBegin - обрезка сплайна по длинне, начало активного участка
 * @param cropLengthEnd - обрезка сплайна по длинне, конец активного участка
 * @param preCap - заглушка, закрываюшая профиль в начале
 * @param postCap - заглушка, закрываюшая профиль в конце
 * 
 * @param profileCapTris - индексы вершин для треугольников заглушки. При отсутстывии генерируются 
 *                         в функции 'generateMeshFromSpline', в структуру занесены для повторного 
 *                         использования. 
 */
struct GenerateMeshFromSplineArgs
{
    Unigine::MeshPtr dstMesh;
    int surfaceIndex = -1; // if != -1 use this
    std::string surfaceName;
    std::vector<BezierSegment> splineList;
    std::vector<Unigine::Math::Vec3> profilePointPositions;
    std::vector<Unigine::Math::vec3> profilePointNormals;
    std::vector<Unigine::Math::vec2> profilePointUVCoords;
    std::vector<ProfileScaleKey> profileScaleKeys;
    double numSegmentsPerUnit{ 0.0 };
    double uCoordStepPerUnit{ 0.0 };
    double lengthOffsetForUV{ 0.0 };
    Unigine::Math::Vec3 profileOffset;
    bool profileMirrorY{ false };
    double cropLengthBegin{ 0.0 };
    double cropLengthEnd{ 0.0 };
    bool adaptiveDividing{ false };

    bool preCap{ false };
    bool postCap{ false };
    std::vector<std::array<int, 3>> profileCapTris;
};

struct GenerateMeshFromSplineOutput
{
    Unigine::Math::Mat4 meshWorldTransform;
};

GenerateMeshFromSplineOutput 
generateMeshFromSpline(const GenerateMeshFromSplineArgs& args);
