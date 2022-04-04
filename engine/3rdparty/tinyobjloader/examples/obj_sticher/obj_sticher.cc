//
// Stiches multiple .obj files into one .obj.
//
#include "obj_writer.h"

#include "../../tiny_obj_loader.h"

#include <cassert>
#include <iostream>
#include <cstdlib>
#include <cstdio>

typedef std::vector<tinyobj::shape_t> Shape;
typedef std::vector<tinyobj::material_t> Material;
typedef tinyobj::attrib_t Attribute;

void
StichObjs(
  tinyobj::attrib_t& out_attribute,
  std::vector<tinyobj::shape_t>& out_shape,
  std::vector<tinyobj::material_t>& out_material,
  const std::vector<Attribute>& attributes,
  const std::vector<Shape>& shapes,
  const std::vector<Material>& materials)
{
  // The amount of attributes, shape-vectors and material-vecotrs should be the same.
  if(attributes.size() != shapes.size() && attributes.size() != materials.size()){
    std::cerr << "Size of attributes, shapes and Materials don't fit!" << attributes.size() << " " << shapes.size() <<" " << materials.size() << std::endl;;
    exit(1);
  }
  int num_shapes = 0;
  // 4 values (vertices, normals, texcoords, colors)
  std::vector<int> num_attributes(4, 0);
  int num_materials = 0;
  for(int i = 0; i < shapes.size(); i++){
    num_shapes += shapes[i].size();
  }
  for(int i = 0; i < attributes.size(); i++){
    num_attributes[0] += attributes[i].vertices.size();
    num_attributes[1] += attributes[i].normals.size();
    num_attributes[2] += attributes[i].texcoords.size();
    num_attributes[3] += attributes[i].colors.size();
  }
  for(int i = 0; i < materials.size(); i++){
    num_materials += materials[i].size();
  }

  // More performant, than push_back
  out_attribute.vertices.resize(num_attributes[0]);
  out_attribute.normals.resize(num_attributes[1]);
  out_attribute.texcoords.resize(num_attributes[2]);
  out_attribute.colors.resize(num_attributes[3]);
  out_shape.resize(num_shapes);
  out_material.resize(num_materials);

  int material_id_offset = 0;
  int shape_id_offset = 0;
  int vertex_idx_offset = 0;
  int normal_idx_offset = 0;
  int texcoord_idx_offset = 0;
  int color_idx_offset = 0;

  // shapes.size() = attributes.size() = materials.size()
  for (size_t i = 0; i < shapes.size(); i++) {

    // Copy shapes
    for (size_t k = 0; k < shapes[i].size(); k++) {
      std::string new_name = shapes[i][k].name;
      // Add suffix
      char buf[1024];
      sprintf(buf, "_%04d", (int)i);
      new_name += std::string(buf);

      printf("shape[%ld][%ld].name = %s\n", i, k, shapes[i][k].name.c_str());

      tinyobj::shape_t new_shape = shapes[i][k];
      // Add material offset.
      for(size_t f = 0; f < new_shape.mesh.material_ids.size(); f++) {
        new_shape.mesh.material_ids[f] += material_id_offset;
      }
      // Add indices offset.
      for(size_t f = 0; f < new_shape.mesh.indices.size(); f++){
        tinyobj::index_t& ref = new_shape.mesh.indices[f];
        if(ref.vertex_index > -1){
          ref.vertex_index += vertex_idx_offset;
        }
        if(ref.normal_index > -1){
          ref.normal_index += normal_idx_offset;
        }
        if(ref.texcoord_index > -1){
          ref.texcoord_index += texcoord_idx_offset;
        }
      }

      new_shape.name = new_name;
      printf("shape[%ld][%ld].new_name = %s\n", i, k, new_shape.name.c_str());

      out_shape[shape_id_offset++] = new_shape;
    }

    // Copy materials
    for (size_t k = 0; k < materials[i].size(); k++) {
      out_material[material_id_offset++] = materials[i][k];
    }

    // Copy attributes (3 floats per vertex, 3 floats per normal, 2 floats per texture-coordinate, 3 floats per color)
    // You could also include a check here, if the sizes are dividable by 3 (resp. 2), but it's safe to simply assume, they do.
    std::copy(attributes[i].vertices.begin(), attributes[i].vertices.end(), out_attribute.vertices.begin() + vertex_idx_offset * 3);
    vertex_idx_offset += attributes[i].vertices.size() / 3;
    std::copy(attributes[i].normals.begin(), attributes[i].normals.end(), out_attribute.normals.begin() + normal_idx_offset * 3);
    normal_idx_offset += attributes[i].normals.size() / 3;
    std::copy(attributes[i].texcoords.begin(), attributes[i].texcoords.end(), out_attribute.texcoords.begin() + texcoord_idx_offset * 2);
    texcoord_idx_offset += attributes[i].texcoords.size() / 2;
    std::copy(attributes[i].colors.begin(), attributes[i].colors.end(), out_attribute.colors.begin() + color_idx_offset);
    color_idx_offset += attributes[i].colors.size();
  }
}

int main(int argc, char **argv)
{
  if (argc < 3) {
    printf("Usage: obj_sticher input0.obj input1.obj ... output.obj\n");
    exit(1);
  }

  int num_objfiles = argc - 2;
  std::string out_filename = std::string(argv[argc-1]); // last element

  std::vector<Attribute> attributes(num_objfiles);
  std::vector<Shape> shapes(num_objfiles);
  std::vector<Material> materials(num_objfiles);

  for (int i = 0; i < num_objfiles; i++) {
    std::cout << "Loading " << argv[i+1] << " ... " << std::flush;

    std::string warn;
    std::string err;
    bool ret = tinyobj::LoadObj(&attributes[i], &shapes[i], &materials[i], &warn, &err, argv[i+1]);
    if (!warn.empty()) {
      std::cerr << "WARN:" << warn << std::endl;
    }

    if (!err.empty()) {
      std::cerr << err << std::endl;
    }
    if (!ret) {
      exit(1);
    }

    std::cout << "DONE." << std::endl;
  }

  Attribute out_attribute;
  Shape out_shape;
  Material out_material;
  StichObjs(out_attribute, out_shape, out_material, attributes, shapes, materials);

  bool coordTransform = true;
  bool ret = WriteObj(out_filename, out_attribute, out_shape, out_material, coordTransform);
  assert(ret);

  return 0;
}
