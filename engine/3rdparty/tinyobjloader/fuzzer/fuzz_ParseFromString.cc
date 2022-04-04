#include <stdio.h>
#include <stdlib.h>
#include <stdint.h>
#include <stdarg.h>
#include <string.h>

#define TINYOBJLOADER_IMPLEMENTATION // define this in only *one* .cc
#include "tiny_obj_loader.h"

extern "C" int LLVMFuzzerTestOneInput(const uint8_t *Data, size_t Size) {
    tinyobj::ObjReaderConfig reader_config;
    tinyobj::ObjReader reader;
    if (Size < 2) {
        return 0;
    }
    for (size_t i = 0; i < Size-1; i++) {
        if (Data[i] == 0) {
            std::string obj_text (reinterpret_cast<const char*>(Data), i);
            std::string mtl_text (reinterpret_cast<const char*>(Data+i+1), Size-i-1);
            reader.ParseFromString(obj_text, mtl_text,reader_config);
            return 0;
        }
    }
    return 0;
}

