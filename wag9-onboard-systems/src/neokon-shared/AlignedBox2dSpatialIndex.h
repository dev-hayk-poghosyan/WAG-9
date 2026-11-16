#pragma once

#include "AlignedBox.h"

#include <optional>
#include <vector>

/**
 * Represents the spatial index which uses aligned boxes.
 * The template type Box2D must provide method box() which returns the AlignedBox2D.
 * Spatial index will be a simple table of cells, and the box Box2D will be put into the cells
 * depending on its min and max
 * coordinates. The query function will take a spatial point and return vector of boxes
 * which contain it.
 */
template<typename Box2D>
class AlignedBox2dSpatialIndex
{
public:
    AlignedBox2dSpatialIndex(std::vector<Box2D> const& boxes, double cellSize)
        : m_boxes(boxes)
        , m_worldBox(glm::dvec2(0.0), glm::dvec2(0.0))
        , m_cellSize(cellSize)
    {
        assert(m_cellSize > 0.0);

        // At first, determine the world box size. The world contains all boxes.
        if (m_boxes.empty())
        {
            return;
        }

        m_worldBox = m_boxes.front().box();

        size_t const totalBoxes = boxes.size();
        for (size_t boxIdx = 1; boxIdx < totalBoxes; ++boxIdx)
        {
            m_worldBox |= boxes[boxIdx].box();
        }

        m_cellColumnCount = static_cast<size_t>(ceil(m_worldBox.maxCoords().x -
            m_worldBox.minCoords().x + m_cellSize - 1.0)) / m_cellSize;
        m_cellRowCount = static_cast<size_t>(ceil(m_worldBox.maxCoords().y -
            m_worldBox.minCoords().y + m_cellSize - 1.0)) / m_cellSize;

        // Fill cells with box indices.

        m_cells.resize(m_cellColumnCount * m_cellRowCount);

        for (size_t boxIdx = 0; boxIdx < totalBoxes; ++boxIdx)
        {
            AlignedBox2D const& alignedBox = m_boxes[boxIdx].box();

            auto const minAddress = addressOfPoint(alignedBox.minCoords());
            auto const maxAddress = addressOfPoint(alignedBox.maxCoords());

            assert(minAddress && maxAddress); // by definition of m_worldBox.

            size_t const minX = (alignedBox.minCoords().x - m_worldBox.minCoords().x) / cellSize;
            size_t const minY = (alignedBox.minCoords().y - m_worldBox.minCoords().y) / cellSize;
            size_t const maxX = std::min(static_cast<size_t>((alignedBox.maxCoords().x -
                m_worldBox.minCoords().x) / cellSize), m_cellColumnCount - 1);
            size_t const maxY = std::min(static_cast<size_t>((alignedBox.maxCoords().y -
                m_worldBox.minCoords().y) / cellSize), m_cellRowCount - 1);

            for (size_t y = minY; y <= maxY; ++y)
            {
                for (size_t x = minX; x <= maxX; ++x)
                {
                    size_t const cellIdx = y * m_cellColumnCount + x;

                    m_cells[cellIdx].push_back(boxIdx);
                }
            }
        }
    }

    std::vector<Box2D> queryPoint(glm::dvec2 const& point) const
    {
        std::vector<Box2D> result;

        if (auto address = addressOfPoint(point))
        {
            const auto& boxIndices = m_cells[*address];

            for (const auto& cellBoxIdx : boxIndices)
            {
                const auto& box = m_boxes[cellBoxIdx];
                if (box.box().containsPoint(point))
                {
                    result.push_back(box);
                }
            }
        }

        return result;
    }

    // TODO: also implement query for circular area with no accurate test of each box in the cell.
    std::vector<Box2D> queryRectArea(glm::dvec2 const& minPoint, glm::dvec2 const& maxPoint) const
    {
        long const minCellX =
            static_cast<long>(floor(minPoint.x - m_worldBox.minCoords().x)) / m_cellSize;
        long const minCellY =
            static_cast<long>(floor(minPoint.y - m_worldBox.minCoords().y)) / m_cellSize;
        long const maxCellX =
            static_cast<long>(floor(maxPoint.x - m_worldBox.minCoords().x)) / m_cellSize;
        long const maxCellY =
            static_cast<long>(floor(maxPoint.y - m_worldBox.minCoords().y)) / m_cellSize;

        long const startCellX = std::max(0l, minCellX);
        long const startCellY = std::max(0l, minCellY);
        long const endCellX = std::min(static_cast<long>(m_cellColumnCount) - 1, maxCellX);
        long const endCellY = std::min(static_cast<long>(m_cellRowCount) - 1, maxCellY);

        std::unordered_set<size_t> queryBoxIndices; // Use set to remove repeated box indices.

        for (long cellY = startCellY; cellY <= endCellY; ++cellY)
        {
            for (long cellX = startCellX; cellX <= endCellX; ++cellX)
            {
                assert(cellY >= 0 && cellX >= 0);
                size_t const address = static_cast<size_t>(cellY) * m_cellColumnCount +
                    static_cast<size_t>(cellX);
                const auto& boxIndices = m_cells[address];

                if (boxIndices.empty())
                {
                    continue;
                }

                if (cellY > startCellY && cellY < endCellY &&
                    cellX > startCellX && cellX < endCellX)
                {
                    queryBoxIndices.insert(boxIndices.begin(), boxIndices.end());
                }
                else
                {
                    // We are on the edge cells. Construct the new box and test the cell boxes
                    // for intersection with it.
                    glm::vec2 partialBoxMin, partialBoxMax;

                    if (cellX == startCellX)
                    {
                        partialBoxMin.x = minPoint.x;
                    }
                    else
                    {
                        partialBoxMin.x = m_worldBox.minCoords().x + cellX * m_cellSize;
                    }

                    if (cellY == startCellY)
                    {
                        partialBoxMin.y = minPoint.y;
                    }
                    else
                    {
                        partialBoxMin.y = m_worldBox.minCoords().y + cellY * m_cellSize;
                    }

                    if (cellX == endCellX)
                    {
                        partialBoxMax.x = maxPoint.x;
                    }
                    else
                    {
                        partialBoxMax.x = m_worldBox.minCoords().x + (cellX + 1) * m_cellSize;
                    }

                    if (cellY == endCellY)
                    {
                        partialBoxMax.y = maxPoint.y;
                    }
                    else
                    {
                        partialBoxMax.y = m_worldBox.minCoords().y + (cellY + 1) * m_cellSize;
                    }

                    AlignedBox2D const partialCellBox(partialBoxMin, partialBoxMax);

                    for (const auto& cellBoxIdx : boxIndices)
                    {
                        if (queryBoxIndices.find(cellBoxIdx) != queryBoxIndices.end())
                        {
                            continue;
                        }

                        const auto& box = m_boxes[cellBoxIdx];

                        if (partialCellBox.intersectsWith(box.box()))
                        {
                            queryBoxIndices.insert(cellBoxIdx);
                        }
                    }
                }
            }
        }

        std::vector<Box2D> result;

        for (size_t boxIdx : queryBoxIndices)
        {
            result.push_back(m_boxes[boxIdx]);
        }

        return result;
    }

private:
    std::optional<size_t> addressOfPoint(glm::dvec2 const& point) const
    {
        if (!m_worldBox.containsPoint(point))
        {
            return std::nullopt;
        }
        size_t const x = static_cast<size_t>(floor(point.x - m_worldBox.minCoords().x)) / m_cellSize;
        size_t const y = static_cast<size_t>(floor(point.y - m_worldBox.minCoords().y)) / m_cellSize;

        return y * m_cellColumnCount + x;
    }

    /**
     * Each cell contains an array of indices of boxes in @p m_boxes vector.
     */
    using Cell = std::vector<size_t>;

    std::vector<Box2D> m_boxes;

    std::vector<Cell> m_cells;

    AlignedBox2D m_worldBox;

    size_t m_cellColumnCount = 0;
    size_t m_cellRowCount = 0;

    double const m_cellSize;
};
