#ifndef MTBMODULES_H
#define MTBMODULES_H

#include "QStringList"

class TMtbModuleState {
public:
    int address;
    int type;
    QString name;
    bool active;
    bool warning;
    bool error;
};

class TMtbModuleTypes {
public:
    enum mtbModuleType {
        mtbTypeUNIv2 = 0x10,
        mtbTypeUNIv2noIR = 0x11,
        mtbTypeUNIv4 = 0x15,
        mtbTypeUNIv42 = 0x16,
        mtbTypeBOOST = 0x20,
        mtbTypeRC = 0x30,
        mtbTypeLC = 0x40, // level crossing
        mtbTypeUINS = 0x50
    };
    const int map[8] = {0x10, 0x11, 0x15, 0x16, 0x20, 0x30, 0x40, 0x50};
    const int count = 8;

    const QStringList names ={
        "UNI v2",
        "UNI v2 no IR",
        "UNI v4.0",
        "UNI v4.2",
        "BOOST",
        "MTB-RC",
        "MTB-LC",
        "UNIS"
    };

    // conversion functions
    int IndexToType(int index);
    int TypeToIndex(int type);
    QString GetNameByIndex(int index);
    QString GetNameByType(int type);
};

#endif // MTBMODULES_H
