#pragma once

#include <string>

/**
 * Contains static functions to check node name with different patterns.
 */
class UkSemaphoreNodePatterns
{
public:
    /**
     * Checks node if it's shunt spindle head.
     */
    static bool shuntHeadNode(const std::string& nodeName)
    {
        return contains(nodeName, "_head_");
    }

    /**
     * Checks node if it's an arm.
     */
    static bool armNode(const std::string& nodeName)
    {
        return contains(nodeName, "_sem_arm_");
    }

    /**
     * Checks node if it's a gear.
     */
    static const std::string gearPattern() { return "_gear"; }

    static bool gearNode(const std::string& nodeName)
    {
        return contains(nodeName, gearPattern());
    }

    /**
     * Checks if top node has arm sub nodes or node is gantry arm.
     */
    static bool gantryNode(const std::string& nodeName)
    {
        return contains(nodeName, "_sem_gantry");
    }

    /**
     * Checks top node if it's an single arm as well.
     */
    static bool singleArmNode(const std::string& nodeName)
    {
        return contains(nodeName, "_sem_pole");
    }

    /**
     * Checks node if it's a rising arm.
     */
    static bool risingArm(const std::string& nodeName)
    {
        return contains(nodeName, "_sem_arm_rising_");
    }

    /**
     * Checks node if it's a small spindle.
     */
    static bool smallSpindle(const std::string& nodeName)
    {
        return contains(nodeName, "_small");
    }

    /**
     * Checks node if it's a shunt spindle.
     */
    static bool shuntSpindle(const std::string& nodeName)
    {
        return contains(nodeName, "_shunt");
    }

    /**
     * Checks node if it's a yellow spindle.
     */
    static bool yellowSpindle(const std::string& nodeName)
    {
        return contains(nodeName, "_yellow");
    }

private:
    static bool contains(const std::string& str, const std::string& pattern)
    {
        return str.find(pattern) != std::string::npos;
    }
};
