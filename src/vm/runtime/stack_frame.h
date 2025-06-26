//
// Created by 峰峰的🍎 on 2025/6/26.
//

#ifndef FRAME_H
#define FRAME_H

#include "slots.h"
#include "vm/classfile/attribute.h"

using namespace jvm::classfile;

namespace jvm::rt {

struct JavaClass;
struct JavaObject;

struct StackFrame {
    JavaClass* owner{};       // 拥有该方法的类
    const CodeAttr* code{};   // Code属性，用于取字节码
    std::vector<Slot> locals; // 局部变量表
    std::vector<Slot> stack;  // 操作数栈
    size_t pc = 0;            // 当前字节码下标（程序计数器）
    StackFrame* prev{};       // 上一个栈帧（调用者）

    StackFrame(const size_t max_locals, const size_t max_stack)
        : locals(max_locals) // 初始化元素 size() = max_locals
    {
        stack.reserve(max_stack); // 预留栈大小, 仅分配空间，为初始化元素, size() = 0
    }
};

}

#endif //FRAME_H