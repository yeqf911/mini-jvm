//
// Created by 峰峰的🍎 on 2025/6/26.
//

#ifndef NATIVE_REGISTRY_H
#define NATIVE_REGISTRY_H
#include <functional>
#include <string>

namespace jvm::rt {
class StackFrame;
}

namespace jvm::native {

/*
 * NativeFn: 用 std::function 包一层，参数为当前栈帧的引用，
 *           本地方法直接在帧的操作数栈里取/放值
 */
using NativeFn = std::function<void(rt::StackFrame&)>;

void register_natives(const std::string& clazz, // 类名  java/io/Print
                      const std::string& name,  // 方法名 println
                      const std::string& desc,  // 方法描述符 (Ljava/lang/String;)V
                      NativeFn fn);

NativeFn find(const std::string& clazz,
                     const std::string& name,
                     const std::string& desc);

}

#endif //NATIVE_REGISTRY_H