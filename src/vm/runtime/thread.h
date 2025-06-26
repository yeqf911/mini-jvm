//
// Created by 峰峰的🍎 on 2025/6/26.
//

#ifndef THREAD_H
#define THREAD_H
#include "stack_frame.h"

namespace jvm::rt {

/**
 * 目前只支持单线程执行
 * 维护一个帧链表，top指向当前栈帧
 */
struct Thread {
    StackFrame* top{}; // 顶部栈帧

    // 压栈
    void push(StackFrame* frame)
    {
        frame->prev = top;
        top = frame;
    }

    // 出栈
    void pop()
    {
        const auto tmp = top;
        top = top->prev;
        delete tmp;
    }
};

}

#endif //THREAD_H