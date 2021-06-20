#include <inttypes.h>


typedef uint64_t tFlags;

#define FL(pos) (1<<pos)

enum tFlag
{
    fRotation = 1<<0,
    fJustBorrowed = 1<<3,        // e.g. used in tSetting to detect m_values just borrowed
    fAutoSet = 1<<4,
    fDisabled = 1<<5,
    fReseatable = 1<<6,
    fClasses = 1<<7
};

enum tMutex
{
    fGeneralMutex = 1<<0,
    fCallbackMutex = 1<<1
};
