
typedef uint64_t tSignals;

enum tSignal
{
    sigAll = 0xFFFFFFFFFFFF,
    sigDecaying = 1<<0,
    sigChanged = 1<<1,              // general changed signal accompanying other changed-signals
    sigDataChanged = 1<<2,           // something within a structure but not the structure itself changed (e.g. matrix data)
    sigExtentChanged = 1<<3,            // structure changed (e.g. matrix data extent)
    sigInputChanged = 1<<4,             // some input changed (e.g. new tWindow as input)
    sigOutputChanged = 1<<5,
    sigNameChanged = 1<<6,
    sigSettingChanged = 1<<7,
    sigBeforeChange = 1<<8,
    sigByUser = 1<<9,

    sigAND = 1 << 31
};

#define T_SIG(x) (static_cast<tSignal>(x))
