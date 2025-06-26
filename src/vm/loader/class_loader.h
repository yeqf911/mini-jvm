//
// Created by 峰峰的🍎 on 2025/6/26.
//

#ifndef CLASS_LOADER_H
#define CLASS_LOADER_H
#include <string>
#include <memory>
#include <unordered_map>

#include "vm/runtime/java_class.h"

namespace jvm::loader {

/*
 * ClassLoader: 只实现最简单的 Bootstrap ClassLoader
 * - classpath: 搜索 .class 文件的根目录
 * - cache    : 已加载类缓存，防止重复加载
 */
class ClassLoader {
public:
    explicit ClassLoader(std::string cp) : classpath(std::move(cp)) {};

    // 根据二进制名加载类？二进制名是什么
    rt::JavaClass* load_class(const std::string& class_name);

private:
    std::string classpath;                                                 // 搜索路径
    std::unordered_map<std::string, std::unique_ptr<rt::JavaClass>> cache; // 加载的类的缓存
    rt::JavaClass* define_class(const classfile::ClassFile& cf);           // 创建 JavaClass 对象
    // std::string path_of(const std::string& class_name) const;             // 转磁盘路径
    void link(rt::JavaClass*);                                             // prepare 阶段
};

} // namespace jvm::loader

#endif //CLASS_LOADER_H