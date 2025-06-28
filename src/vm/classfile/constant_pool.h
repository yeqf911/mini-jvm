//
// Created by 峰峰的🍎 on 2025/6/25.
//

#ifndef CONSTANT_POOL_H
#define CONSTANT_POOL_H

#include <string>
#include <vector>
#include <variant>

#include "vm/common/types.h"
/**
    1  CONSTANT_Utf8
    3  CONSTANT_Integer
    4  CONSTANT_Float
    5  CONSTANT_Long
    6  CONSTANT_Double
    7  CONSTANT_Class
    8  CONSTANT_String
    9  CONSTANT_Fieldref
    10 CONSTANT_Methodref
    11 CONSTANT_InterfaceMethodref
    12 CONSTANT_NameAndType
    15 CONSTANT_MethodHandle        <-- Java 7 新增
    16 CONSTANT_MethodType          <-- Java 7 新增
    18 CONSTANT_InvokeDynamic       <-- Java 7 新增
    2,13,14,17  当前保留未用
 */

namespace jvm::classfile {

enum class CpTag : uint8_t {
    Utf8               = 1,
    Integer            = 3,
    Float              = 4,
    Long               = 5,
    Double             = 6,
    Class              = 7,
    String             = 8,
    FiledRef           = 9,
    MethodRef          = 10,
    InterfaceMethodRef = 11,
    NameAndType        = 12,
    MethodHandle       = 15,
    MethodType         = 16,
    InvokeDynamic      = 18
};

/* ------------ 每种 cp_info 结构体 ------------ */
struct CpUtf8          { std::string str; };
struct CpInt           { i32 v; };
struct CpFloat         { float v; };
struct CpLong          { i64 v; };
struct CpDouble        { double v; };
struct CpClass         { u16 name_index; };
struct CpString        { u16 utf8_index; };
struct CpFieldMethod   { u16 class_index, nt_index; };
struct CpNameAndType   { u16 name_index, desc_index; };
struct CpMethodHandle  { u8 ref_kind; uint16_t ref_index; };
struct CpMethodType    { u16 desc_index; };
struct CpInvokeDynamic { u16 bootstrap, nt_index; };

using CpInfo = std::variant<CpUtf8, CpInt, CpFloat, CpLong, CpDouble, CpClass, CpString,
                            CpFieldMethod, CpNameAndType, CpMethodType, CpMethodHandle,
                            CpInvokeDynamic>;

// template<CpTag Tag> struct CpType;
// template<> struct CpType<CpTag::Utf8>               { using type = CpUtf8; };
// template<> struct CpType<CpTag::Integer>            { using type = CpInt; };
// template<> struct CpType<CpTag::Float>              { using type = CpFloat; };
// template<> struct CpType<CpTag::Long>               { using type = CpLong; };
// template<> struct CpType<CpTag::Double>             { using type = CpDouble; };
// template<> struct CpType<CpTag::Class>              { using type = CpClass; };
// template<> struct CpType<CpTag::String>             { using type = CpString; };
// template<> struct CpType<CpTag::FiledRef>           { using type = CpFieldMethod; };
// template<> struct CpType<CpTag::MethodRef>          { using type = CpFieldMethod; };
// template<> struct CpType<CpTag::InterfaceMethodRef> { using type = CpFieldMethod; };
// template<> struct CpType<CpTag::NameAndType>        { using type = CpNameAndType; };
// template<> struct CpType<CpTag::MethodHandle>       { using type = CpMethodHandle; };
// template<> struct CpType<CpTag::MethodType>         { using type = CpMethodType; };
// template<> struct CpType<CpTag::InvokeDynamic>      { using type = CpInvokeDynamic; };
//
// template<CpTag Tag>
// using CpType_t = typename CpType<Tag>::type;

// template<CpTag Tag>
// CpType<Tag> get_type(const CpTag& tag)
// {
//     switch (tag) {
//         case CpTag::Utf8:
//         return CpType<CpTag::Utf8>{};
//     default:
//         return CpType<Tag>{};
//     }
// }

// 常量池容器
struct ConstantPool {
    std::vector<CpInfo> pool{};
    std::vector<CpTag> tags{};

    template <typename T>
    const T& get(const u16 index) const
    {
        return std::get<T>(pool.at(index));
    }
};

} // namespace jvm::classfile
#endif //CONSTANT_POOL_H
