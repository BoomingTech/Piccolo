//
// g++ -g -std=c++11 main.cc
//
#define TINYOBJLOADER_IMPLEMENTATION
#include "tiny_obj_loader.h"

#include <cassert>
#include <cstdio>
#include <cstdlib>
#include <fstream>
#include <iostream>
#include <sstream>

#include <unordered_map> // C++11

#ifdef __clang__
#pragma clang diagnostic push
#if __has_warning("-Wzero-as-null-pointer-constant")
#pragma clang diagnostic ignored "-Wzero-as-null-pointer-constant"
#endif
#endif

static void ConstructVertexWeight(
  const std::vector<tinyobj::real_t> &vertices,
  const std::vector<tinyobj::skin_weight_t> &skin_weights,
  std::vector<tinyobj::skin_weight_t> *vertex_skin_weights)
{
  size_t num_vertices = vertices.size() / 3;

  vertex_skin_weights->resize(num_vertices);

  for (size_t i = 0; i < skin_weights.size(); i++) {
    const tinyobj::skin_weight_t &skin = skin_weights[i];

    assert(skin.vertex_id >= 0);
    assert(skin.vertex_id < num_vertices);

    (*vertex_skin_weights)[skin.vertex_id] = skin;
  }

  // now you can lookup i'th vertex skin weight by `vertex_skin_weights[i]`


}

static bool TestLoadObj(const char* filename, const char* basepath = nullptr,
                        bool triangulate = true) {
  std::cout << "Loading " << filename << std::endl;

  tinyobj::attrib_t attrib;
  std::vector<tinyobj::shape_t> shapes;
  std::vector<tinyobj::material_t> materials;

  std::string warn;
  std::string err;
  bool ret = tinyobj::LoadObj(&attrib, &shapes, &materials, &warn, &err, filename,
                              basepath, triangulate);
  if (!warn.empty()) {
    std::cout << "WARN: " << warn << std::endl;
  }

  if (!err.empty()) {
    std::cerr << "ERR: " << err << std::endl;
  }

  if (!ret) {
    printf("Failed to load/parse .obj.\n");
    return false;
  }

  std::vector<tinyobj::skin_weight_t> vertex_skin_weights;

  ConstructVertexWeight(
    attrib.vertices,
    attrib.skin_weights,
    &vertex_skin_weights);

  for (size_t v = 0; v < vertex_skin_weights.size(); v++) {
    std::cout << "vertex[" << v << "] num_weights = " << vertex_skin_weights[v].weightValues.size() << "\n";
    for (size_t w = 0; w < vertex_skin_weights[v].weightValues.size(); w++) {
      std::cout << "  w[" << w << "] joint = " << vertex_skin_weights[v].weightValues[w].joint_id
                << ", weight = " << vertex_skin_weights[v].weightValues[w].weight << "\n";
    }
  }

  return true;
}


int main(int argc, char** argv) {
  if (argc < 2) {
    std::cerr << "Need input.obj\n";
    return EXIT_FAILURE;
  }

  const char* basepath = nullptr;
  if (argc > 2) {
   basepath = argv[2];
  }
  assert(true == TestLoadObj(argv[1], basepath));

  return 0;
}
