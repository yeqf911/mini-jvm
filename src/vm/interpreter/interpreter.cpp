//
// Created by 峰峰的🍎 on 2025/6/26.
//

#include "interpreter.h"

#include <iostream>
#include <iomanip>

#include "vm/common/utils.h"
#include "vm/native/native_registry.h"
#include "vm/runtime/acc.h"
#include "jvm_opcode.h"
#include "vm/runtime/java_object.h"

using namespace jvm;

namespace jvm::interp {

void Interpreter::run_main(const std::string& cls)
{
    auto clazz = loader.load_class(cls);
    print_summery(*clazz->cf);
    const rt::JavaClass::MethodRT* main = nullptr;
    for (const auto& m : clazz->methods) {
        auto name = clazz->cf->cp.get<CpUtf8>(m.cp_name).str;
        auto desc = clazz->cf->cp.get<CpUtf8>(m.cp_desc).str;
        if (name == "main" && desc == "([Ljava/lang/String;)V" && (m.acc & rt::Acc::ACC_STATIC)) {
            main = &m;
            break;
        }
    }

    if (!main) {
        throw std::runtime_error("method not found");
    }

    invoke(clazz, *main, {});
    exec_loop();
}

void Interpreter::invoke(rt::JavaClass* owner, const rt::JavaClass::MethodRT& m,
                         std::vector<rt::Slot> args)
{
    using namespace rt;

    const auto m_name = owner->cf->cp.get<CpUtf8>(m.cp_name).str;
    const auto m_desc = owner->cf->cp.get<CpUtf8>(m.cp_desc).str;

    // 处理 native 方法
    if (const auto nf = native::find(owner->name, m_name, m_desc)) {
        auto* frame = new StackFrame(0, 0);
        frame->stack = std::move(args);
        thread.push(frame);
        nf(*frame);
        thread.pop();
        return;
    }

    /*----- 非 native 方法 -----*/
    auto* code = m.code;
    if (code == nullptr) {
        throw std::runtime_error("no codeAttr");
    }

    // 创建普通方法的栈帧
    auto* frame = new StackFrame(code->max_locals, code->max_stack);
    frame->owner = owner;
    frame->code = code;

    // 将实参复制进本地变量表
    for (size_t i = 0; i < args.size(); ++i) {
        frame->locals[i] = args[i];
    }
    // 压入栈中
    thread.push(frame);
}

// 主循环
void Interpreter::exec_loop()
{
    using jvm::interp::Instruction;
    while (thread.top) {                     // 当线程还有栈帧
        rt::StackFrame& frame = *thread.top; // 取线程栈顶的栈帧
        auto& code = frame.code->code;       // 字节码向量
        const u8 op = code[frame.pc++];      // 取 opcode，程序计数器+1
        std::cout << "0x";
        std::cout << std::hex    // 切换到 16 进制
            << std::setw(2)      // 宽度 2 位
            << std::setfill('0') // 不足补 0
            << static_cast<int>(op)
            << std::dec; // 打完恢复 10 进制
        std::cout << "\t" << std::string(to_string(static_cast<Instruction>(op))) << '\n';

        switch (static_cast<Instruction>(op)) {
        case Instruction::iconst_m1:
        case Instruction::iconst_0:
        case Instruction::iconst_1:
        case Instruction::iconst_2: // iconst_-1..2
        case Instruction::iconst_3:
        case Instruction::iconst_4:
        case Instruction::iconst_5: { // iconst_3..5
            const int val = static_cast<i8>(op) - 3;
            frame.stack.push_back(val);
            break;
        }
        case Instruction::bipush: { // 读取 1 字节带符号立即数
            i8 value = code[frame.pc++];
            frame.stack.push_back(rt::static_to<i32>(value));
            break;
        }
        case Instruction::ldc: { // 将后面的索引对应的目标推入操作数栈，索引为常量池的有效索引
            u8 idx = code[frame.pc++];
            auto& cp = frame.owner->cf->cp;
            switch (cp.tags[idx]) {
            case CpTag::Integer: {
                i32 v = cp.get<CpInt>(idx).v;
                frame.stack.push_back(rt::static_to<i32>(v));
                break;
            }
            case CpTag::String: {
                auto utf_index = cp.get<CpString>(idx).utf8_index;
                std::string txt = cp.get<CpUtf8>(utf_index).str;

                // 在堆中构造 java/lang/String 对象
                rt::JavaClass* cls = loader.load_class("java/lang/String");
                rt::JavaObject* obj = rt::new_object(cls, 1);

                // 在第 0 槽位放置 std::string* 指针
                obj->fields[0] = rt::reinterpret_to(new std::string(txt));
                frame.stack.push_back(rt::reinterpret_to(obj));
                break;
            }
            default:
                throw std::runtime_error("idc type not yet implement");
            }
            break;
        }
        case Instruction::iadd: { // 算术指令
            i32 v2 = rt::as<i32>(frame.stack.back());
            frame.stack.pop_back();
            i32 v1 = rt::as<i32>(frame.stack.back());
            frame.stack.pop_back();
            frame.stack.push_back(rt::static_to(v1 + v2));
            break;
        }
        case Instruction::getstatic: { // 访问静态字段
            // 读取两个字节构成常量池索引
            u8 p1 = code[frame.pc++];
            u8 p2 = code[frame.pc++];
            u16 idx = p1 << 8 | p2;
            auto& cp = frame.owner->cf->cp;
            auto& field = cp.get<CpFieldMethod>(idx);

            // 解析目标类
            std::string cls_name = cp.get<CpUtf8>(cp.get<CpClass>(field.class_index).name_index).
                                      str;
            rt::JavaClass* target_cls = loader.load_class(cls_name);

            // 解析字段名和字段类型
            auto& nt = cp.get<CpNameAndType>(field.nt_index);
            std::string f_name = cp.get<CpUtf8>(nt.name_index).str;

            // 在线性表中扫描字段
            for (auto& f : target_cls->fields) {
                if (target_cls->cf->cp.get<CpUtf8>(f.cp_name).str == f_name && f.is_static) {
                    frame.stack.push_back(target_cls->statics[f.slot]);
                    break;
                }
            }
            break;
        }
        case Instruction::putstatic: { // 与 getstatic 类似，但需要把栈顶值写入
            u16 idx = code[frame.pc++] << 8 | code[frame.pc++];
            auto value = frame.stack.back();
            frame.stack.pop_back();
            auto& cp = frame.owner->cf->cp;
            auto& field = cp.get<CpFieldMethod>(idx);
            std::string cls_name = cp.get<CpUtf8>(cp.get<CpClass>(field.class_index).name_index).
                                      str;
            rt::JavaClass* target_cls = loader.load_class(cls_name);
            auto& nt = cp.get<CpNameAndType>(field.nt_index);
            std::string f_name = cp.get<CpUtf8>(nt.name_index).str;
            for (auto& f : target_cls->fields) {
                if (target_cls->cf->cp.get<CpUtf8>(f.cp_name).str == f_name && f.is_static) {
                    target_cls->statics[f.slot] = value;
                    break;
                }
            }
            break;
        }
        case Instruction::invokestatic: { // 调用静态方法
            u16 idx = code[frame.pc++] << 8 | code[frame.pc++];
            auto& cp = frame.owner->cf->cp;
            auto& method = cp.get<CpFieldMethod>(idx);
            std::string cls_name = cp.get<CpUtf8>(cp.get<CpClass>(method.class_index).name_index).
                                      str;
            rt::JavaClass* target_cls = loader.load_class(cls_name);

            // 方法名以及描述符
            auto nt = cp.get<CpNameAndType>(method.nt_index);
            std::string m_name = cp.get<CpUtf8>(nt.name_index).str;
            std::string m_desc = cp.get<CpUtf8>(nt.desc_index).str;

            // 在线性表中匹配方法
            for (auto& m : target_cls->methods) {
                if (target_cls->cf->cp.get<CpUtf8>(m.cp_name).str == m_name
                    && cp.get<CpUtf8>(m.cp_desc).str == m_desc) {
                    std::vector<rt::Slot> args;
                    for (int i = static_cast<int>(m_desc.find(')')) - 1; i >= 1; --i) {
                        if (m_desc[i] == 'I') {
                            args.push_back(frame.stack.back());
                            frame.stack.pop_back();
                        } else if (m_desc[i] == 'L') {
                            while (m_desc[i] != ';'&& i >=0) {
                                --i;
                            }
                        }
                    }
                    std::reverse(args.begin(), args.end());
                    invoke(target_cls, m, args);
                    break;
                }
            }
        }
        case Instruction::invokevirtual: {
            u16 idx = code[frame.pc++] << 8 | code[frame.pc++];
            auto& cp = frame.owner->cf->cp;
            auto& method_ref = cp.get<CpFieldMethod>(idx);
            // 方法归属类名
            std::string cls_name = cp.get<CpUtf8>(
                cp.get<CpClass>(method_ref.class_index).name_index).str;

            auto& nt = cp.get<CpNameAndType>(method_ref.nt_index);
            std::string m_name = cp.get<CpUtf8>(nt.name_index).str;
            std::string m_desc = cp.get<CpUtf8>(nt.desc_index).str;

            std::vector<rt::Slot> args;
            for (int i = static_cast<int>(m_desc.find(')')) - 1; i >= 0; --i) {
                if (m_desc[i] == 'I') {
                    args.push_back(frame.stack.back());
                    frame.stack.pop_back();
                } else if (m_desc[i] == 'L') {
                    args.push_back(frame.stack.back());
                    frame.stack.pop_back();
                    while (m_desc[i] != ';' && i >=0) {
                        --i;
                    }
                } else {}
            }

            rt::Slot ref = frame.stack.back();
            frame.stack.pop_back();

            args.push_back(ref);
            std::reverse(args.begin(), args.end());

            if (ref == 0) {
                throw std::runtime_error("NullPointerException");
            }

            auto* recv_obj = reinterpret_cast<rt::Oop>(ref);
            auto dyn_cls = recv_obj->clazz; // 动态类型

            // 从 dyn_cls 向上匹配方法
            rt::JavaClass* target_cls = nullptr;
            const rt::JavaClass::MethodRT* target_m = nullptr;

            for (auto* c = dyn_cls; c != nullptr;) {
                for (auto& mt : c->methods) {
                    if (c->cf->cp.get<CpUtf8>(mt.cp_name).str == m_name
                        && c->cf->cp.get<CpUtf8>(mt.cp_desc).str == m_desc) {
                        target_cls = c;
                        target_m = &mt;
                        break;
                    }
                }
                if (target_m)
                    break;
                c = c->super;
            }

            if (target_m == nullptr) {
                throw std::runtime_error(
                    "NoSuchMethodError: " + dyn_cls->name + "." + m_name + m_desc);
            }
            invoke(target_cls, *target_m, args);
            break;
        }
        case Instruction::ireturn: {           // 返回 int
            rt::Slot ret = frame.stack.back(); // 取出返回值
            frame.stack.pop_back();
            thread.pop();                     // 弹出当前栈
            thread.top->stack.push_back(ret); // 把返回值压入调用者栈帧
            break;
        }
        case Instruction::return_:
            thread.pop();
            break;
        case Instruction::nop: {
            break;
        }
        default:
            throw std::runtime_error(
                "unsupported instruction " + std::string(to_string(static_cast<Instruction>(op))));
        }
    }
}

}