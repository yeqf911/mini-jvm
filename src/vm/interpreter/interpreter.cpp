//
// Created by 峰峰的🍎 on 2025/6/26.
//

#include "interpreter.h"

#include "vm/common/utils.h"
#include "vm/native/native_registry.h"
#include "vm/runtime/acc.h"

using namespace jvm;

namespace jvm::interp {

void Interpreter::run_main(const std::string& cls)
{
    auto clazz = loader.load_class(cls);
    const rt::JavaClass::MethodRT* main = nullptr;
    for (const auto& m : clazz->methods) {
        auto name = clazz->cf->cp.get<CpUtf8>(m.cp_name).str;
        auto desc = clazz->cf->cp.get<CpUtf8>(m.cp_desc).str;
        if (name == "main" && desc == "([Ljava/lang/String;)V"
            && (m.acc & rt::Acc::ACC_STATIC)) {
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

    // 处理 native 方法
    const auto m_name = owner->cf->cp.get<CpUtf8>(m.cp_name).str;
    const auto m_desc = owner->cf->cp.get<CpUtf8>(m.cp_desc).str;

    if (const auto nf = native::find(owner->name, m_name, m_desc)) {
        auto* frame = new StackFrame(0 , 0);
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
    auto* frame = new StackFrame(code->max_locals , code->max_stack);
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
    while (thread.top) {                     // 当线程还有栈帧
        rt::StackFrame& frame = *thread.top; // 取线程栈顶的栈帧
        auto& code = frame.code->code;       // 字节码向量
        const u8 op = code[frame.pc++];      // 取 opcode，程序计数器+1

        switch (op) {
        case 0x02:
        case 0x03:
        case 0x04:
        case 0x05: // iconst_-1..2
        case 0x06:
        case 0x07:
        case 0x08: { // iconst_3..5
            const int val = static_cast<i8>(op) - 3;
            frame.stack.push_back(val);
            break;
        }
        default:
            throw std::runtime_error("unsupported opcode " + std::to_string(op));
        }
    }
}

}