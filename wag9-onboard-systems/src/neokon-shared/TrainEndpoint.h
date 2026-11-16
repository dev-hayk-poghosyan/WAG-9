#pragma once

enum class TrainEndpoint
{
    /**
     * The head of the train is normally where the driver is located,
     * however that may not be the case when shunting work is involved.
     * In general, the "Head" and "Tail" terms are meant to be relative
     * to the driver, that is, the "Head" is where the driver looks and
     * where the train will move when the driver activates forward motion.
     */
    Head,

    /**
     * @see Head
     */
    Tail
};
