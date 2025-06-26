//
// Created by 峰峰的🍎 on 2025/6/26.
//

#ifndef ACC_H
#define ACC_H
#include "vm/common/types.h"

namespace jvm::rt {

enum Acc : u16 {
    ACC_PUBLIC     = 0x0001, // 第0位  0000 0000 0000 0001
    ACC_PRIVATE    = 0x0002, // 第1位  0000 0000 0000 0010
    ACC_PROTECTED  = 0x0004, // 第2位  0000 0000 0000 0100
    ACC_STATIC     = 0x0008, // 第3位  0000 0000 0000 1000
    ACC_FINAL      = 0x0010, // 第4位  0000 0000 0001 0000
    ACC_SUPER      = 0x0020, // 第5位  0000 0000 0010 0000
    ACC_INTERFACE  = 0x0200, // 第9位  0000 0010 0000 0000
    ACC_ABSTRACT   = 0x0400, // 第10位 0000 0100 0000 0000
    ACC_SYNTHETIC  = 0x1000, // 第12位 0001 0000 0000 0000
    ACC_ANNOTATION = 0x2000, // 第13位 0010 0000 0000 0000
    ACC_ENUM       = 0x4000, // 第14位 0100 0000 0000 0000
};

}

#endif //ACC_H