//
// Created by 峰峰的🍎 on 2025/6/28.
//

#ifndef JVM_OPCODE_H
#define JVM_OPCODE_H

// jvm_opcode.hpp
#pragma once
#include <array>
#include <cstdint>
#include <optional>
#include <string_view>
#include <unordered_map>

namespace jvm::interp {

/* ───────────────────────────────────────────
   1. 一张总表：opcode, mnemonic（官方写法）
   改动 / 新增指令时，只维护这一处
   ─────────────────────────────────────────── */
#define JVM_OPCODE_LIST(X)                                                       \
    X(0x00, nop)                X(0x01, aconst_null)       X(0x02, iconst_m1)    \
    X(0x03, iconst_0)           X(0x04, iconst_1)          X(0x05, iconst_2)     \
    X(0x06, iconst_3)           X(0x07, iconst_4)          X(0x08, iconst_5)     \
    X(0x09, lconst_0)           X(0x0A, lconst_1)          X(0x0B, fconst_0)     \
    X(0x0C, fconst_1)           X(0x0D, fconst_2)          X(0x0E, dconst_0)     \
    X(0x0F, dconst_1)           X(0x10, bipush)            X(0x11, sipush)       \
    X(0x12, ldc)                X(0x13, ldc_w)             X(0x14, ldc2_w)       \
    X(0x15, iload)              X(0x16, lload)             X(0x17, fload)        \
    X(0x18, dload)              X(0x19, aload)             X(0x1A, iload_0)      \
    X(0x1B, iload_1)            X(0x1C, iload_2)           X(0x1D, iload_3)      \
    X(0x1E, lload_0)            X(0x1F, lload_1)           X(0x20, lload_2)      \
    X(0x21, lload_3)            X(0x22, fload_0)           X(0x23, fload_1)      \
    X(0x24, fload_2)            X(0x25, fload_3)           X(0x26, dload_0)      \
    X(0x27, dload_1)            X(0x28, dload_2)           X(0x29, dload_3)      \
    X(0x2A, aload_0)            X(0x2B, aload_1)           X(0x2C, aload_2)      \
    X(0x2D, aload_3)            X(0x2E, iaload)            X(0x2F, laload)       \
    X(0x30, faload)             X(0x31, daload)            X(0x32, aaload)       \
    X(0x33, baload)             X(0x34, caload)            X(0x35, saload)       \
    X(0x36, istore)             X(0x37, lstore)            X(0x38, fstore)       \
    X(0x39, dstore)             X(0x3A, astore)            X(0x3B, istore_0)     \
    X(0x3C, istore_1)           X(0x3D, istore_2)          X(0x3E, istore_3)     \
    X(0x3F, lstore_0)           X(0x40, lstore_1)          X(0x41, lstore_2)     \
    X(0x42, lstore_3)           X(0x43, fstore_0)          X(0x44, fstore_1)     \
    X(0x45, fstore_2)           X(0x46, fstore_3)          X(0x47, dstore_0)     \
    X(0x48, dstore_1)           X(0x49, dstore_2)          X(0x4A, dstore_3)     \
    X(0x4B, astore_0)           X(0x4C, astore_1)          X(0x4D, astore_2)     \
    X(0x4E, astore_3)           X(0x4F, iastore)           X(0x50, lastore)      \
    X(0x51, fastore)            X(0x52, dastore)           X(0x53, aastore)      \
    X(0x54, bastore)            X(0x55, castore)           X(0x56, sastore)      \
    X(0x57, pop)                X(0x58, pop2)              X(0x59, dup)          \
    X(0x5A, dup_x1)             X(0x5B, dup_x2)            X(0x5C, dup2)         \
    X(0x5D, dup2_x1)            X(0x5E, dup2_x2)           X(0x5F, swap)         \
    X(0x60, iadd)               X(0x61, ladd)              X(0x62, fadd)         \
    X(0x63, dadd)               X(0x64, isub)              X(0x65, lsub)         \
    X(0x66, fsub)               X(0x67, dsub)              X(0x68, imul)         \
    X(0x69, lmul)               X(0x6A, fmul)              X(0x6B, dmul)         \
    X(0x6C, idiv)               X(0x6D, ldiv)              X(0x6E, fdiv)         \
    X(0x6F, ddiv)               X(0x70, irem)              X(0x71, lrem)         \
    X(0x72, frem)               X(0x73, drem)              X(0x74, ineg)         \
    X(0x75, lneg)               X(0x76, fneg)              X(0x77, dneg)         \
    X(0x78, ishl)               X(0x79, lshl)              X(0x7A, ishr)         \
    X(0x7B, lshr)               X(0x7C, iushr)             X(0x7D, lushr)        \
    X(0x7E, iand)               X(0x7F, land)              X(0x80, ior)          \
    X(0x81, lor)                X(0x82, ixor)              X(0x83, lxor)         \
    X(0x84, iinc)               X(0x85, i2l)               X(0x86, i2f)          \
    X(0x87, i2d)               X(0x88, l2i)               X(0x89, l2f)          \
    X(0x8A, l2d)               X(0x8B, f2i)               X(0x8C, f2l)          \
    X(0x8D, f2d)               X(0x8E, d2i)               X(0x8F, d2l)          \
    X(0x90, d2f)               X(0x91, i2b)               X(0x92, i2c)          \
    X(0x93, i2s)               X(0x94, lcmp)              X(0x95, fcmpl)        \
    X(0x96, fcmpg)             X(0x97, dcmpl)             X(0x98, dcmpg)        \
    X(0x99, ifeq)              X(0x9A, ifne)              X(0x9B, iflt)         \
    X(0x9C, ifge)              X(0x9D, ifgt)              X(0x9E, ifle)         \
    X(0x9F, if_icmpeq)         X(0xA0, if_icmpne)         X(0xA1, if_icmplt)    \
    X(0xA2, if_icmpge)         X(0xA3, if_icmpgt)         X(0xA4, if_icmple)    \
    X(0xA5, if_acmpeq)         X(0xA6, if_acmpne)         X(0xA7, goto_)        \
    X(0xA8, jsr)               X(0xA9, ret)               X(0xAA, tableswitch)  \
    X(0xAB, lookupswitch)      X(0xAC, ireturn)           X(0xAD, lreturn)      \
    X(0xAE, freturn)           X(0xAF, dreturn)           X(0xB0, areturn)      \
    X(0xB1, return_)           X(0xB2, getstatic)         X(0xB3, putstatic)    \
    X(0xB4, getfield)          X(0xB5, putfield)          X(0xB6, invokevirtual)\
    X(0xB7, invokespecial)     X(0xB8, invokestatic)      X(0xB9, invokeinterface)\
    X(0xBA, invokedynamic)     X(0xBB, new_)              X(0xBC, newarray)     \
    X(0xBD, anewarray)         X(0xBE, arraylength)       X(0xBF, athrow)       \
    X(0xC0, checkcast)         X(0xC1, instanceof)        X(0xC2, monitorenter) \
    X(0xC3, monitorexit)       X(0xC4, wide)              X(0xC5, multianewarray)\
    X(0xC6, ifnull)            X(0xC7, ifnonnull)         X(0xC8, goto_w)       \
    X(0xC9, jsr_w)             X(0xCA, breakpoint)                                     \
    /* 0xCB–0xFD 预留 */                                                               \
    X(0xCB, reserved_cb)       X(0xCC, reserved_cc)       X(0xCD, reserved_cd)  \
    X(0xCE, reserved_ce)       X(0xCF, reserved_cf)       X(0xD0, reserved_d0)  \
    X(0xD1, reserved_d1)       X(0xD2, reserved_d2)       X(0xD3, reserved_d3)  \
    X(0xD4, reserved_d4)       X(0xD5, reserved_d5)       X(0xD6, reserved_d6)  \
    X(0xD7, reserved_d7)       X(0xD8, reserved_d8)       X(0xD9, reserved_d9)  \
    X(0xDA, reserved_da)       X(0xDB, reserved_db)       X(0xDC, reserved_dc)  \
    X(0xDD, reserved_dd)       X(0xDE, reserved_de)       X(0xDF, reserved_df)  \
    X(0xE0, reserved_e0)       X(0xE1, reserved_e1)       X(0xE2, reserved_e2)  \
    X(0xE3, reserved_e3)       X(0xE4, reserved_e4)       X(0xE5, reserved_e5)  \
    X(0xE6, reserved_e6)       X(0xE7, reserved_e7)       X(0xE8, reserved_e8)  \
    X(0xE9, reserved_e9)       X(0xEA, reserved_ea)       X(0xEB, reserved_eb)  \
    X(0xEC, reserved_ec)       X(0xED, reserved_ed)       X(0xEE, reserved_ee)  \
    X(0xEF, reserved_ef)       X(0xF0, reserved_f0)       X(0xF1, reserved_f1)  \
    X(0xF2, reserved_f2)       X(0xF3, reserved_f3)       X(0xF4, reserved_f4)  \
    X(0xF5, reserved_f5)       X(0xF6, reserved_f6)       X(0xF7, reserved_f7)  \
    X(0xF8, reserved_f8)       X(0xF9, reserved_f9)       X(0xFA, reserved_fa)  \
    X(0xFB, reserved_fb)       X(0xFC, reserved_fc)       X(0xFD, reserved_fd)  \
    X(0xFE, impdep1)           X(0xFF, impdep2)

/* ───────────────────────────────────────────
   2. enum class Op : uint8_t
   ─────────────────────────────────────────── */
enum class Instruction : std::uint8_t {
#define X(code, name) name = code,
    JVM_OPCODE_LIST(X)
#undef  X
};

/* ───────────────────────────────────────────
   3. opcode → 助记词字符串  (constexpr 数组)
   ─────────────────────────────────────────── */
inline constexpr std::array<std::string_view, 256> mnemonic = [] {
    std::array<std::string_view, 256> a{};
#define X(code, name) a[code] = #name;
    JVM_OPCODE_LIST(X)
#undef  X
    return a;
}();

/* ───────────────────────────────────────────
   4. enum → string   (constexpr)
   ─────────────────────────────────────────── */
constexpr std::string_view to_string(Instruction op) noexcept
{
    return mnemonic[static_cast<std::uint8_t>(op)];
}

/* ───────────────────────────────────────────
   5. string → enum   (运行期反查)
   ─────────────────────────────────────────── */
inline std::optional<Instruction> from_string(std::string_view s)
{
    static const auto table = [] {
        std::unordered_map<std::string_view, Instruction> m;
#define X(code, name) m.emplace(#name, Instruction::name);
        JVM_OPCODE_LIST(X)
#undef  X
        return m;
    }();

    auto it = table.find(s);
    return it == table.end()
               ? std::nullopt
               : std::optional<Instruction>{it->second};
}

} // namespace jvm

#endif //JVM_OPCODE_H