//
// Created by 峰峰的🍎 on 2025/6/26.
//

#ifndef JAVA_OBJECT_H
#define JAVA_OBJECT_H
#include <vector>

#include "slots.h"

namespace jvm::rt {

struct JavaClass;

/**
 *  JavaObject: 所有 Java 实例在 c++ 中的通用表示
 *  - clazz   : 指向所属运行时类
 *  - fields  : 实例字段槽（大小由 ClassLoader 在加载时决定）
 */
struct JavaObject {
    JavaClass* clazz{};       // 指向运行时类对象
    std::vector<Slot> fields; // 实例字段槽
};

using Oop = JavaObject*; // 普通对象指针 ordinary object pointer

/**
 * 堆内分配一个对象
 * @param clazz 所属类
 * @param slot_count 字段槽数量
 * @return
 */
inline Oop new_object(JavaClass* clazz, const size_t slot_count)
{
    auto* obj = new JavaObject; // 暂时没有 GC，直接 new
    obj->clazz = clazz;
    obj->fields.resize(slot_count); // 填充 slot_count 个 0 到槽中
    return obj;
}

} // namespace jvm::rt

#endif //JAVA_OBJECT_H