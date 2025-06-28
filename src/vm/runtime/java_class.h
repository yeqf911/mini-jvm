//
// Created by 峰峰的🍎 on 2025/6/26.
//

#ifndef JAVA_CLASS_H
#define JAVA_CLASS_H

#include <string>

#include "slots.h"
#include "vm/classfile/class_file.h"
#include "vm/classfile/member_info.h"

namespace jvm::rt {

/**
 * JavaClass: ClassLoader 解析完后在 JVM 中的“类元数据”
 * 这里只保留最小子集
 * - name   : 完整二进制名称
 * - super  : 父类指针
 * - cf     : 源 ClassFile（只读）
 * - statics: 静态变量槽
 * - fields : 字段运行时布局
 * - methods: 方法运行时布局（这里只存储 Code 指针）
 */
struct JavaClass {
    std::string name;      // java/lang/String
    JavaClass* super{};    // 父类
    std::unique_ptr<classfile::ClassFile> cf; // 源 ClassFile

    // 静态字段槽
    std::vector<Slot> statics; // 大小为 static 字段槽数

    // 字段布局
    struct FieldRT {
        u16 acc,        // 访问标志
            cp_name,    // 常量池 name 索引
            cp_desc,    // 常量池 desc 索引
            slot;       // 在对象或者 statics 向量中的槽号
        bool is_static; // 是否 static
    };

    std::vector<FieldRT> fields;

    // 方法布局，只存储 code 指针
    struct MethodRT {
        u16 acc,
            cp_name,
            cp_desc;
        const classfile::CodeAttr* code{}; // 指向 Code 属性
    };

    std::vector<MethodRT> methods;
    bool clint_called = false; // <clinit> 是否执行过
};

}

#endif //JAVA_CLASS_H