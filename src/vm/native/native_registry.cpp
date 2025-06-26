//
// Created by 峰峰的🍎 on 2025/6/26.
//

#include <unordered_map>
#include "native_registry.h"

#include "vm/common/utils.h"

namespace jvm::native {

using Key = std::tuple<std::string, std::string, std::string>; // 类名,方法名,描述符

// 运行时只保留这一个散列表用于存储 native 方法
static std::unordered_map<Key, NativeFn, util::TupleHash> tbl;

void register_natives(const std::string& clazz,
                      const std::string& name,
                      const std::string& desc,
                      NativeFn fn)
{
    tbl[{clazz, name, desc}] = std::move(fn);
}

NativeFn find(const std::string& clazz,
                     const std::string& name,
                     const std::string& desc)
{
    const auto it = tbl.find({clazz, name, desc});
    return it == tbl.end() ? nullptr : it->second; // 找不到返回 nullptr
}

} // namespace jvm::native