//
// Created by 峰峰的🍎 on 2025/6/25.
//

#ifndef ATTRIBUTE_H
#define ATTRIBUTE_H

#include <vector>

#include "vm/common/types.h"

namespace jvm::classfile {
struct CodeAttr {
    u16 max_stack{};
    u16 max_locals{};
    std::vector<u8> code; // 保存 code 字节码
};

struct ConstantValueAttr {
    u16 value_index{};
};

struct SourceFileAttr {
    u16 sourcefile_index{};
};

struct RawAttr {
    std::vector<u8> info; // 其他属性
};

using Attribute = std::variant<CodeAttr, ConstantValueAttr, SourceFileAttr, RawAttr>;
} // namespace classfile

#endif //ATTRIBUTE_H