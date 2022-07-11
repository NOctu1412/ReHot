#pragma once

#include <Windows.h>
#include <iostream>
#include <cstdio>
#include <string>
#include <vector>
#include <unordered_map>
#include <algorithm>

#define vm_obj uintptr_t

class vm_utils {
public:
	static std::string read_string(uintptr_t addr);
};

class vm_field {
public:
	std::string owner_type;
	std::string field_name;
	std::string type;
	bool is_static;
	long offset;

	vm_field(std::string owner, std::string name, std::string& t, bool is_static, long offset) : owner_type(owner), field_name(name), type(t), is_static(is_static), offset(offset) {}
};

class vm_type {
public:
	std::string name;
	std::string super_class;
	bool is_oop;
	bool is_int;
	long is_unsigned;
	int size;
	std::vector<vm_field*> fields;

	vm_type(std::string name, std::string super_class, bool is_oop, bool is_int, bool is_unsigned, int size, std::vector<vm_field*>& fields) : name(name), super_class(super_class), is_oop(is_oop), is_int(is_int), is_unsigned(is_unsigned), size(size), fields(fields) {}

	vm_field* get_field(std::string name) {
		for (auto& f : fields) {
			if (f->field_name == name)
				return f;
		}
		return nullptr;
	}
};

class vm_based_object {
public:
	class JavaVM* vm;
	uintptr_t address;

	vm_based_object(class JavaVM* vm, uintptr_t address) : vm(vm), address(address) {}
};

class oop_desc : public vm_based_object {
public:
	oop_desc(class JavaVM* vm, uintptr_t address) : vm_based_object(vm, address) {}

	inline void* field_base(int offset) const { return (void*)&((char*)this)[offset]; }

	template <class T> inline T* obj_field_addr(int offset) const { return (T*)field_base(offset); }

	inline byte* byte_field_addr(int offset)   const { return (byte*)field_base(offset); }
	inline char* char_field_addr(int offset)   const { return (char*)field_base(offset); }
	inline bool* bool_field_addr(int offset)   const { return (bool*)field_base(offset); }
	inline int* int_field_addr(int offset)    const { return (int*)field_base(offset); }
	inline short* short_field_addr(int offset)  const { return (short*)field_base(offset); }
	inline long* long_field_addr(int offset)   const { return (long*)field_base(offset); }
	inline float* float_field_addr(int offset)  const { return (float*)field_base(offset); }
	inline double* double_field_addr(int offset) const { return (double*)field_base(offset); }

	inline byte byte_field(int offset) const { return (byte)*byte_field_addr(offset); }
	inline void byte_field_put(int offset, byte contents) { *byte_field_addr(offset) = (int)contents; }

	inline boolean bool_field(int offset) const { return (boolean)*bool_field_addr(offset); }
	inline void bool_field_put(int offset, boolean contents) { *bool_field_addr(offset) = (((int)contents) & 1); }

	inline char char_field(int offset) const { return (char)*char_field_addr(offset); }
	inline void char_field_put(int offset, char contents) { *char_field_addr(offset) = (int)contents; }

	inline int int_field(int offset) const { return *int_field_addr(offset); }
	inline void int_field_put(int offset, int contents) { *int_field_addr(offset) = contents; }

	inline short short_field(int offset) const { return (short)*short_field_addr(offset); }
	inline void short_field_put(int offset, short contents) { *short_field_addr(offset) = (int)contents; }

	inline long long_field(int offset) const { return *long_field_addr(offset); }
	inline void long_field_put(int offset, long contents) { *long_field_addr(offset) = contents; }

	inline float float_field(int offset) const { return *float_field_addr(offset); }
	inline void float_field_put(int offset, float contents) { *float_field_addr(offset) = contents; }

	inline double double_field(int offset) const { return *double_field_addr(offset); }
	inline void double_field_put(int offset, double contents) { *double_field_addr(offset) = contents; }

	vm_obj obj_field(int offset) {
		return (vm_obj)(UIntToPtr(int_field(offset)));
	}

	vm_obj compressed_obj_field(int offset) {
		return ((vm_obj)(UIntToPtr(int_field(offset)))) << 3;
	}
};

template <typename T>
class vm_array {
public:
	int32_t length;
	T data[1];

	auto at(int i) const -> T {
		if (i >= 0 && i < length)
			return data[i];
		return (T)0;
	}

	auto is_empty() const -> bool {
		return length == 0;
	}

	T* adr_at(const int i) {
		if (i >= 0 && i < length)
			return &data[i];
		return nullptr;
	}

	T& operator[](int idx) {
		return at(idx);
	}
};

class symbol : public vm_based_object{
public:
	symbol(class JavaVM* vm, uintptr_t address) : vm_based_object(vm, address) {}

	short get_length();
	std::string get_value();
};

class klass : public vm_based_object {
public:
	klass(class JavaVM* vm, uintptr_t address) :vm_based_object(vm, address) {}

	std::string get_name();
	klass* get_super_klass();
	klass* get_sub_klass();
	int get_access_flags();
};

class constant_pool : public vm_based_object {
public:
	enum class constant_tag : uint8_t {
		CONSTANT_Invalid = 0,
		CONSTANT_Class = 7,
		CONSTANT_Fieldref = 9,
		CONSTANT_Methodref = 10,
		CONSTANT_InterfaceMethodref = 11,
		CONSTANT_String = 8,
		CONSTANT_Integer = 3,
		CONSTANT_Float = 4,
		CONSTANT_Long = 5,
		CONSTANT_Double = 6,
		CONSTANT_NameAndType = 12,
		CONSTANT_Utf8 = 1,
		CONSTANT_Unicode = 2,
		CONSTANT_MethodHandle = 15,
		CONSTANT_MethodType = 16,
		CONSTANT_InvokeDynamic = 18,
		CONSTANT_Unresolved_Class = 100,
		CONSTANT_ClassIndex = 101,
		CONSTANT_StringIndex = 102,
		CONSTANT_UnresolvedClassInError = 103,
		CONSTANT_MethodHandleInError = 104,
		CONSTANT_MethodTypeInError = 105
	};

	constant_pool(class JavaVM* vm, uintptr_t address) :vm_based_object(vm, address) {}

	int get_length();
	vm_array<constant_tag>* get_tags();

private:
	intptr_t* base() const;
	
public:
	symbol* symbol_at(int which) {
		return new symbol(vm, (uintptr_t)(*(void**)&base()[which]));
	}
};

class field_tags {
public:
	uint16_t tags[6];
};

class field {
public:
	class instance_klass* owner;

	int access_flags;
	std::string name;
	std::string signature;
	int offset;

	bool is_static() { return (access_flags & 0x8) != 0; }

private:
	oop_desc* get_oop_desc();

public:
	//STATIC//
	template <class T> T get_static_value() {
		return *get_oop_desc()->obj_field_addr<T>(offset);
	}

	template <class T> void set_static_value(T v) {
		*get_oop_desc()->obj_field_addr<T>(offset) = v;
	}

	vm_obj get_static_object() {
		return get_oop_desc()->obj_field(offset);
	}

	//sometimes the jvm use compressed oops (if get_static_object doesn't work, use this)
	vm_obj get_static_compressed_object() {
		return get_oop_desc()->compressed_obj_field(offset);
	}

	//NON-STATIC//
	template <class T> T get_value(vm_obj obj) {
		return *reinterpret_cast<T*>(obj + offset);
	}

	template <class T> void set_value(vm_obj obj, T v) {
		*reinterpret_cast<T*>(obj + offset) = v;
	}

	vm_obj get_object(vm_obj obj) {
		return (vm_obj)(ULongToPtr(*(int32_t*)(obj + obj)));
	}

	//sometimes the jvm use compressed oops (if get_object doesn't work, use this)
	vm_obj get_compressed_object(vm_obj instance) {
		return ((vm_obj)(ULongToPtr(*(int32_t*)(instance + offset)))) << 3;
	}
};

class instance_klass : public klass {
public:
	instance_klass(class JavaVM* vm, uintptr_t address) : klass(vm, address) {}

	constant_pool* get_constants();

	uint16_t get_field_count();
	std::vector<field*> get_fields();
	field* get_field(std::string name);
};

class vm_dictionary_entry {
public:
	DWORD64 hash;
	uintptr_t _next;
	uintptr_t klass;
	uintptr_t class_loader;

	vm_dictionary_entry* next() {
		return reinterpret_cast<vm_dictionary_entry*>(_next & 0xFFFFFFFFFFFFFFFE);
	}
};

class vm_dictionary {
public:
	int64_t table_size;
	vm_dictionary_entry** entries;
	PVOID free_list;
	PVOID first_free_entry;
	PVOID end_block;
	int entry_size;
	int entry_count;
};

class JavaVM {
public:
	HMODULE jvm_module = 0;

	std::unordered_map<std::string, std::vector<vm_field*>> vm_structs;
	std::unordered_map<std::string, vm_type*> vm_types;
	std::unordered_map<std::string, int> vm_int_constants;
	std::unordered_map<std::string, long> vm_long_constants;
public:
	struct field_tags {
		inline static int access_flags_offset;
		inline static int name_offset;
		inline static int signature_offset;
		inline static int initval_offset;
		inline static int low_packed_offset;
		inline static int high_packed_offset;
		inline static int field_slots;
	};

	JavaVM();

	static void alloc_console();

	vm_type* get_type(std::string name) {
		for (auto& t : vm_types) {
			if (t.first == name)
				return t.second;
		}
		return nullptr;
	}

	int get_int_constant(std::string name) {
		for (auto& c : vm_int_constants) {
			if (c.first == name)
				return c.second;
		}
		return -1;
	}

	long get_long_constant(std::string name) {
		for (auto& c : vm_long_constants) {
			if (c.first == name)
				return c.second;
		}
		return -1;
	}

	vm_dictionary* get_dictionary() {
		return *reinterpret_cast<vm_dictionary**>((uintptr_t)get_type("SystemDictionary")->get_field("_dictionary")->offset);
	}

	std::vector<instance_klass*> get_classes() {
		std::vector<instance_klass*> result;
		auto dict = get_dictionary();
		for (int64_t idx = 0; idx < dict->table_size; idx++) {
			for (auto entry = dict->entries[idx]; entry; entry = entry->next()) {
				if (!entry->klass)
					continue;
				result.push_back(new instance_klass(this, entry->klass));
			}
		}
		return result;
	}

	instance_klass* get_class(std::string name) {
		auto classes = get_classes();
		for (auto& c : classes) {
			if (c->get_name() == name)
				return c;
		}
		return nullptr;
	}
};