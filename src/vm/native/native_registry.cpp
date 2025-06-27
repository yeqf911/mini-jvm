//
// Created by 峰峰的🍎 on 2025/6/26.
//

#include <unordered_map>
#include "native_registry.h"

#include "vm/common/utils.h"

namespace jvm::native {

using Key = std::tuple<std::string, std::string, std::string>; // 类名,方法名,描述符

// 为 Key 类型自定义 hash
struct TupleHash {
    std::size_t operator()(const Key& key) const noexcept
    {
        constexpr std::hash<std::string> hasher;
        std::size_t seed = hasher(std::get<0>(key));
        auto combine = [](std::size_t& s, const std::size_t v) {
            s ^= v + 0x9e3779b9 + (s << 6) + (s >> 2);
        };
        combine(seed, hasher(std::get<1>(key)));
        combine(seed, hasher(std::get<2>(key)));
        return seed;
    }
};

// 运行时只保留这一个散列表用于存储 native 方法
// 定义方法是因为避免静态资源初始化顺序出错
static std::unordered_map<Key, NativeFn, TupleHash>& native_table()
{
    static std::unordered_map<Key, NativeFn, TupleHash> tbl;
    return tbl;
}

void register_natives(const std::string& clazz,
                      const std::string& name,
                      const std::string& desc,
                      NativeFn fn)
{
    auto& tbl = native_table();
    tbl[{clazz, name, desc}] = std::move(fn);
}

NativeFn find(const std::string& clazz,
              const std::string& name,
              const std::string& desc)
{
    auto& tbl = native_table();
    const auto it = tbl.find({clazz, name, desc});
    return it == tbl.end() ? nullptr : it->second; // 找不到返回 nullptr
}

} // namespace jvm::native