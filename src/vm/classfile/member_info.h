//
// Created by 峰峰的🍎 on 2025/6/25.
//

#ifndef MEMBER_INFO_H
#define MEMBER_INFO_H
#include <vector>

#include "attribute.h"
#include "vm/common/types.h"

namespace jvm::classfile {

struct MemberInfo {
    u16 access_flags;
    u16 name_index;
    u16 desc_index;
    std::vector<Attribute> attributes;
};

}

#endif //MEMBER_INFO_H