//
// Created by 峰峰的🍎 on 2025/6/25.
//

#include <iostream>
#include "vm/classfile/class_file.h"
#include "vm/loader/class_loader.h"
#include "vm/native/native_registry.h"
#include "vm/runtime/slots.h"
#include "vm/runtime/stack_frame.h"
#include "vm/runtime/java_object.h"

using namespace std;
using namespace jvm::classfile;
using namespace jvm::rt;
using namespace jvm::loader;

// [[maybe_unused]] static struct Init {
//     Init()
//     {
//         jvm::native::register_natives(
//             "java/io/PrintStream",
//             "println",
//             "(Ljava/lang/String;)V",
//             [](jvm::rt::StackFrame& frame) {
//                 using namespace jvm::rt;
//                 Slot ref = frame.stack.back();
//                 frame.stack.pop_back();
//                 const JavaObject* str_obj = reinterpret_cast<JavaObject*>(ref);
//                 // 假定该对象的 fields 中的第0个槽位就是一个 std::string 指针
//                 std::cout << reinterpret_cast<std::string*>(str_obj->fields[0])->c_str() << '\n';
//             });
//     }
// } auto_register_println; // 静态初始化变量自动执行构造函

int main(int argc, char* argv[])
{
    cout << "Hello, mini-jvm!" << endl;
    if (argc != 2) {
        std::cerr << "Usage: " << argv[0] << " <*.class>\n" << std::endl;
        return 1;
    }

    try {
        // const ClassFile cf = parse_class_file(argv[1]);
        // print_summery(cf);
        ClassLoader class_loader("../example");
        auto load_class = class_loader.load_class("Hello");
        cout << "load_class: " << load_class << endl;
    } catch (const std::exception& e) {
        std::cerr << "there's error: " << e.what() << std::endl;
        return 2;
    }
    return 0;
}