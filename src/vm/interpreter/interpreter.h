//
// Created by 峰峰的🍎 on 2025/6/26.
//

#ifndef INTERPRETER_H
#define INTERPRETER_H
#include "vm/loader/class_loader.h"
#include "vm/runtime/thread.h"

namespace jvm::interp {

class Interpreter {
public:
    explicit Interpreter(loader::ClassLoader& l) : loader(l) {}

    // 以 <MainClass>.main(String[]) 为入口启动解释器
    void run_main(const std::string& cls);

private:
    loader::ClassLoader& loader;
    rt::Thread thread;

    // 解释循环
    void exec_loop();
    // 方法调用
    void invoke(rt::JavaClass* owner, const rt::JavaClass::MethodRT& m,
                std::vector<rt::Slot> args);
};

}

#endif //INTERPRETER_H