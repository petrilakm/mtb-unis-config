#include "MtbModules.h"

int TMtbModuleTypes::IndexToType(int index)
{
    if((index >= 0) && (index < count)) {
        return map[index];
    }
    return -1;
}

int TMtbModuleTypes::TypeToIndex(int type)
{
    for(int i = 0; i < count; i++) {
        if (map[i] == type) return i;
    }
    return 0;
}

QString TMtbModuleTypes::GetNameByIndex(int index)
{
    if((index >= 0) && (index < count)) {
        return names[index];
    }
    return QString("-");
}

QString TMtbModuleTypes::GetNameByType(int type)
{
    int index = TypeToIndex(type);
    if (index > -1) {
        return GetNameByIndex(index);
    }
    return QString("-");
}
