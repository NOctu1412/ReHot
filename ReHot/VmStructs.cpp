#include "VmStructs.hpp"

JavaVM::JavaVM() {
    this->jvm_module = GetModuleHandleA("jvm.dll");

    //read structs//
    uintptr_t structs_entry = *reinterpret_cast<uintptr_t*>(GetProcAddress(jvm_module, "gHotSpotVMStructs"));
    uintptr_t structs_type_name = *reinterpret_cast<uintptr_t*>(GetProcAddress(jvm_module, "gHotSpotVMStructEntryTypeNameOffset"));
    uintptr_t structs_field_name = *reinterpret_cast<uintptr_t*>(GetProcAddress(jvm_module, "gHotSpotVMStructEntryFieldNameOffset"));
    uintptr_t structs_type_string = *reinterpret_cast<uintptr_t*>(GetProcAddress(jvm_module, "gHotSpotVMStructEntryTypeStringOffset"));
    uintptr_t structs_static = *reinterpret_cast<uintptr_t*>(GetProcAddress(jvm_module, "gHotSpotVMStructEntryIsStaticOffset"));
    uintptr_t structs_offset = *reinterpret_cast<uintptr_t*>(GetProcAddress(jvm_module, "gHotSpotVMStructEntryOffsetOffset"));
    uintptr_t structs_address = *reinterpret_cast<uintptr_t*>(GetProcAddress(jvm_module, "gHotSpotVMStructEntryAddressOffset"));
    uintptr_t structs_array_stride = *reinterpret_cast<uintptr_t*>(GetProcAddress(jvm_module, "gHotSpotVMStructEntryArrayStride"));

    for (;; structs_entry += structs_array_stride) {
        std::string type_name = vm_utils::read_string(*reinterpret_cast<uintptr_t*>(structs_entry + structs_type_name));
        if (type_name.empty()) break;
        std::string field_name = vm_utils::read_string(*reinterpret_cast<uintptr_t*>(structs_entry + structs_field_name));
        std::string type_string = vm_utils::read_string(*reinterpret_cast<uintptr_t*>(structs_entry + structs_type_string));
        bool is_static = *reinterpret_cast<bool*>(structs_entry + structs_static);
        long offset = *reinterpret_cast<long*>(structs_entry + (is_static ? structs_address : structs_offset));

        vm_field* field = new vm_field(type_name, field_name, type_string, is_static, offset);
        if (vm_structs.find(type_name) == vm_structs.end()) {
            std::vector<vm_field*> vec;
            vec.push_back(field);
            vm_structs.insert(std::make_pair(type_name, vec));
        } else {
            vm_structs.at(type_name).push_back(field);
        }
    }

    //read types//
    uintptr_t types_entry = *reinterpret_cast<uintptr_t*>(GetProcAddress(jvm_module, "gHotSpotVMTypes"));
    uintptr_t types_type_name = *reinterpret_cast<uintptr_t*>(GetProcAddress(jvm_module, "gHotSpotVMTypeEntryTypeNameOffset"));
    uintptr_t types_super_class_name = *reinterpret_cast<uintptr_t*>(GetProcAddress(jvm_module, "gHotSpotVMTypeEntrySuperclassNameOffset"));
    uintptr_t types_is_oop = *reinterpret_cast<uintptr_t*>(GetProcAddress(jvm_module, "gHotSpotVMTypeEntryIsOopTypeOffset"));
    uintptr_t types_is_int = *reinterpret_cast<uintptr_t*>(GetProcAddress(jvm_module, "gHotSpotVMTypeEntryIsIntegerTypeOffset"));
    uintptr_t types_is_unsigned = *reinterpret_cast<uintptr_t*>(GetProcAddress(jvm_module, "gHotSpotVMTypeEntryIsUnsignedOffset"));
    uintptr_t types_size = *reinterpret_cast<uintptr_t*>(GetProcAddress(jvm_module, "gHotSpotVMTypeEntrySizeOffset"));
    uintptr_t types_array_stride = *reinterpret_cast<uintptr_t*>(GetProcAddress(jvm_module, "gHotSpotVMTypeEntryArrayStride"));

    for (;; types_entry += types_array_stride) {
        std::string type_name = vm_utils::read_string(*reinterpret_cast<uintptr_t*>(types_entry + types_type_name));
        if (type_name.empty()) break;
        std::string type_super_class = vm_utils::read_string(*reinterpret_cast<uintptr_t*>(types_entry + types_super_class_name));
        bool is_oop = *reinterpret_cast<bool*>(types_entry + types_is_oop);
        bool is_int = *reinterpret_cast<bool*>(types_entry + types_is_int);
        bool is_unsigned = *reinterpret_cast<bool*>(types_entry + types_is_unsigned);
        int size = *reinterpret_cast<int*>(types_entry + types_size);
        std::vector<vm_field*> fields;
        for (auto& s : vm_structs) {
            if (s.first == type_name) {
                fields = s.second;
                break;
            }
        }
        vm_types.insert(std::make_pair(type_name, new vm_type(type_name, type_super_class, is_oop, is_int, is_unsigned, size, fields)));
    }

    //read int constants//
    uintptr_t ic_entry = *reinterpret_cast<uintptr_t*>(GetProcAddress(jvm_module, "gHotSpotVMIntConstants"));
    uintptr_t ic_name = *reinterpret_cast<uintptr_t*>(GetProcAddress(jvm_module, "gHotSpotVMIntConstantEntryNameOffset"));
    uintptr_t ic_value = *reinterpret_cast<uintptr_t*>(GetProcAddress(jvm_module, "gHotSpotVMIntConstantEntryValueOffset"));
    uintptr_t ic_array_stride = *reinterpret_cast<uintptr_t*>(GetProcAddress(jvm_module, "gHotSpotVMIntConstantEntryArrayStride"));

    for (;; ic_entry += ic_array_stride) {
        std::string name = vm_utils::read_string(*reinterpret_cast<uintptr_t*>(ic_entry + ic_name));
        if (name.empty()) break;
        int value = *reinterpret_cast<int*>(ic_entry + ic_value);
        vm_int_constants.insert(std::make_pair(name, value));
    }

    //read long constants//
    uintptr_t lc_entry = *reinterpret_cast<uintptr_t*>(GetProcAddress(jvm_module, "gHotSpotVMLongConstants"));
    uintptr_t lc_name = *reinterpret_cast<uintptr_t*>(GetProcAddress(jvm_module, "gHotSpotVMLongConstantEntryNameOffset"));
    uintptr_t lc_value = *reinterpret_cast<uintptr_t*>(GetProcAddress(jvm_module, "gHotSpotVMLongConstantEntryValueOffset"));
    uintptr_t lc_array_stride = *reinterpret_cast<uintptr_t*>(GetProcAddress(jvm_module, "gHotSpotVMLongConstantEntryArrayStride"));

    for (;; lc_entry += lc_array_stride) {
        std::string name = vm_utils::read_string(*reinterpret_cast<uintptr_t*>(lc_entry + lc_name));
        if (name.empty()) break;
        long value = *reinterpret_cast<long*>(lc_entry + lc_value);
        vm_long_constants.insert(std::make_pair(name, value));
    }

    //read field tags//
    field_tags::access_flags_offset = get_int_constant("FieldInfo::access_flags_offset");
    field_tags::name_offset = get_int_constant("FieldInfo::name_index_offset");
    field_tags::signature_offset = get_int_constant("FieldInfo::signature_index_offset");
    field_tags::initval_offset = get_int_constant("FieldInfo::initval_index_offset");
    field_tags::low_packed_offset = get_int_constant("FieldInfo::low_packed_offset");
    field_tags::high_packed_offset = get_int_constant("FieldInfo::high_packed_offset");
    field_tags::field_slots = get_int_constant("FieldInfo::field_slots");

    //finished//
    printf("[*] %zu types were read !\n", vm_types.size());
    printf("[*] %zu int constants were read !\n", vm_int_constants.size());
    printf("[*] %zu long constants were read !\n", vm_long_constants.size());
}

void JavaVM::alloc_console() {
    AllocConsole();
    FILE* in;
    FILE* out;
    freopen_s(&in, "conin$", "r", stdin);
    freopen_s(&out, "conout$", "w", stdout);
    freopen_s(&out, "conout$", "w", stderr);
}

std::string vm_utils::read_string(uintptr_t addr){
    if (!addr) return std::string();
    return std::string(reinterpret_cast<char*>(addr));
}

std::string klass::get_name() {
    return (new symbol(vm, *reinterpret_cast<uintptr_t*>((uintptr_t)(address + vm->get_type("Klass")->get_field("_name")->offset))))->get_value();
}

klass* klass::get_super_klass() {
    return new klass(vm, *reinterpret_cast<uintptr_t*>((uintptr_t)(address + vm->get_type("Klass")->get_field("_super")->offset)));
}

klass* klass::get_sub_klass(){
    return new klass(vm, *reinterpret_cast<uintptr_t*>((uintptr_t)(address + vm->get_type("Klass")->get_field("_subklass")->offset)));
}

int klass::get_access_flags() {
    return *reinterpret_cast<int*>((uintptr_t)(address + vm->get_type("Klass")->get_field("_access_flags")->offset));
}

short symbol::get_length() {
    return *reinterpret_cast<short*>((uintptr_t)(address + vm->get_type("Symbol")->get_field("_length")->offset));
}

std::string symbol::get_value() {
    std::string result = reinterpret_cast<char*>((uintptr_t)(address + vm->get_type("Symbol")->get_field("_body")->offset));
    result.resize(get_length());
    return result;
}

constant_pool* instance_klass::get_constants() {
    return new constant_pool(vm, *reinterpret_cast<uintptr_t*>((uintptr_t)(address + vm->get_type("InstanceKlass")->get_field("_constants")->offset)));
}

oop_desc* field::get_oop_desc() {
    return *reinterpret_cast<oop_desc**>((uintptr_t)(owner->address + owner->vm->get_type("Klass")->get_field("_java_mirror")->offset));
}

std::vector<field*> instance_klass::get_fields() {
    std::vector<field*> result;
    auto fields = *reinterpret_cast<vm_array<uint16_t>**>((uintptr_t)(address + vm->get_type("InstanceKlass")->get_field("_fields")->offset));
    for (int i = 0; i < get_field_count(); i++) {
        auto tags = (((class field_tags*)fields->adr_at(i * JavaVM::field_tags::field_slots)))->tags;

        field* f = new field();
        f->owner = this;
        f->access_flags = tags[JavaVM::field_tags::access_flags_offset];
        f->name = get_constants()->symbol_at(tags[JavaVM::field_tags::name_offset])->get_value();
        f->signature = get_constants()->symbol_at(tags[JavaVM::field_tags::signature_offset])->get_value();
        short high_packed = tags[JavaVM::field_tags::high_packed_offset];
        short low_packed = tags[JavaVM::field_tags::low_packed_offset];
        f->offset = ((int)((uint32_t)high_packed << 16) | (uint32_t)low_packed) >> 2;
        
        result.push_back(f);
        
    }
    return result;
}

field* instance_klass::get_field(std::string name) {
    auto fields = get_fields();
    for (auto& f : fields) {
        if (f->name == name)
            return f;
    }
    return nullptr;
}

uint16_t instance_klass::get_field_count() {
    return *reinterpret_cast<uint16_t*>((uintptr_t)(address + vm->get_type("InstanceKlass")->get_field("_java_fields_count")->offset));
}

int constant_pool::get_length() {
    return *reinterpret_cast<int*>((uintptr_t)(address + vm->get_type("ConstantPool")->get_field("_length")->offset));
}

vm_array<constant_pool::constant_tag>* constant_pool::get_tags() {
    return *reinterpret_cast<vm_array<constant_tag>**>((uintptr_t)(address + vm->get_type("ConstantPool")->get_field("_tags")->offset));
}

intptr_t* constant_pool::base() const {
    return (intptr_t*)(((char*)address) + vm->get_type("ConstantPool")->size);
}