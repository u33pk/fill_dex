/*
 * Copyright (C) 2008 The Android Open Source Project
 *
 * Licensed under the Apache License, Version 2.0 (the "License");
 * you may not use this file except in compliance with the License.
 * You may obtain a copy of the License at
 *
 *      http://www.apache.org/licenses/LICENSE-2.0
 *
 * Unless required by applicable law or agreed to in writing, software
 * distributed under the License is distributed on an "AS IS" BASIS,
 * WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
 * See the License for the specific language governing permissions and
 * limitations under the License.
 */

/*
 * Access .dex (Dalvik Executable Format) files.  The code here assumes that
 * the DEX file has been rewritten (byte-swapped, word-aligned) and that
 * the contents can be directly accessed as a collection of C arrays.  Please
 * see docs/dalvik/dex-format.html for a detailed description.
 *
 * The structure and field names were chosen to match those in the DEX spec.
 *
 * It's generally assumed that the DEX file will be stored in shared memory,
 * obviating the need to copy code and constant pool entries into newly
 * allocated storage.  Maintaining local pointers to items in the shared area
 * is valid and encouraged.
 *
 * All memory-mapped structures are 32-bit aligned unless otherwise noted.
 * 9.0.0_r3 dalvik/libdex/DexFile.h
 */

#ifndef LIBDEX_DEXFILE_H_
#define LIBDEX_DEXFILE_H_

#include <stdbool.h>
#include <stdint.h>
#include <stdio.h>
#include <assert.h>
#include "log.h"

/*
 * These match the definitions in the VM specification.
 */
typedef uint8_t u1;
typedef uint16_t u2;
typedef uint32_t u4;
typedef uint64_t u8;
typedef int8_t s1;
typedef int16_t s2;
typedef int32_t s4;
typedef int64_t s8;

/*
 * gcc-style inline management -- ensures we have a copy of all functions
 * in the library, so code that links against us will work whether or not
 * it was built with optimizations enabled.
 */
#ifndef _DEX_GEN_INLINES /* only defined by DexInlines.c */
#define DEX_INLINE extern __inline__
#else
#define DEX_INLINE
#endif

/* DEX file magic number */
#define DEX_MAGIC "dex\n"

/* The version for android N, encoded in 4 bytes of ASCII. This differentiates dex files that may
 * use default methods.
 */
#define DEX_MAGIC_VERS_37 "037\0"

/* The version for android O, encoded in 4 bytes of ASCII. This differentiates dex files that may
 * contain invoke-custom, invoke-polymorphic, call-sites, and method handles.
 */
#define DEX_MAGIC_VERS_38 "038\0"

/* The version for android P, encoded in 4 bytes of ASCII. This differentiates dex files that may
 * contain const-method-handle and const-proto.
 */
#define DEX_MAGIC_VERS_39 "039\0"

/* current version, encoded in 4 bytes of ASCII */
#define DEX_MAGIC_VERS "036\0"

/*
 * older but still-recognized version (corresponding to Android API
 * levels 13 and earlier
 */
#define DEX_MAGIC_VERS_API_13 "035\0"

/* same, but for optimized DEX header */
#define DEX_OPT_MAGIC "dey\n"
#define DEX_OPT_MAGIC_VERS "036\0"

#define DEX_DEP_MAGIC "deps"

/*
 * 160-bit SHA-1 digest.
 */
enum
{
    kSHA1DigestLen = 20,
    kSHA1DigestOutputLen = kSHA1DigestLen * 2 + 1
};

/* general constants */
enum
{
    kDexEndianConstant = 0x12345678, /* the endianness indicator */
    kDexNoIndex = 0xffffffff,        /* not a valid index value */
};

/*
 * Enumeration of all the primitive types.
 */
enum PrimitiveType
{
    PRIM_NOT = 0, /* value is a reference type, not a primitive type */
    PRIM_VOID = 1,
    PRIM_BOOLEAN = 2,
    PRIM_BYTE = 3,
    PRIM_SHORT = 4,
    PRIM_CHAR = 5,
    PRIM_INT = 6,
    PRIM_LONG = 7,
    PRIM_FLOAT = 8,
    PRIM_DOUBLE = 9,
};

/*
 * access flags and masks; the "standard" ones are all <= 0x4000
 *
 * Note: There are related declarations in vm/oo/Object.h in the ClassFlags
 * enum.
 */
enum
{
    ACC_PUBLIC = 0x00000001,       // class, field, method, ic
    ACC_PRIVATE = 0x00000002,      // field, method, ic
    ACC_PROTECTED = 0x00000004,    // field, method, ic
    ACC_STATIC = 0x00000008,       // field, method, ic
    ACC_FINAL = 0x00000010,        // class, field, method, ic
    ACC_SYNCHRONIZED = 0x00000020, // method (only allowed on natives)
    ACC_SUPER = 0x00000020,        // class (not used in Dalvik)
    ACC_VOLATILE = 0x00000040,     // field
    ACC_BRIDGE = 0x00000040,       // method (1.5)
    ACC_TRANSIENT = 0x00000080,    // field
    ACC_VARARGS = 0x00000080,      // method (1.5)
    ACC_NATIVE = 0x00000100,       // method
    ACC_INTERFACE = 0x00000200,    // class, ic
    ACC_ABSTRACT = 0x00000400,     // class, method, ic
    ACC_STRICT = 0x00000800,       // method
    ACC_SYNTHETIC = 0x00001000,    // field, method, ic
    ACC_ANNOTATION = 0x00002000,   // class, ic (1.5)
    ACC_ENUM = 0x00004000,         // class, field, ic (1.5)
    ACC_CONSTRUCTOR = 0x00010000,  // method (Dalvik only)
    ACC_DECLARED_SYNCHRONIZED =
        0x00020000, // method (Dalvik only)
    ACC_CLASS_MASK =
        (ACC_PUBLIC | ACC_FINAL | ACC_INTERFACE | ACC_ABSTRACT | ACC_SYNTHETIC | ACC_ANNOTATION | ACC_ENUM),
    ACC_INNER_CLASS_MASK =
        (ACC_CLASS_MASK | ACC_PRIVATE | ACC_PROTECTED | ACC_STATIC),
    ACC_FIELD_MASK =
        (ACC_PUBLIC | ACC_PRIVATE | ACC_PROTECTED | ACC_STATIC | ACC_FINAL | ACC_VOLATILE | ACC_TRANSIENT | ACC_SYNTHETIC | ACC_ENUM),
    ACC_METHOD_MASK =
        (ACC_PUBLIC | ACC_PRIVATE | ACC_PROTECTED | ACC_STATIC | ACC_FINAL | ACC_SYNCHRONIZED | ACC_BRIDGE | ACC_VARARGS | ACC_NATIVE | ACC_ABSTRACT | ACC_STRICT | ACC_SYNTHETIC | ACC_CONSTRUCTOR | ACC_DECLARED_SYNCHRONIZED),
};

/* annotation constants */
enum
{
    kDexVisibilityBuild = 0x00, /* annotation visibility */
    kDexVisibilityRuntime = 0x01,
    kDexVisibilitySystem = 0x02,

    kDexAnnotationByte = 0x00,
    kDexAnnotationShort = 0x02,
    kDexAnnotationChar = 0x03,
    kDexAnnotationInt = 0x04,
    kDexAnnotationLong = 0x06,
    kDexAnnotationFloat = 0x10,
    kDexAnnotationDouble = 0x11,
    kDexAnnotationMethodType = 0x15,
    kDexAnnotationMethodHandle = 0x16,
    kDexAnnotationString = 0x17,
    kDexAnnotationType = 0x18,
    kDexAnnotationField = 0x19,
    kDexAnnotationMethod = 0x1a,
    kDexAnnotationEnum = 0x1b,
    kDexAnnotationArray = 0x1c,
    kDexAnnotationAnnotation = 0x1d,
    kDexAnnotationNull = 0x1e,
    kDexAnnotationBoolean = 0x1f,

    kDexAnnotationValueTypeMask = 0x1f, /* low 5 bits */
    kDexAnnotationValueArgShift = 5,
};

/* map item type codes */
enum
{
    kDexTypeHeaderItem = 0x0000,
    kDexTypeStringIdItem = 0x0001,
    kDexTypeTypeIdItem = 0x0002,
    kDexTypeProtoIdItem = 0x0003,
    kDexTypeFieldIdItem = 0x0004,
    kDexTypeMethodIdItem = 0x0005,
    kDexTypeClassDefItem = 0x0006,
    kDexTypeCallSiteIdItem = 0x0007,
    kDexTypeMethodHandleItem = 0x0008,
    kDexTypeMapList = 0x1000,
    kDexTypeTypeList = 0x1001,
    kDexTypeAnnotationSetRefList = 0x1002,
    kDexTypeAnnotationSetItem = 0x1003,
    kDexTypeClassDataItem = 0x2000,
    kDexTypeCodeItem = 0x2001,
    kDexTypeStringDataItem = 0x2002,
    kDexTypeDebugInfoItem = 0x2003,
    kDexTypeAnnotationItem = 0x2004,
    kDexTypeEncodedArrayItem = 0x2005,
    kDexTypeAnnotationsDirectoryItem = 0x2006,
};

/* auxillary data section chunk codes */
enum
{
    kDexChunkClassLookup = 0x434c4b50,  /* CLKP */
    kDexChunkRegisterMaps = 0x524d4150, /* RMAP */

    kDexChunkEnd = 0x41454e44, /* AEND */
};

/* debug info opcodes and constants */
enum
{
    DBG_END_SEQUENCE = 0x00,
    DBG_ADVANCE_PC = 0x01,
    DBG_ADVANCE_LINE = 0x02,
    DBG_START_LOCAL = 0x03,
    DBG_START_LOCAL_EXTENDED = 0x04,
    DBG_END_LOCAL = 0x05,
    DBG_RESTART_LOCAL = 0x06,
    DBG_SET_PROLOGUE_END = 0x07,
    DBG_SET_EPILOGUE_BEGIN = 0x08,
    DBG_SET_FILE = 0x09,
    DBG_FIRST_SPECIAL = 0x0a,
    DBG_LINE_BASE = -4,
    DBG_LINE_RANGE = 15,
};

/*
 * Direct-mapped "header_item" struct.
 */
struct DexHeader
{
    u1 magic[8];                  /* includes version number */
    u4 checksum;                  /* adler32 checksum */
    u1 signature[kSHA1DigestLen]; /* SHA-1 hash */
    u4 fileSize;                  /* length of entire file */
    u4 headerSize;                /* offset to start of next section */
    u4 endianTag;
    u4 linkSize;
    u4 linkOff;
    u4 mapOff;
    u4 stringIdsSize;
    u4 stringIdsOff;
    u4 typeIdsSize;
    u4 typeIdsOff;
    u4 protoIdsSize;
    u4 protoIdsOff;
    u4 fieldIdsSize;
    u4 fieldIdsOff;
    u4 methodIdsSize;
    u4 methodIdsOff;
    u4 classDefsSize;
    u4 classDefsOff;
    u4 dataSize;
    u4 dataOff;
};

/*
 * Direct-mapped "map_item".
 */
struct DexMapItem
{
    u2 type; /* type code (see kDexType* above) */
    u2 unused;
    u4 size;   /* count of items of the indicated type */
    u4 offset; /* file offset to the start of data */
};

/*
 * Direct-mapped "map_list".
 */
struct DexMapList
{
    u4 size;            /* #of entries in list */
    struct DexMapItem list[1]; /* entries */
};

/*
 * Direct-mapped "string_id_item".
 */
struct DexStringId
{
    u4 stringDataOff; /* file offset to string_data_item */
};

/*
 * Direct-mapped "type_id_item".
 */
struct DexTypeId
{
    u4 descriptorIdx; /* index into stringIds list for type descriptor */
};

/*
 * Direct-mapped "field_id_item".
 */
struct DexFieldId
{
    u2 classIdx; /* index into typeIds list for defining class */
    u2 typeIdx;  /* index into typeIds for field type */
    u4 nameIdx;  /* index into stringIds for field name */
};

/*
 * Direct-mapped "method_id_item".
 */
struct DexMethodId
{
    u2 classIdx; /* index into typeIds list for defining class */
    u2 protoIdx; /* index into protoIds for method prototype */
    u4 nameIdx;  /* index into stringIds for method name */
};

/*
 * Direct-mapped "proto_id_item".
 */
struct DexProtoId
{
    u4 shortyIdx;     /* index into stringIds for shorty descriptor */
    u4 returnTypeIdx; /* index into typeIds list for return type */
    u4 parametersOff; /* file offset to type_list for parameter types */
};

/*
 * Direct-mapped "class_def_item".
 */
struct DexClassDef
{
    u4 classIdx; /* index into typeIds for this class */
    u4 accessFlags;
    u4 superclassIdx;   /* index into typeIds for superclass */
    u4 interfacesOff;   /* file offset to DexTypeList */
    u4 sourceFileIdx;   /* index into stringIds for source file name */
    u4 annotationsOff;  /* file offset to annotations_directory_item */
    u4 classDataOff;    /* file offset to class_data_item */
    u4 staticValuesOff; /* file offset to DexEncodedArray */
};

struct ClassDataItem
{
    uint32_t static_fields_size;
    uint32_t instance_fields_size;
    uint32_t direct_methods_size;
    uint32_t virtual_methods_size;
};


/*
 * Direct-mapped "call_site_id_item"
 */
struct DexCallSiteId
{
    u4 callSiteOff; /* file offset to DexEncodedArray */
};

/*
 * Enumeration of method handle type codes.
 */
enum MethodHandleType
{
    STATIC_PUT = 0x00,
    STATIC_GET = 0x01,
    INSTANCE_PUT = 0x02,
    INSTANCE_GET = 0x03,
    INVOKE_STATIC = 0x04,
    INVOKE_INSTANCE = 0x05,
    INVOKE_CONSTRUCTOR = 0x06,
    INVOKE_DIRECT = 0x07,
    INVOKE_INTERFACE = 0x08
};

/*
 * Direct-mapped "method_handle_item"
 */
struct DexMethodHandleItem
{
    u2 methodHandleType; /* type of method handle */
    u2 reserved1;        /* reserved for future use */
    u2 fieldOrMethodIdx; /* index of associated field or method */
    u2 reserved2;        /* reserved for future use */
};

/*
 * Direct-mapped "type_item".
 */
struct DexTypeItem
{
    u2 typeIdx; /* index into typeIds */
};

/*
 * Direct-mapped "type_list".
 */
struct DexTypeList
{
    u4 size;             /* #of entries in list */
    struct DexTypeItem list[1]; /* entries */
};

/*
 * Direct-mapped "code_item".
 *
 * The "catches" table is used when throwing an exception,
 * "debugInfo" is used when displaying an exception stack trace or
 * debugging. An offset of zero indicates that there are no entries.
 */
struct DexCode
{
    u2 registersSize;
    u2 insSize;
    u2 outsSize;
    u2 triesSize;
    u4 debugInfoOff; /* file offset to debug info stream */
    u4 insnsSize;    /* size of the insns array, in u2 units */
    u2 insns[1];
    /* followed by optional u2 padding */
    /* followed by try_item[triesSize] */
    /* followed by uleb128 handlersSize */
    /* followed by catch_handler_item[handlersSize] */
};

/*
 * Direct-mapped "try_item".
 */
struct DexTry
{
    u4 startAddr;  /* start address, in 16-bit code units */
    u2 insnCount;  /* instruction count, in 16-bit code units */
    u2 handlerOff; /* offset in encoded handler data to handlers */
};

/*
 * Link table.  Currently undefined.
 */
struct DexLink
{
    u1 bleargh;
};

/*
 * Direct-mapped "annotations_directory_item".
 */
struct DexAnnotationsDirectoryItem
{
    u4 classAnnotationsOff; /* offset to DexAnnotationSetItem */
    u4 fieldsSize;          /* count of DexFieldAnnotationsItem */
    u4 methodsSize;         /* count of DexMethodAnnotationsItem */
    u4 parametersSize;      /* count of DexParameterAnnotationsItem */
    /* followed by DexFieldAnnotationsItem[fieldsSize] */
    /* followed by DexMethodAnnotationsItem[methodsSize] */
    /* followed by DexParameterAnnotationsItem[parametersSize] */
};

/*
 * Direct-mapped "field_annotations_item".
 */
struct DexFieldAnnotationsItem
{
    u4 fieldIdx;
    u4 annotationsOff; /* offset to DexAnnotationSetItem */
};

/*
 * Direct-mapped "method_annotations_item".
 */
struct DexMethodAnnotationsItem
{
    u4 methodIdx;
    u4 annotationsOff; /* offset to DexAnnotationSetItem */
};

/*
 * Direct-mapped "parameter_annotations_item".
 */
struct DexParameterAnnotationsItem
{
    u4 methodIdx;
    u4 annotationsOff; /* offset to DexAnotationSetRefList */
};

/*
 * Direct-mapped "annotation_set_ref_item".
 */
struct DexAnnotationSetRefItem
{
    u4 annotationsOff; /* offset to DexAnnotationSetItem */
};

/*
 * Direct-mapped "annotation_set_ref_list".
 */
struct DexAnnotationSetRefList
{
    u4 size;
    struct DexAnnotationSetRefItem list[1];
};

/*
 * Direct-mapped "annotation_set_item".
 */
struct DexAnnotationSetItem
{
    u4 size;
    u4 entries[1]; /* offset to DexAnnotationItem */
};

/*
 * Direct-mapped "annotation_item".
 *
 * NOTE: this structure is byte-aligned.
 */
struct DexAnnotationItem
{
    u1 visibility;
    u1 annotation[1]; /* data in encoded_annotation format */
};

/*
 * Direct-mapped "encoded_array".
 *
 * NOTE: this structure is byte-aligned.
 */
struct DexEncodedArray
{
    u1 array[1]; /* data in encoded_array format */
};

/*
 * Lookup table for classes.  It provides a mapping from class name to
 * class definition.  Used by dexFindClass().
 *
 * We calculate this at DEX optimization time and embed it in the file so we
 * don't need the same hash table in every VM.  This is slightly slower than
 * a hash table with direct pointers to the items, but because it's shared
 * there's less of a penalty for using a fairly sparse table.
 */
struct DexClassLookup
{
    int size;       // total size, including "size"
    int numEntries; // size of table[]; always power of 2
    struct
    {
        u4 classDescriptorHash;    // class descriptor hash code
        int classDescriptorOffset; // in bytes, from start of DEX
        int classDefOffset;        // in bytes, from start of DEX
    } table[1];
};

/*
 * Header added by DEX optimization pass.  Values are always written in
 * local byte and structure padding.  The first field (magic + version)
 * is guaranteed to be present and directly readable for all expected
 * compiler configurations; the rest is version-dependent.
 *
 * Try to keep this simple and fixed-size.
 */
struct DexOptHeader
{
    u1 magic[8]; /* includes version number */

    u4 dexOffset; /* file offset of DEX header */
    u4 dexLength;
    u4 depsOffset; /* offset of optimized DEX dependency table */
    u4 depsLength;
    u4 optOffset; /* file offset of optimized data tables */
    u4 optLength;

    u4 flags;    /* some info flags */
    u4 checksum; /* adler32 checksum covering deps/opt */

    /* pad for 64-bit alignment if necessary */
};

struct EncodeField {
    uint32_t field_idx_diff;
    uint32_t access_flags;
};

struct EncodeMethod {
    uint32_t method_idx;
    uint32_t access_flags;
    uint32_t code_off;
};

#define DEX_OPT_FLAG_BIG (1 << 1) /* swapped to big-endian */

#define DEX_INTERFACE_CACHE_SIZE 128 /* must be power of 2 */

/*
 * Structure representing a DEX file.
 *
 * Code should regard DexFile as opaque, using the API calls provided here
 * to access specific structures.
 */
struct DexFile
{
    /* directly-mapped "opt" header */
    struct DexOptHeader *pOptHeader;

    /* pointers to directly-mapped structs and arrays in base DEX */
    struct DexHeader *pHeader;
    struct DexStringId *pStringIds;
    struct DexTypeId *pTypeIds;
    struct DexFieldId *pFieldIds;
    struct DexMethodId *pMethodIds;
    struct DexProtoId *pProtoIds;
    struct DexClassDef *pClassDefs;
    struct DexLink *pLinkData;

    /*
     * These are mapped out of the "auxillary" section, and may not be
     * included in the file.
     */
    struct DexClassLookup *pClassLookup;
    const void *pRegisterMapPool; // RegisterMapClassPool

    /* points to start of DEX file data */
    const u1 *baseAddr;

    /* track memory overhead for auxillary structures */
    int overhead;

    /* additional app-specific data structures associated with the DEX */
    // void*               auxData;
};

/* bit values for "flags" argument to dexFileParse */
enum
{
    kDexParseDefault = 0,
    kDexParseVerifyChecksum = 1,
    kDexParseContinueOnError = (1 << 1),
};

/// @brief 通过method_id 获取在指定dex文件中的偏移, 并获取method长度
/// @param dex_mem 指定dex文件头的内存
/// @param method_id 指定的method_id
/// @param method_size 指定method的长度
/// @return 指定的DexCode item 结构体
struct DexCode* get_method_off(struct DexHeader* dex_mem, uint32_t method_id, uint32_t* method_size);

/// @brief 通过 string_idx 寻找字符串
/// @param dex_mem 指定dex文件头的内存
/// @param string_idx 指定的string索引
/// @param strlen 带回查找到的字符串长度
/// @return 返回找到的字符串
char* get_string(struct DexHeader* dex_mem, uint32_t string_idx, uint32_t* str_len);

/// @brief 找到指定的type
/// @param dex_mem 指定dex文件头的内存
/// @param type_idx 指定的type索引
/// @return DexTypeId 类型, 找到的type
struct DexTypeId* get_type(struct DexHeader* dex_mem, uint32_t type_idx);

/// @brief 根据 class 所在的偏移, 获取class信息
/// @param dex_mem 指定dex文件头的内存
/// @param class_data_off 指定的class偏移
/// @param class_data_out 带回 读取完信息之后的class内容地址
/// @return 返回 ClassDataItem 信息
struct ClassDataItem* get_class_item(struct DexHeader* dex_mem, uint32_t class_data_off, uint8_t **class_data_out);

/// @brief 根据指定的code偏移, 获取code内容
/// @param dex_mem 指定dex文件头的内存
/// @param code_off 指定的code偏移
/// @param code_out 带回 真正code内容
/// @return DexCode 基础内容
struct DexCode* get_code_item(struct DexHeader* dex_mem, uint32_t code_off, uint8_t** code_out);

/// @brief 从指定的fields起始位置, 读取EncodeField
/// @param fields_start 指定的fields起始位置
/// @param old_diff 上一次 diff idx, 并带回
/// @return EncodeField
struct EncodeField* get_field_x(uint8_t** fields_start, uint32_t* old_diff);

/// @brief 从指定的method起始位置, 读取EncodeMethod
/// @param methods_start 指定的method起始位置
/// @param old_diff 上一次 diff idx, 并带回
/// @return EncodeMethod
struct EncodeMethod* get_method_x(uint8_t** methods_start, uint32_t* old_diff);

/// @brief 在指定的class中寻找指定索引的method
/// @param dex_mem 指定dex文件头的内存
/// @param class_data_item 指定的class
/// @param method_idx 指定的method索引
/// @param class_data class内容地址
/// @return 如果找到返回对应method的 DexCode 结构, 如果没找到返回NULL
struct DexCode* find_method_from_class(struct DexHeader* dex_mem, struct ClassDataItem* class_data_item, uint32_t method_idx, uint8_t **class_data);

/// @brief 在指定的dex中根据指定的索引寻找指定的method
/// @param dex_mem 指定dex文件头的内存
/// @param method_idx 指定的method索引
/// @return 如果找到返回对应method的 DexCode 结构, 如果没找到返回NULL
struct DexCode* find_method(struct DexHeader* dex_mem, uint32_t method_idx);

#endif // LIBDEX_DEXFILE_H_
