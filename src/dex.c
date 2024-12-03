#include "dex.h"
#include "leb128.h"
#include <string.h>
#include "base64.h"

struct DexCode* get_method_off(struct DexHeader* dex_mem, uint32_t method_id, uint32_t* method_size){
    struct DexMethodId* method_id_item = (struct DexMethodId*)((uint64_t)dex_mem + dex_mem->methodIdsOff);
    method_id_item = method_id_item + method_id;

    // get_string(dex_mem, method_id_item->nameIdx, NULL);

    // method class type
    struct DexClassDef* class_def = (struct DexClassDef*)((uint64_t)dex_mem + dex_mem->classDefsOff);
    class_def = class_def + method_id_item->classIdx;
    struct DexTypeId* type_item = get_type(dex_mem, class_def->classIdx);
    get_string(dex_mem, type_item->descriptorIdx, NULL);

    uint8_t *class_data_out;
    struct ClassDataItem* class_data_item = get_class_item(dex_mem, class_def->classDataOff, &class_data_out);
    // DEBUG("%d %d\n", class_data_item->direct_methods_size, class_data_item->virtual_methods_size);

    // uint8_t* method_idx_diff_start = (uint8_t*)((uint64_t)class_data_out + class_data_item->static_fields_size + class_data_item->instance_fields_size);
    
    
    
    return NULL;
}


char* get_string(struct DexHeader* dex_mem, uint32_t string_idx, uint32_t* str_len){
    if(string_idx < dex_mem->stringIdsSize){
        struct DexStringId* string_id_item = (struct DexStringId*)((uint64_t)dex_mem + dex_mem->stringIdsOff);
        uint8_t* data = (uint8_t*)((uint64_t)dex_mem + (string_id_item + string_idx)->stringDataOff);
        uint32_t data_len = DecodeUnsignedLeb128(&data);
        char* data_str = (char*)malloc(data_len);
        if(str_len != NULL){
            *str_len = data_len;
        }
        memcpy(data_str, data, data_len);
        // DEBUG("strlen: %d, data: %s\n", data_len, data_str);
        return data_str;
    } else {
        return NULL;
    }
}

struct DexTypeId* get_type(struct DexHeader* dex_mem, uint32_t type_idx) {
    if(type_idx < dex_mem->typeIdsSize){
        struct DexTypeId* type_item  = (struct DexTypeItem*)((uint64_t)dex_mem + dex_mem->typeIdsOff);
        type_item += type_idx;
        return type_item;
    } else {
        return NULL;
    }
    
}

struct ClassDataItem* get_class_item(struct DexHeader* dex_mem, uint32_t class_data_off, uint8_t **class_data_out){
    struct ClassDataItem* class_data_item = (struct ClassDataItem*)malloc(sizeof(struct ClassDataItem));
    uint8_t* class_data_item_mem = (uint8_t*)((uint64_t)dex_mem + class_data_off);
    class_data_item->static_fields_size =  DecodeUnsignedLeb128(&class_data_item_mem);
    class_data_item->instance_fields_size =  DecodeUnsignedLeb128(&class_data_item_mem);
    class_data_item->direct_methods_size =  DecodeUnsignedLeb128(&class_data_item_mem);
    class_data_item->virtual_methods_size =  DecodeUnsignedLeb128(&class_data_item_mem);
    if(class_data_out != NULL){
        *class_data_out = class_data_item_mem;
    }
    return class_data_item;
}

struct DexCode* get_code_item(struct DexHeader* dex_mem, uint32_t code_off, uint8_t** code_out){
    struct DexCode* code_item = (struct DexCode*)((uint64_t)dex_mem + code_off);
    if(code_out != NULL){
        *code_out = &code_item->insns;
    }
    // DEBUG("reg: %d, in: %d, out: %d, insns: 0x%x\n", code_item->registersSize, code_item->insSize, code_item->outsSize, code_item->insnsSize);
    return code_item;
}

struct EncodeField* get_field_x(uint8_t** fields_start, uint32_t* old_diff){
    struct EncodeField* field = (struct EncodeField*)malloc(sizeof(struct EncodeField));
    field->field_idx_diff = DecodeUnsignedLeb128(fields_start) + *old_diff;
    field->access_flags =DecodeUnsignedLeb128(fields_start);
    *old_diff = field->field_idx_diff;
    // DEBUG("field idx diff: %d, flag: 0x%x\n", field->field_idx_diff, field->access_flags);
    return field;
}

struct EncodeMethod* get_method_x(uint8_t** methods_start, uint32_t* old_diff){
    struct EncodeMethod* method = (struct EncodeMethod*)malloc(sizeof(struct EncodeMethod));
    method->method_idx = DecodeUnsignedLeb128(methods_start) + *old_diff;
    method->access_flags = DecodeUnsignedLeb128(methods_start);
    method->code_off = DecodeUnsignedLeb128(methods_start);
    *old_diff = method->method_idx;
    // DEBUG("method idx: %d, flag: 0x%x, code: 0x%lx\n", method->method_idx, method->access_flags, method->code_off);
    return method;
}

struct DexCode* find_method_from_class(struct DexHeader* dex_mem, struct ClassDataItem* class_data_item, uint32_t method_idx, uint8_t **class_data){
    uint32_t old_diff = 0;
    // uint8_t *class_data_out = NULL;
    for (uint32_t i = 0; i < class_data_item->static_fields_size; i++)
    {
        struct EncodeField* field = get_field_x(class_data, &old_diff);
        free(field);
    }
    old_diff = 0;
    for (uint32_t i = 0; i < class_data_item->instance_fields_size; i++)
    {
        struct EncodeField* field = get_field_x(class_data, &old_diff);
        free(field);
    }
    old_diff = 0;
    for (uint32_t i = 0; i < class_data_item->direct_methods_size; i++)
    {
        struct EncodeMethod* method = get_method_x(class_data, &old_diff);
        if(method->method_idx == method_idx){
            uint8_t* code_out;
            struct DexCode * code_item = get_code_item(dex_mem, method->code_off, &code_out);
            DEBUG("method idx: %d, flag: 0x%x, code: 0x%x\n", method->method_idx, method->access_flags, method->code_off);
            if(method->code_off > 0){
                // hex_dump(code_out, 0x60);
                return code_item;
            }
        }
        free(method);
    }
    old_diff = 0;
    for (uint32_t i = 0; i < class_data_item->virtual_methods_size; i++)
    {
        struct EncodeMethod* method = get_method_x(class_data, &old_diff);
        if(method->method_idx == method_idx){
            uint8_t* code_out;
            struct DexCode * code_item = get_code_item(dex_mem, method->code_off, &code_out);
            DEBUG("method idx: %d, flag: 0x%x, code: 0x%x\n", method->method_idx, method->access_flags, method->code_off);
            if(method->code_off > 0){
                // hex_dump(code_out, 0x60);
                return code_item;
            }
        }
        free(method);
    }
    return NULL;
}

struct DexCode* find_method(struct DexHeader* dex_mem, uint32_t method_idx) {
    for (uint32_t i = 0; i < dex_mem->classDefsSize; i++)
    {
        struct DexClassDef* class_def = (struct DexClassDef*)((uint64_t)dex_mem + dex_mem->classDefsOff) + i;
        if(class_def->classDataOff == 0) 
            continue;
        uint8_t *class_data_out;
        struct ClassDataItem* class_data_item = get_class_item(dex_mem, class_def->classDataOff, &class_data_out);
        struct DexCode* dex_code = find_method_from_class(dex_mem, class_data_item, method_idx, &class_data_out);
        if(dex_code != NULL){
            // if(dex_code->insns[0] == 0)
            return dex_code;
        }

    }
    return NULL;
}