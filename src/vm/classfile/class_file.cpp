//
// Created by 峰峰的🍎 on 2025/6/25.
//
#include <iostream>

#include "class_file.h"

#include <map>

#include "vm/common/utils.h"
using namespace jvm::util;

namespace jvm::classfile {
static void read_constant_pool(std::istream& in, ConstantPool& cp)
{
    const u16 count = read_word(in);
    cp.pool.resize(count);
    cp.tags.resize(count);

    for (u16 idx = 1; idx < count; ++idx) {
        CpTag tag = static_cast<CpTag>(read_byte(in));
        cp.tags[idx] = tag;
        switch (tag) {
        case CpTag::Utf8: {
            u16 len = read_word(in);
            std::string s(len, '\0');
            in.read(s.data(), len);
            cp.pool[idx] = CpUtf8{std::move(s)};
            break;
        }
        case CpTag::Integer:
            cp.pool[idx] = CpInt{static_cast<i32>(read_dword(in))};
            break;
        case CpTag::Float: {
            u32 bits = read_dword(in);
            float f;
            std::memcpy(&f, &bits, 4);
            cp.pool[idx] = CpFloat{f};
            break;
        }
        case CpTag::Long: {
            i64 v = (static_cast<i64>(read_dword(in)) << 32) | read_dword(in);
            cp.pool[idx] = CpLong{v};
            ++idx; // Long / Double 占两个槽位
            break;
        }
        case CpTag::Double: {
            u64 bits = static_cast<u64>(read_dword(in)) << 32 | read_dword(in);
            double d;
            std::memcpy(&d, &bits, 8);
            cp.pool[idx] = CpDouble{d};
            ++idx;
            break;
        }
        case CpTag::Class:
            cp.pool[idx] = CpClass{read_word(in)};
            break;
        case CpTag::String:
            cp.pool[idx] = CpString{read_word(in)};
            break;
        case CpTag::FiledRef:
        case CpTag::MethodRef:
        case CpTag::InterfaceMethodRef:
            cp.pool[idx] = CpFieldMethod{read_word(in), read_word(in)};
            break;
        case CpTag::NameAndType:
            cp.pool[idx] = CpNameAndType{read_word(in), read_word(in)};
            break;
        case CpTag::MethodHandle:
            cp.pool[idx] = CpMethodHandle{read_byte(in), read_word(in)};
            break;
        case CpTag::MethodType:
            cp.pool[idx] = CpMethodType{read_word(in)};
            break;
        case CpTag::InvokeDynamic:
            cp.pool[idx] = CpInvokeDynamic{read_word(in), read_word(in)};
            break;
        default:
            die("unknown cp tag");
        }
    }
}

static Attribute read_attribute(std::istream& in, const ConstantPool& cp)
{
    u16 name_index = read_word(in);
    u32 length = read_dword(in);
    auto name = cp.get<CpUtf8>(name_index).str;

    if (name == "Code") {
        CodeAttr ca;
        ca.max_stack = read_word(in);
        ca.max_locals = read_word(in);
        u32 code_length = read_dword(in);
        ca.code.resize(code_length);
        in.read(reinterpret_cast<char*>(ca.code.data()), code_length);

        // 跳过异常表和内部属性
        u16 ex_cnt = read_word(in);
        in.ignore(ex_cnt * 8);
        u16 sub_attr_cnt = read_word(in);
        for (u16 i = 0; i < sub_attr_cnt; ++i) {
            in.ignore(2);
            u32 skip_len = read_dword(in);
            in.ignore(skip_len);
        }
        return ca;
    }

    if (name == "ConstantValue") {
        ConstantValueAttr cva{read_word(in)};
        return cva;
    }

    if (name == "SourceFile") {
        SourceFileAttr sfa{read_word(in)};
        return sfa;
    }

    // 其余属性直接复制原始字节
    RawAttr ra;
    ra.info.resize(length);
    in.read(reinterpret_cast<char*>(ra.info.data()), length);
    return ra;
}

static MemberInfo read_member(std::istream& in, ConstantPool& cp)
{
    MemberInfo member;
    member.access_flags = read_word(in);
    member.name_index = read_word(in);
    member.desc_index = read_word(in);
    u16 attr_cnt = read_word(in);
    member.attributes.reserve(attr_cnt);
    for (u16 i = 0; i < attr_cnt; ++i) {
        member.attributes.emplace_back(read_attribute(in, cp));
    }
    return member;
}

std::ifstream get_class_file_stream(const std::string& classpath, const std::string& class_name)
{
    std::string full_path = classpath + "/" + class_name + ".class";
    std::ifstream in(full_path, std::ios::binary);
    if (!in) {
        full_path = classpath + "/rt/" + class_name + ".class";
        in  = std::ifstream(full_path, std::ios::binary);
        if (!in) {
            die("no such class file");
        }
    }
    return in;
}

ClassFile parse_class_file(std::ifstream&& in)
{
    u32 r = read_dword(in);
    if (r != 0xCAFEBABE)
        die("not a class file");

    ClassFile cf{};
    cf.minor = read_word(in);
    cf.major = read_word(in);

    // 解析常量池
    read_constant_pool(in, cf.cp);

    cf.access_flags = read_word(in);
    cf.this_class = read_word(in);
    cf.super_class = read_word(in);
    cf.interfaces = read_word_vector(in);

    // 读取类成员变量
    u16 field_cnt = read_word(in);
    cf.fields.reserve(field_cnt);
    for (u16 i = 0; i < field_cnt; ++i) {
        cf.fields.emplace_back(read_member(in, cf.cp));
    }

    // 读取类方法
    u16 method_cnt = read_word(in);
    cf.methods.reserve(method_cnt);
    for (u16 i = 0; i < method_cnt; ++i) {
        cf.methods.emplace_back(read_member(in, cf.cp));
    }

    // 读取其他类属性
    u16 attr_cnt = read_word(in);
    cf.attributes.reserve(attr_cnt);
    for (u16 i = 0; i < attr_cnt; ++i) {
        cf.attributes.emplace_back(read_attribute(in, cf.cp));
    }
    return cf;
}

void print_summery(const ClassFile& cf)
{
    const auto& cp = cf.cp;
    auto utf8 = [&](u16 i) { return cp.get<CpUtf8>(i).str; };
    auto cls_nm = [&](u16 i) { return utf8(cp.get<CpClass>(i).name_index); };

    std::cout << "\n======== Class Summary ========\n";
    std::cout << "Version  : " << cf.major << '.' << cf.minor << '\n';
    std::cout << "This     : " << cls_nm(cf.this_class) << '\n';
    std::cout << "Super    : " << cls_nm(cf.super_class) << '\n';

    std::cout << "Interfaces(" << cf.interfaces.size() << "): ";
    for (auto idx : cf.interfaces)
        std::cout << cls_nm(idx) << ' ';
    std::cout << '\n';

    std::cout << "Fields(" << cf.fields.size() << ")\n";
    for (auto& f : cf.fields)
        std::cout << "  " << utf8(f.name_index)
            << ' ' << utf8(f.desc_index) << '\n';

    std::cout << "Methods(" << cf.methods.size() << ")\n";
    for (auto& m : cf.methods) {
        std::cout << "  " << utf8(m.name_index)
            << ' ' << utf8(m.desc_index);
        for (auto& a : m.attributes) {
            if (std::holds_alternative<CodeAttr>(a)) {
                auto& c = std::get<CodeAttr>(a);
                std::cout << "  [Code len=" << c.code.size() << ']';
            }
        }
        std::cout << '\n';
    }

    std::cout << "Attributes(" << cf.attributes.size() << ")\n";
    for (auto& a : cf.attributes) {
        if (std::holds_alternative<ConstantValueAttr>(a)) {
            auto& c = std::get<ConstantValueAttr>(a);
            std::cout << "  ConstantValueAttr\n";
            std::cout << "      name    " << utf8(c.value_index) << "\n";
        }
        if (std::holds_alternative<SourceFileAttr>(a)) {
            auto& c = std::get<SourceFileAttr>(a);
            std::cout << "  SourceFileAttr\n";
            std::cout << "      filename    " << utf8(c.sourcefile_index) << "\n";
        }
        if (std::holds_alternative<RawAttr>(a)) {
            auto& c = std::get<RawAttr>(a);
            std::cout << "  RawAttr.info.size   " << c.info.size() << "\n";
        }
        if (std::holds_alternative<CodeAttr>(a)) {
            auto& c = std::get<CodeAttr>(a);
            std::cout << "  CodeAttr  [max_stack=" << c.max_stack << ", max_locals=" << c.max_locals
                << "<Code len=" << c.code.size() << ">]\n";
        }
    }
    std::cout << "======== Class Summary ========\n\n";
}
}