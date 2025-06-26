//
// Created by 峰峰的🍎 on 2025/6/25.
//

#ifndef UTILS_H
#define UTILS_H
#include <fstream>
#include <stdexcept>
#include "types.h"

namespace jvm::util {
// 按大端读取无符号整数
inline u8 read_byte(std::istream& in) { return in.get(); }

inline u16 read_word(std::istream& in)
{
    // 两次 in.get() --> 0x1200 | 0x0034 --> 0x1234
    return (in.get() << 8) | in.get();
}

inline u32 read_dword(std::istream& in)
{
    return static_cast<u32>(read_word(in)) << 16 | read_word(in);
}

inline std::vector<u16> read_word_vector(std::istream& in)
{
    u16 n = read_word(in);
    std::vector<u16> v(n);
    for (u16& e : v) {
        e = read_word(in);
    }
    return v;
}

/**
 * 异常终止
 * @param msg 异常信息
 */
inline void die(const std::string& msg)
{
    throw std::runtime_error(msg);
}

using Key = std::tuple<std::string, std::string, std::string>;

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

}

#endif //UTILS_H