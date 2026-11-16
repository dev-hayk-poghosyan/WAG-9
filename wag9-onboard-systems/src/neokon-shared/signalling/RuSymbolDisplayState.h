#pragma once

#include "neokon-shared/PlusEnum.h"

/**
 * All possible values of Russian Metro Signals Tables.
 *
 * Note: on any changes shuld fix methods in
 * RuSymbolDisplayCalculator class
 */
enum class RuSymbolDisplayState
{
    Error = -1,
    Off = 0, 

    Digit0 = 1,
    Digit1 = 2,
    Digit2 = 3,
    Digit3 = 4,
    Digit4 = 5,
    Digit5 = 6,
    Digit6 = 7,
    Digit7 = 8,
    Digit8 = 9,
    Digit9 = 10,

    LetterA = 11,   // А
    LetterB = 12,   // Б
    LetterV = 13,   // В
    LetterG = 14,   // Г
    LetterD = 15,   // Д
    LetterYe = 16,  // Е
    LetterZh = 17,  // Ж
    LetterZ = 18,   // З
    LetterI = 19,   // И
    LetterK = 20,   // К
    LetterL = 21,   // Л
    LetterM = 22,   // М
    LetterN = 23,   // Н
    LetterO = 24,   // О
    LetterP = 25,   // П
    LetterR = 26,   // Р
    LetterS = 27,   // С
    LetterT = 28,   // Т
    LetterU = 29,   // У
    LetterF = 30,   // Ф
    LetterH = 31,   // Х
    LetterC = 32,   // Ц
    LetterCh = 33,  // Ч
    LetterSh = 34,  // Ш
    LetterSha = 35, // Щ
    LetterE = 36,   // Э
    LetterYu = 37,  // Ю
    LetterYa = 38,  // Я

    LineVR = 39,    // |
    LineHR = 40,    // -
    LineSL = 41,    // /
    LineBSL = 42,   // \ 

    ArrowUp = 43,   // ↑
    ArrowDown = 44, // ↓
    ArrowLeft = 45, // ← 
    ArrowRight = 46,// → 

    MAX
};

