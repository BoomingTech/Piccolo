#ifndef __OBJ_WRITER_H__
#define __OBJ_WRITER_H__

#include "../../tiny_obj_loader.h"

extern bool WriteObj(const std::string& filename, const tinyobj::attrib_t& attributes, const std::vector<tinyobj::shape_t>& shapes, const std::vector<tinyobj::material_t>& materials, bool coordTransform = false);

#endif // __OBJ_WRITER_H__
