//
// Created by 峰峰的🍎 on 2025/6/26.
//

#ifndef SLOTS_H
#define SLOTS_H

#include "vm/common/types.h"

namespace jvm::rt {

// 通用槽
using Slot = u64;

template <typename T> T as(Slot v) { return static_cast<T>(v); }

template <typename T> Slot static_to(T v) { return static_cast<Slot>(v); }

template <typename T> Slot reinterpret_to(T v) { return reinterpret_cast<Slot>(v); }

}

#endif //SLOTS_H