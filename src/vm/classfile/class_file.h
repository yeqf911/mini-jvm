//
// Created by 峰峰的🍎 on 2025/6/25.
//

#ifndef CLASS_READER_H
#define CLASS_READER_H
#include "constant_pool.h"
#include "member_info.h"

namespace jvm::classfile {

struct ClassFile {
    u16 minor{};
    u16 major{};
    ConstantPool cp;

    u16 access_flags{};
    u16 this_class{};
    u16 super_class{};

    std::vector<u16> interfaces;
    std::vector<MemberInfo> fields;
    std::vector<MemberInfo> methods;
    std::vector<Attribute> attributes;
};

std::ifstream get_class_file_stream(const std::string& classpath, const std::string& class_name);
ClassFile parse_class_file(std::ifstream&& in);
void print_summery(const ClassFile& cf);

}

#endif //CLASS_READER_H