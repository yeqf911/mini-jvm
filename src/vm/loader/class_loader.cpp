//
// Created by 峰峰的🍎 on 2025/6/26.
//

#include "class_loader.h"

#include <iostream>

#include "vm/common/utils.h"
#include "vm/runtime/acc.h"
#include "vm/runtime/java_object.h"

#include <cstring>

namespace jvm::loader {

/* --------------------------------------------------------
 *  把 java/lang/String → <classpath>/java/lang/String.class
 * ------------------------------------------------------*/
// std::string ClassLoader::path_of(const std::string& class_name) const
// {
//     return classpath + "/" + class_name + ".class";
// }

rt::JavaClass* ClassLoader::load_class(const std::string& class_name)
{
    // 先检查是否加载过
    if (const auto it = cache.find(class_name); it != cache.end()) {
        return it->second.get();
    }

    // 读取磁盘
    std::ifstream in = classfile::get_class_file_stream(classpath, class_name);
    classfile::ClassFile cf = classfile::parse_class_file(std::move(in));
    // define & link
    rt::JavaClass* clazz = define_class(std::make_unique<classfile::ClassFile>(std::move(cf)));
    link(clazz);
    std::unique_ptr<rt::JavaClass> tmp(clazz);
    cache.emplace(class_name, std::move(tmp));
    // std::cout << "cache class: " << cache.size() << " loaded" << std::endl;
    return clazz;
}

/* --------------------------------------------------------
 *  defineClass：把 ClassFile 转换成 JavaClass（运行时结构）
 * ------------------------------------------------------*/
rt::JavaClass* ClassLoader::define_class(std::unique_ptr<classfile::ClassFile> cff)
{
    using rt::JavaClass;
    using rt::Slot;
    using rt::Acc;

    auto* clazz = new JavaClass;
    // 解析类名
    clazz->cf = std::move(cff);
    auto cf = clazz->cf.get();
    clazz->name = cf->cp.get<classfile::CpUtf8>(
        cf->cp.get<classfile::CpClass>(cf->this_class).name_index).str;

    u16 next_inst_slot = 0;   // 下一个实例字段槽号
    u16 next_static_slot = 0; // 下一个静态字段槽号

    // 收集字段 （包含普通成员和静态成员）
    for (auto& f : cf->fields) {
        bool is_static = (f.access_flags & Acc::ACC_STATIC); // 检查第 15 位是否为 1
        auto& rt_field = clazz->fields.emplace_back();       // 直接在 vector 末尾构造一个对象，并返回引用
        rt_field.acc = f.access_flags;
        rt_field.cp_name = f.name_index;
        rt_field.cp_desc = f.desc_index;
        rt_field.is_static = is_static;
        rt_field.slot = is_static ? next_static_slot++ : next_inst_slot++;
    }

    clazz->statics.resize(next_static_slot); // 分配静态变量槽，构造 n 个默认元素

    // 收集方法
    for (auto& m : cf->methods) {
        const classfile::CodeAttr* code = nullptr;
        for (auto& attr : m.attributes) {
            if (std::holds_alternative<classfile::CodeAttr>(attr)) {
                code = &std::get<classfile::CodeAttr>(attr);
            }
        }
        clazz->methods.push_back({m.access_flags, m.name_index, m.desc_index, code});
    }

    return clazz;
}

/* --------------------------------------------------------
 *  link：这里只做 prepare 子阶段
 *        - 把 static final 基本类型常量写入 statics
 * ------------------------------------------------------*/
void ClassLoader::link(rt::JavaClass* clazz)
{
    using namespace jvm::classfile;
    auto& cp = clazz->cf->cp;

    for (auto& f : clazz->cf->fields) {
        // 仅处理 static 类型
        if (const bool is_static = f.access_flags & rt::Acc::ACC_STATIC; !is_static)
            continue;

        // 只有 final 字段才会有 ConstantValue, jvm才会将 ConstantValue 写入静态变量区
        for (auto& attr : f.attributes) {
            // 找到 ConstantValueAttr 类型
            if (const auto cva = std::get_if<ConstantValueAttr>(&attr)) {
                // 属性值在常量池的 index
                const u16 value_index = cva->value_index;
                // 常量属性在常量池中的类型
                const CpTag& tag = cp.tags.at(value_index);
                // 计算ClassFile对象中当前的field在JavaClass对象中的fields中的index;
                // 定位对应的 FieldRT
                const size_t idx = &f - clazz->cf->fields.data();
                // 拿到 slot,因为当前 field 已经是 static 的了，所以拿到的 slot 也是 statics 的 slot
                const u16 slot = clazz->fields.at(idx).slot;
                // 将常量按类型写入 JavaClass.statics[slot]
                switch (tag) {
                case CpTag::String: {
                    const u16 utf8_idx = cp.get<CpString>(value_index).utf8_index;
                    std::string text = cp.get<CpUtf8>(utf8_idx).str;
                    auto str_clazz = load_class("java/lang/String");
                    auto obj = rt::new_object(str_clazz, 1);
                    // 仅设置一个槽演示用
                    obj->fields[0] = reinterpret_cast<u64>(new std::string(text));
                    clazz->statics[slot] = reinterpret_cast<u64>(obj);
                    break;
                }
                case CpTag::Integer: {
                    const auto v = cp.get<CpInt>(value_index).v;
                    // 设置静态变量槽位的值
                    clazz->statics[slot] = rt::static_to<i32>(v);
                    break;
                }
                case CpTag::Long: {
                    const auto v = cp.get<CpLong>(value_index).v;
                    clazz->statics[slot] = rt::static_to<i64>(v);
                    break;
                }
                case CpTag::Float: { // float 和 double 用位模式 copy 保证精度不丢失
                    const auto v = cp.get<CpFloat>(value_index).v;
                    u32 bits;
                    std::memcpy(&bits, &v, sizeof(bits));
                    clazz->statics[slot] = rt::static_to<u32>(bits);
                    break;
                }
                case CpTag::Double: {
                    const auto v = cp.get<CpDouble>(value_index).v;
                    u64 bits;
                    std::memcpy(&bits, &v, sizeof(bits));
                    clazz->statics[slot] = rt::static_to<u64>(bits);
                    break;
                }
                default:
                    util::die("Unsupported ConstantValue CpTag");
                    break;
                }
            }
        }
    }
}

} // namespace jvm::loader