#include "VmStructs.hpp"

void MainThread() {
    JavaVM::alloc_console(); //creates a console window
    JavaVM* vm = new JavaVM(); //creates a new vm object

    //retrieve classes//
    printf("--- CLASS ---\n");
    auto classes = vm->get_classes();
    auto first_class = vm->get_class("tests/AllStaticShits");
    std::cout << "class count: " << classes.size() << std::endl;

    //retrieve simple class infos//
    std::cout << first_class->get_name().c_str() << std::endl;
    std::cout << first_class->get_super_klass()->get_name().c_str() << std::endl;
    std::cout << first_class->get_access_flags() << std::endl;
    
    //constant pool tests//
    printf("--- CONSTANT POOL ---\n");
    auto cp = first_class->get_constants();

    for (int i = 0; i < cp->get_length(); i++) {
        if (cp->get_tags()->at(i) == constant_pool::constant_tag::CONSTANT_Utf8) { //get all utf8 constants
            std::cout << i << std::endl;
            std::cout << cp->symbol_at(i)->get_value().c_str() << std::endl;
        }
    }

    //fields tests//
    printf("--- FIELDS ---\n");
    std::cout << first_class->get_field_count() << std::endl;
    auto fields = first_class->get_fields();

    for (auto& f : fields) {
        std::cout << (f->is_static() ? "static " : "") << f->name.c_str() << " " << f->signature.c_str() << " (0x" << std::hex << f->offset << std::dec << ")" << std::endl;
    }

    //field get tests//
    std::cout << first_class->get_field("SIZE")->get_static_value<int>() << std::endl; //get a static field value
    std::cout << std::hex << first_class->get_field("testStr")->get_static_compressed_object() << std::dec << std::endl; //get a static field object
    
    vm_obj instance = first_class->get_field("instance")->get_static_compressed_object();
    std::cout << std::hex << first_class->get_field("otherTest")->get_compressed_object(instance) << std::dec << std::endl;

    //methods tests//
    printf("--- METHODS ---\n");
}

BOOL APIENTRY DllMain( HMODULE hModule,
                       DWORD  ul_reason_for_call,
                       LPVOID lpReserved
                     )
{
    if (ul_reason_for_call == DLL_PROCESS_ATTACH)
        CreateThread(NULL, NULL, reinterpret_cast<LPTHREAD_START_ROUTINE>(MainThread), NULL, NULL, NULL);
    return TRUE;
}

