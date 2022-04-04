#define TINYOBJLOADER_IMPLEMENTATION
#include "../tiny_obj_loader.h"

#if defined(__clang__)
#pragma clang diagnostic push
#pragma clang diagnostic ignored "-Weverything"
#elif defined(__GNUC__)
#pragma GCC diagnostic ignored "-Wmissing-declarations"
#pragma GCC diagnostic ignored "-Wignored-qualifiers"
#pragma GCC diagnostic push
#pragma GCC diagnostic ignored "-Wcast-qual"
#pragma GCC diagnostic ignored "-Wsign-conversion"
#pragma GCC diagnostic ignored "-Wformat"
#pragma GCC diagnostic ignored "-Wswitch-default"
#endif


#include "acutest.h"

#if defined(__clang__)
#pragma clang diagnostic pop
#elif defined(__GNUC__)
#pragma GCC diagnostic pop
#endif

#include <cassert>
#include <cstdio>
#include <cstdlib>
#include <fstream>
#include <iostream>
#include <sstream>

template <typename T>
static bool FloatEquals(const T& a, const T& b) {
  // Edit eps value as you wish.
  const T eps = std::numeric_limits<T>::epsilon() * static_cast<T>(100);

  const T abs_diff = std::abs(a - b);

  if (abs_diff < eps) {
    return true;
  } else {
    return false;
  }
}

static void PrintInfo(const tinyobj::attrib_t& attrib,
                      const std::vector<tinyobj::shape_t>& shapes,
                      const std::vector<tinyobj::material_t>& materials,
                      bool triangulate = true) {
  std::cout << "# of vertices  : " << (attrib.vertices.size() / 3) << std::endl;
  std::cout << "# of normals   : " << (attrib.normals.size() / 3) << std::endl;
  std::cout << "# of texcoords : " << (attrib.texcoords.size() / 2)
            << std::endl;

  std::cout << "# of shapes    : " << shapes.size() << std::endl;
  std::cout << "# of materials : " << materials.size() << std::endl;

  for (size_t v = 0; v < attrib.vertices.size() / 3; v++) {
    printf("  v[%ld] = (%f, %f, %f)\n", v,
           static_cast<const double>(attrib.vertices[3 * v + 0]),
           static_cast<const double>(attrib.vertices[3 * v + 1]),
           static_cast<const double>(attrib.vertices[3 * v + 2]));
  }

  for (size_t v = 0; v < attrib.normals.size() / 3; v++) {
    printf("  n[%ld] = (%f, %f, %f)\n", v,
           static_cast<const double>(attrib.normals[3 * v + 0]),
           static_cast<const double>(attrib.normals[3 * v + 1]),
           static_cast<const double>(attrib.normals[3 * v + 2]));
  }

  for (size_t v = 0; v < attrib.texcoords.size() / 2; v++) {
    printf("  uv[%ld] = (%f, %f)\n", v,
           static_cast<const double>(attrib.texcoords[2 * v + 0]),
           static_cast<const double>(attrib.texcoords[2 * v + 1]));
  }

  for (size_t i = 0; i < shapes.size(); i++) {
    printf("shape[%ld].name = %s\n", i, shapes[i].name.c_str());
    printf("Size of shape[%ld].indices: %ld\n", i,
           shapes[i].mesh.indices.size());

    if (triangulate) {
      printf("Size of shape[%ld].material_ids: %ld\n", i,
             shapes[i].mesh.material_ids.size());
      assert((shapes[i].mesh.indices.size() % 3) == 0);
      for (size_t f = 0; f < shapes[i].mesh.indices.size() / 3; f++) {
        tinyobj::index_t i0 = shapes[i].mesh.indices[3 * f + 0];
        tinyobj::index_t i1 = shapes[i].mesh.indices[3 * f + 1];
        tinyobj::index_t i2 = shapes[i].mesh.indices[3 * f + 2];
        printf("  idx[%ld] = %d/%d/%d, %d/%d/%d, %d/%d/%d. mat_id = %d\n", f,
               i0.vertex_index, i0.normal_index, i0.texcoord_index,
               i1.vertex_index, i1.normal_index, i1.texcoord_index,
               i2.vertex_index, i2.normal_index, i2.texcoord_index,
               shapes[i].mesh.material_ids[f]);
      }
    } else {
      for (size_t f = 0; f < shapes[i].mesh.indices.size(); f++) {
        tinyobj::index_t idx = shapes[i].mesh.indices[f];
        printf("  idx[%ld] = %d/%d/%d\n", f, idx.vertex_index, idx.normal_index,
               idx.texcoord_index);
      }

      printf("Size of shape[%ld].material_ids: %ld\n", i,
             shapes[i].mesh.material_ids.size());
      assert(shapes[i].mesh.material_ids.size() ==
             shapes[i].mesh.num_face_vertices.size());
      for (size_t m = 0; m < shapes[i].mesh.material_ids.size(); m++) {
        printf("  material_id[%ld] = %d\n", m, shapes[i].mesh.material_ids[m]);
      }
    }

    printf("shape[%ld].num_faces: %ld\n", i,
           shapes[i].mesh.num_face_vertices.size());
    for (size_t v = 0; v < shapes[i].mesh.num_face_vertices.size(); v++) {
      printf("  num_vertices[%ld] = %ld\n", v,
             static_cast<long>(shapes[i].mesh.num_face_vertices[v]));
    }

    // printf("shape[%ld].vertices: %ld\n", i, shapes[i].mesh.positions.size());
    // assert((shapes[i].mesh.positions.size() % 3) == 0);
    // for (size_t v = 0; v < shapes[i].mesh.positions.size() / 3; v++) {
    //  printf("  v[%ld] = (%f, %f, %f)\n", v,
    //    static_cast<const double>(shapes[i].mesh.positions[3*v+0]),
    //    static_cast<const double>(shapes[i].mesh.positions[3*v+1]),
    //    static_cast<const double>(shapes[i].mesh.positions[3*v+2]));
    //}

    printf("shape[%ld].num_tags: %ld\n", i, shapes[i].mesh.tags.size());
    for (size_t t = 0; t < shapes[i].mesh.tags.size(); t++) {
      printf("  tag[%ld] = %s ", t, shapes[i].mesh.tags[t].name.c_str());
      printf(" ints: [");
      for (size_t j = 0; j < shapes[i].mesh.tags[t].intValues.size(); ++j) {
        printf("%ld", static_cast<long>(shapes[i].mesh.tags[t].intValues[j]));
        if (j < (shapes[i].mesh.tags[t].intValues.size() - 1)) {
          printf(", ");
        }
      }
      printf("]");

      printf(" floats: [");
      for (size_t j = 0; j < shapes[i].mesh.tags[t].floatValues.size(); ++j) {
        printf("%f", static_cast<const double>(
                         shapes[i].mesh.tags[t].floatValues[j]));
        if (j < (shapes[i].mesh.tags[t].floatValues.size() - 1)) {
          printf(", ");
        }
      }
      printf("]");

      printf(" strings: [");
      for (size_t j = 0; j < shapes[i].mesh.tags[t].stringValues.size(); ++j) {
        printf("%s", shapes[i].mesh.tags[t].stringValues[j].c_str());
        if (j < (shapes[i].mesh.tags[t].stringValues.size() - 1)) {
          printf(", ");
        }
      }
      printf("]");
      printf("\n");
    }
  }

  for (size_t i = 0; i < materials.size(); i++) {
    printf("material[%ld].name = %s\n", i, materials[i].name.c_str());
    printf("  material.Ka = (%f, %f ,%f)\n",
           static_cast<const double>(materials[i].ambient[0]),
           static_cast<const double>(materials[i].ambient[1]),
           static_cast<const double>(materials[i].ambient[2]));
    printf("  material.Kd = (%f, %f ,%f)\n",
           static_cast<const double>(materials[i].diffuse[0]),
           static_cast<const double>(materials[i].diffuse[1]),
           static_cast<const double>(materials[i].diffuse[2]));
    printf("  material.Ks = (%f, %f ,%f)\n",
           static_cast<const double>(materials[i].specular[0]),
           static_cast<const double>(materials[i].specular[1]),
           static_cast<const double>(materials[i].specular[2]));
    printf("  material.Tr = (%f, %f ,%f)\n",
           static_cast<const double>(materials[i].transmittance[0]),
           static_cast<const double>(materials[i].transmittance[1]),
           static_cast<const double>(materials[i].transmittance[2]));
    printf("  material.Ke = (%f, %f ,%f)\n",
           static_cast<const double>(materials[i].emission[0]),
           static_cast<const double>(materials[i].emission[1]),
           static_cast<const double>(materials[i].emission[2]));
    printf("  material.Ns = %f\n",
           static_cast<const double>(materials[i].shininess));
    printf("  material.Ni = %f\n", static_cast<const double>(materials[i].ior));
    printf("  material.dissolve = %f\n",
           static_cast<const double>(materials[i].dissolve));
    printf("  material.illum = %d\n", materials[i].illum);
    printf("  material.map_Ka = %s\n", materials[i].ambient_texname.c_str());
    printf("  material.map_Kd = %s\n", materials[i].diffuse_texname.c_str());
    printf("  material.map_Ks = %s\n", materials[i].specular_texname.c_str());
    printf("  material.map_Ns = %s\n",
           materials[i].specular_highlight_texname.c_str());
    printf("  material.map_bump = %s\n", materials[i].bump_texname.c_str());
    printf("  material.map_d = %s\n", materials[i].alpha_texname.c_str());
    printf("  material.disp = %s\n", materials[i].displacement_texname.c_str());
    printf("  material.refl = %s\n", materials[i].reflection_texname.c_str());
    std::map<std::string, std::string>::const_iterator it(
        materials[i].unknown_parameter.begin());
    std::map<std::string, std::string>::const_iterator itEnd(
        materials[i].unknown_parameter.end());

    for (; it != itEnd; it++) {
      printf("  material.%s = %s\n", it->first.c_str(), it->second.c_str());
    }
    printf("\n");
  }
}

static bool TestLoadObj(const char* filename, const char* basepath = NULL,
                        bool triangulate = true) {
  std::cout << "Loading " << filename << std::endl;

  tinyobj::attrib_t attrib;
  std::vector<tinyobj::shape_t> shapes;
  std::vector<tinyobj::material_t> materials;

  std::string warn;
  std::string err;
  bool ret = tinyobj::LoadObj(&attrib, &shapes, &materials, &warn, &err,
                              filename, basepath, triangulate);

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

  PrintInfo(attrib, shapes, materials, triangulate);

  return true;
}

static bool TestLoadObjFromPreopenedFile(const char* filename,
                                         const char* basepath = NULL,
                                         bool readMaterials = true,
                                         bool triangulate = true) {
  std::string fullFilename = std::string(basepath) + filename;
  std::cout << "Loading " << fullFilename << std::endl;

  std::ifstream fileStream(fullFilename.c_str());

  if (!fileStream) {
    std::cerr << "Could not find specified file: " << fullFilename << std::endl;
    return false;
  }

  tinyobj::MaterialStreamReader materialStreamReader(fileStream);
  tinyobj::MaterialStreamReader* materialReader =
      readMaterials ? &materialStreamReader : NULL;

  tinyobj::attrib_t attrib;
  std::vector<tinyobj::shape_t> shapes;
  std::vector<tinyobj::material_t> materials;

  std::string warn;
  std::string err;
  bool ret = tinyobj::LoadObj(&attrib, &shapes, &materials, &warn, &err,
                              &fileStream, materialReader);

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

  std::cout << "Loaded material count: " << materials.size() << "\n";

  return true;
}

static bool TestStreamLoadObj() {
  std::cout << "Stream Loading " << std::endl;

  std::stringstream objStream;
  objStream << "mtllib cube.mtl\n"
               "\n"
               "v 0.000000 2.000000 2.000000\n"
               "v 0.000000 0.000000 2.000000\n"
               "v 2.000000 0.000000 2.000000\n"
               "v 2.000000 2.000000 2.000000\n"
               "v 0.000000 2.000000 0.000000\n"
               "v 0.000000 0.000000 0.000000\n"
               "v 2.000000 0.000000 0.000000\n"
               "v 2.000000 2.000000 0.000000\n"
               "# 8 vertices\n"
               "\n"
               "g front cube\n"
               "usemtl white\n"
               "f 1 2 3 4\n"
               "g back cube\n"
               "# expects white material\n"
               "f 8 7 6 5\n"
               "g right cube\n"
               "usemtl red\n"
               "f 4 3 7 8\n"
               "g top cube\n"
               "usemtl white\n"
               "f 5 1 4 8\n"
               "g left cube\n"
               "usemtl green\n"
               "f 5 6 2 1\n"
               "g bottom cube\n"
               "usemtl white\n"
               "f 2 6 7 3\n"
               "# 6 elements";

  std::string matStream(
      "newmtl white\n"
      "Ka 0 0 0\n"
      "Kd 1 1 1\n"
      "Ks 0 0 0\n"
      "\n"
      "newmtl red\n"
      "Ka 0 0 0\n"
      "Kd 1 0 0\n"
      "Ks 0 0 0\n"
      "\n"
      "newmtl green\n"
      "Ka 0 0 0\n"
      "Kd 0 1 0\n"
      "Ks 0 0 0\n"
      "\n"
      "newmtl blue\n"
      "Ka 0 0 0\n"
      "Kd 0 0 1\n"
      "Ks 0 0 0\n"
      "\n"
      "newmtl light\n"
      "Ka 20 20 20\n"
      "Kd 1 1 1\n"
      "Ks 0 0 0");

  using namespace tinyobj;
  class MaterialStringStreamReader : public MaterialReader {
   public:
    MaterialStringStreamReader(const std::string& matSStream)
        : m_matSStream(matSStream) {}
    virtual ~MaterialStringStreamReader() {}
    virtual bool operator()(const std::string& matId,
                            std::vector<material_t>* materials,
                            std::map<std::string, int>* matMap,
                            std::string* warn, std::string* err) {
      (void)matId;
      (void)warn;
      (void)err;
      std::string warning;
      std::string error_msg;
      LoadMtl(matMap, materials, &m_matSStream, &warning, &error_msg);
      return true;
    }

   private:
    std::stringstream m_matSStream;
  };

  MaterialStringStreamReader matSSReader(matStream);
  tinyobj::attrib_t attrib;
  std::vector<tinyobj::shape_t> shapes;
  std::vector<tinyobj::material_t> materials;
  std::string warn;
  std::string err;
  bool ret = tinyobj::LoadObj(&attrib, &shapes, &materials, &warn, &err,
                              &objStream, &matSSReader);

  if (!warn.empty()) {
    std::cout << "WARN: " << warn << std::endl;
  }

  if (!err.empty()) {
    std::cerr << "ERR: " << err << std::endl;
  }

  if (!ret) {
    return false;
  }

  PrintInfo(attrib, shapes, materials);

  return true;
}

const char* gMtlBasePath = "../models/";

void test_cornell_box() {
  TEST_CHECK(true == TestLoadObj("../models/cornell_box.obj", gMtlBasePath));
}

void test_catmark_torus_creases0() {
  tinyobj::attrib_t attrib;
  std::vector<tinyobj::shape_t> shapes;
  std::vector<tinyobj::material_t> materials;

  std::string warn;
  std::string err;
  bool ret = tinyobj::LoadObj(&attrib, &shapes, &materials, &warn, &err,
                              "../models/catmark_torus_creases0.obj",
                              gMtlBasePath, /*triangulate*/ false);

  if (!warn.empty()) {
    std::cout << "WARN: " << warn << std::endl;
  }

  if (!err.empty()) {
    std::cerr << "ERR: " << err << std::endl;
  }

  TEST_CHECK(true == ret);

  TEST_CHECK(1 == shapes.size());
  TEST_CHECK(8 == shapes[0].mesh.tags.size());
}

void test_pbr() {
  tinyobj::attrib_t attrib;
  std::vector<tinyobj::shape_t> shapes;
  std::vector<tinyobj::material_t> materials;

  std::string warn;
  std::string err;
  bool ret = tinyobj::LoadObj(&attrib, &shapes, &materials, &warn, &err,
                              "../models/pbr-mat-ext.obj", gMtlBasePath,
                              /*triangulate*/ false);

  if (!warn.empty()) {
    std::cout << "WARN: " << warn << std::endl;
  }

  if (!err.empty()) {
    std::cerr << "ERR: " << err << std::endl;
  }
  TEST_CHECK(true == ret);
  TEST_CHECK(1 == materials.size());
  TEST_CHECK(FloatEquals(0.2f, materials[0].roughness));
  TEST_CHECK(FloatEquals(0.3f, materials[0].metallic));
  TEST_CHECK(FloatEquals(0.4f, materials[0].sheen));
  TEST_CHECK(FloatEquals(0.5f, materials[0].clearcoat_thickness));
  TEST_CHECK(FloatEquals(0.6f, materials[0].clearcoat_roughness));
  TEST_CHECK(FloatEquals(0.7f, materials[0].anisotropy));
  TEST_CHECK(FloatEquals(0.8f, materials[0].anisotropy_rotation));
  TEST_CHECK(0 == materials[0].roughness_texname.compare("roughness.tex"));
  TEST_CHECK(0 == materials[0].metallic_texname.compare("metallic.tex"));
  TEST_CHECK(0 == materials[0].sheen_texname.compare("sheen.tex"));
  TEST_CHECK(0 == materials[0].emissive_texname.compare("emissive.tex"));
  TEST_CHECK(0 == materials[0].normal_texname.compare("normalmap.tex"));
}

void test_stream_load() { TEST_CHECK(true == TestStreamLoadObj()); }

void test_stream_load_from_file_skipping_materials() {
  TEST_CHECK(true == TestLoadObjFromPreopenedFile(
                         "../models/pbr-mat-ext.obj", gMtlBasePath,
                         /*readMaterials*/ false, /*triangulate*/ false));
}

void test_stream_load_from_file_with_materials() {
  TEST_CHECK(true == TestLoadObjFromPreopenedFile(
                         "../models/pbr-mat-ext.obj", gMtlBasePath,
                         /*readMaterials*/ true, /*triangulate*/ false));
}

void test_trailing_whitespace_in_mtl_issue92() {
  tinyobj::attrib_t attrib;
  std::vector<tinyobj::shape_t> shapes;
  std::vector<tinyobj::material_t> materials;

  std::string warn;
  std::string err;
  bool ret = tinyobj::LoadObj(&attrib, &shapes, &materials, &warn, &err,
                              "../models/issue-92.obj", gMtlBasePath);

  if (!warn.empty()) {
    std::cout << "WARN: " << warn << std::endl;
  }

  if (!err.empty()) {
    std::cerr << "ERR: " << err << std::endl;
  }
  TEST_CHECK(true == ret);
  TEST_CHECK(1 == materials.size());
  TEST_CHECK(0 == materials[0].diffuse_texname.compare("tmp.png"));
}

void test_transmittance_filter_issue95() {
  tinyobj::attrib_t attrib;
  std::vector<tinyobj::shape_t> shapes;
  std::vector<tinyobj::material_t> materials;

  std::string warn;
  std::string err;
  bool ret = tinyobj::LoadObj(&attrib, &shapes, &materials, &warn, &err,
                              "../models/issue-95.obj", gMtlBasePath);

  if (!warn.empty()) {
    std::cout << "WARN: " << warn << std::endl;
  }

  if (!err.empty()) {
    std::cerr << "ERR: " << err << std::endl;
  }
  TEST_CHECK(true == ret);
  TEST_CHECK(1 == materials.size());
  TEST_CHECK(FloatEquals(0.1f, materials[0].transmittance[0]));
  TEST_CHECK(FloatEquals(0.2f, materials[0].transmittance[1]));
  TEST_CHECK(FloatEquals(0.3f, materials[0].transmittance[2]));
}

void test_transmittance_filter_Tf_issue95() {
  tinyobj::attrib_t attrib;
  std::vector<tinyobj::shape_t> shapes;
  std::vector<tinyobj::material_t> materials;

  std::string warn;
  std::string err;
  bool ret = tinyobj::LoadObj(&attrib, &shapes, &materials, &warn, &err,
                              "../models/issue-95-2.obj", gMtlBasePath);

  if (!warn.empty()) {
    std::cout << "WARN: " << warn << std::endl;
  }

  if (!err.empty()) {
    std::cerr << "ERR: " << err << std::endl;
  }
  TEST_CHECK(true == ret);
  TEST_CHECK(1 == materials.size());
  TEST_CHECK(FloatEquals(0.1f, materials[0].transmittance[0]));
  TEST_CHECK(FloatEquals(0.2f, materials[0].transmittance[1]));
  TEST_CHECK(FloatEquals(0.3f, materials[0].transmittance[2]));
}

void test_transmittance_filter_Kt_issue95() {
  tinyobj::attrib_t attrib;
  std::vector<tinyobj::shape_t> shapes;
  std::vector<tinyobj::material_t> materials;

  std::string warn;
  std::string err;
  bool ret = tinyobj::LoadObj(&attrib, &shapes, &materials, &warn, &err,
                              "../models/issue-95.obj", gMtlBasePath);

  if (!warn.empty()) {
    std::cout << "WARN: " << warn << std::endl;
  }

  if (!err.empty()) {
    std::cerr << "ERR: " << err << std::endl;
  }
  TEST_CHECK(true == ret);
  TEST_CHECK(1 == materials.size());
  TEST_CHECK(FloatEquals(0.1f, materials[0].transmittance[0]));
  TEST_CHECK(FloatEquals(0.2f, materials[0].transmittance[1]));
  TEST_CHECK(FloatEquals(0.3f, materials[0].transmittance[2]));
}

void test_usemtl_at_last_line_issue104() {
  tinyobj::attrib_t attrib;
  std::vector<tinyobj::shape_t> shapes;
  std::vector<tinyobj::material_t> materials;

  std::string warn;
  std::string err;
  bool ret = tinyobj::LoadObj(&attrib, &shapes, &materials, &warn, &err,
                              "../models/usemtl-issue-104.obj", gMtlBasePath);

  if (!warn.empty()) {
    std::cout << "WARN: " << warn << std::endl;
  }

  if (!err.empty()) {
    std::cerr << "ERR: " << err << std::endl;
  }
  TEST_CHECK(true == ret);
  TEST_CHECK(1 == shapes.size());
}

void test_texture_opts_issue85() {
  tinyobj::attrib_t attrib;
  std::vector<tinyobj::shape_t> shapes;
  std::vector<tinyobj::material_t> materials;

  std::string warn;
  std::string err;
  bool ret =
      tinyobj::LoadObj(&attrib, &shapes, &materials, &warn, &err,
                       "../models/texture-options-issue-85.obj", gMtlBasePath);

  if (!warn.empty()) {
    std::cout << "WARN: " << warn << std::endl;
  }

  if (!err.empty()) {
    std::cerr << "ERR: " << err << std::endl;
  }
  TEST_CHECK(true == ret);
  TEST_CHECK(1 == shapes.size());
  TEST_CHECK(3 == materials.size());
  TEST_CHECK(0 == materials[0].name.compare("default"));
  TEST_CHECK(0 == materials[1].name.compare("bm2"));
  TEST_CHECK(0 == materials[2].name.compare("bm3"));
  TEST_CHECK(true == materials[0].ambient_texopt.clamp);
  TEST_CHECK(FloatEquals(0.1f, materials[0].diffuse_texopt.origin_offset[0]));
  TEST_CHECK(FloatEquals(0.0f, materials[0].diffuse_texopt.origin_offset[1]));
  TEST_CHECK(FloatEquals(0.0f, materials[0].diffuse_texopt.origin_offset[2]));
  TEST_CHECK(FloatEquals(0.1f, materials[0].specular_texopt.scale[0]));
  TEST_CHECK(FloatEquals(0.2f, materials[0].specular_texopt.scale[1]));
  TEST_CHECK(FloatEquals(1.0f, materials[0].specular_texopt.scale[2]));
  TEST_CHECK(
      FloatEquals(0.1f, materials[0].specular_highlight_texopt.turbulence[0]));
  TEST_CHECK(
      FloatEquals(0.2f, materials[0].specular_highlight_texopt.turbulence[1]));
  TEST_CHECK(
      FloatEquals(0.3f, materials[0].specular_highlight_texopt.turbulence[2]));
  TEST_CHECK(FloatEquals(3.0f, materials[0].bump_texopt.bump_multiplier));

  TEST_CHECK(
      FloatEquals(0.1f, materials[1].specular_highlight_texopt.brightness));
  TEST_CHECK(
      FloatEquals(0.3f, materials[1].specular_highlight_texopt.contrast));
  TEST_CHECK('r' == materials[1].bump_texopt.imfchan);

  TEST_CHECK(tinyobj::TEXTURE_TYPE_SPHERE == materials[2].diffuse_texopt.type);
  TEST_CHECK(tinyobj::TEXTURE_TYPE_CUBE_TOP ==
             materials[2].specular_texopt.type);
  TEST_CHECK(tinyobj::TEXTURE_TYPE_CUBE_BOTTOM ==
             materials[2].specular_highlight_texopt.type);
  TEST_CHECK(tinyobj::TEXTURE_TYPE_CUBE_LEFT ==
             materials[2].ambient_texopt.type);
  TEST_CHECK(tinyobj::TEXTURE_TYPE_CUBE_RIGHT ==
             materials[2].alpha_texopt.type);
  TEST_CHECK(tinyobj::TEXTURE_TYPE_CUBE_FRONT == materials[2].bump_texopt.type);
  TEST_CHECK(tinyobj::TEXTURE_TYPE_CUBE_BACK ==
             materials[2].displacement_texopt.type);
}

void test_mtllib_multiple_filenames_issue112() {
  tinyobj::attrib_t attrib;
  std::vector<tinyobj::shape_t> shapes;
  std::vector<tinyobj::material_t> materials;

  std::string warn;
  std::string err;
  bool ret = tinyobj::LoadObj(&attrib, &shapes, &materials, &warn, &err,
                              "../models/mtllib-multiple-files-issue-112.obj",
                              gMtlBasePath);

  if (!warn.empty()) {
    std::cout << "WARN: " << warn << std::endl;
  }

  if (!err.empty()) {
    std::cerr << "ERR: " << err << std::endl;
  }
  TEST_CHECK(true == ret);
  TEST_CHECK(1 == materials.size());
}

void test_tr_and_d_issue43() {
  tinyobj::attrib_t attrib;
  std::vector<tinyobj::shape_t> shapes;
  std::vector<tinyobj::material_t> materials;

  std::string warn;
  std::string err;
  bool ret = tinyobj::LoadObj(&attrib, &shapes, &materials, &warn, &err,
                              "../models/tr-and-d-issue-43.obj", gMtlBasePath);

  if (!warn.empty()) {
    std::cout << "WARN: " << warn << std::endl;
  }

  if (!err.empty()) {
    std::cerr << "ERR: " << err << std::endl;
  }
  TEST_CHECK(true == ret);
  TEST_CHECK(2 == materials.size());

  TEST_CHECK(FloatEquals(0.75f, materials[0].dissolve));
  TEST_CHECK(FloatEquals(0.75f, materials[1].dissolve));
}

void test_refl() {
  tinyobj::attrib_t attrib;
  std::vector<tinyobj::shape_t> shapes;
  std::vector<tinyobj::material_t> materials;

  std::string warn;
  std::string err;
  bool ret = tinyobj::LoadObj(&attrib, &shapes, &materials, &warn, &err,
                              "../models/refl.obj", gMtlBasePath);

  if (!warn.empty()) {
    std::cout << "WARN: " << warn << std::endl;
  }

  if (!err.empty()) {
    std::cerr << "ERR: " << err << std::endl;
  }

  PrintInfo(attrib, shapes, materials);

  TEST_CHECK(true == ret);
  TEST_CHECK(5 == materials.size());

  TEST_CHECK(materials[0].reflection_texname.compare("reflection.tga") == 0);
}

void test_map_Bump() {
  tinyobj::attrib_t attrib;
  std::vector<tinyobj::shape_t> shapes;
  std::vector<tinyobj::material_t> materials;

  std::string warn;
  std::string err;
  bool ret = tinyobj::LoadObj(&attrib, &shapes, &materials, &warn, &err,
                              "../models/map-bump.obj", gMtlBasePath);

  if (!warn.empty()) {
    std::cout << "WARN: " << warn << std::endl;
  }

  if (!err.empty()) {
    std::cerr << "ERR: " << err << std::endl;
  }

  PrintInfo(attrib, shapes, materials);

  TEST_CHECK(true == ret);
  TEST_CHECK(2 == materials.size());

  TEST_CHECK(materials[0].bump_texname.compare("bump.jpg") == 0);
}

void test_g_ignored_issue138() {
  tinyobj::attrib_t attrib;
  std::vector<tinyobj::shape_t> shapes;
  std::vector<tinyobj::material_t> materials;

  std::string warn;
  std::string err;
  bool ret = tinyobj::LoadObj(&attrib, &shapes, &materials, &warn, &err,
                              "../models/issue-138.obj", gMtlBasePath);

  if (!warn.empty()) {
    std::cout << "WARN: " << warn << std::endl;
  }

  if (!err.empty()) {
    std::cerr << "ERR: " << err << std::endl;
  }

  PrintInfo(attrib, shapes, materials);

  TEST_CHECK(true == ret);
  TEST_CHECK(2 == shapes.size());
  TEST_CHECK(2 == materials.size());
}

void test_vertex_col_ext_issue144() {
  tinyobj::attrib_t attrib;
  std::vector<tinyobj::shape_t> shapes;
  std::vector<tinyobj::material_t> materials;

  std::string warn;
  std::string err;

  bool ret = tinyobj::LoadObj(&attrib, &shapes, &materials, &warn, &err,
                              "../models/cube-vertexcol.obj", gMtlBasePath);

  if (!warn.empty()) {
    std::cout << "WARN: " << warn << std::endl;
  }

  if (!err.empty()) {
    std::cerr << "ERR: " << err << std::endl;
  }

  // PrintInfo(attrib, shapes, materials);

  TEST_CHECK(true == ret);
  TEST_CHECK((8 * 3) == attrib.colors.size());

  TEST_CHECK(FloatEquals(0.0f, attrib.colors[3 * 0 + 0]));
  TEST_CHECK(FloatEquals(0.0f, attrib.colors[3 * 0 + 1]));
  TEST_CHECK(FloatEquals(0.0f, attrib.colors[3 * 0 + 2]));

  TEST_CHECK(FloatEquals(0.0f, attrib.colors[3 * 1 + 0]));
  TEST_CHECK(FloatEquals(0.0f, attrib.colors[3 * 1 + 1]));
  TEST_CHECK(FloatEquals(1.0f, attrib.colors[3 * 1 + 2]));

  TEST_CHECK(FloatEquals(1.0f, attrib.colors[3 * 4 + 0]));

  TEST_CHECK(FloatEquals(1.0f, attrib.colors[3 * 7 + 0]));
  TEST_CHECK(FloatEquals(1.0f, attrib.colors[3 * 7 + 1]));
  TEST_CHECK(FloatEquals(1.0f, attrib.colors[3 * 7 + 2]));
}

void test_norm_texopts() {
  tinyobj::attrib_t attrib;
  std::vector<tinyobj::shape_t> shapes;
  std::vector<tinyobj::material_t> materials;

  std::string warn;
  std::string err;
  bool ret = tinyobj::LoadObj(&attrib, &shapes, &materials, &warn, &err,
                              "../models/norm-texopt.obj", gMtlBasePath);

  if (!warn.empty()) {
    std::cout << "WARN: " << warn << std::endl;
  }

  if (!err.empty()) {
    std::cerr << "ERR: " << err << std::endl;
  }

  TEST_CHECK(true == ret);
  TEST_CHECK(1 == shapes.size());
  TEST_CHECK(1 == materials.size());
  TEST_CHECK(FloatEquals(3.0f, materials[0].normal_texopt.bump_multiplier));
}

void test_zero_face_idx_value_issue140() {
  tinyobj::attrib_t attrib;
  std::vector<tinyobj::shape_t> shapes;
  std::vector<tinyobj::material_t> materials;

  std::string warn;
  std::string err;
  bool ret =
      tinyobj::LoadObj(&attrib, &shapes, &materials, &warn, &err,
                       "../models/issue-140-zero-face-idx.obj", gMtlBasePath);

  if (!warn.empty()) {
    std::cout << "WARN: " << warn << std::endl;
  }

  if (!err.empty()) {
    std::cerr << "ERR: " << err << std::endl;
  }
  TEST_CHECK(false == ret);
  TEST_CHECK(!err.empty());
}

void test_texture_name_whitespace_issue145() {
  tinyobj::attrib_t attrib;
  std::vector<tinyobj::shape_t> shapes;
  std::vector<tinyobj::material_t> materials;

  std::string warn;
  std::string err;
  bool ret = tinyobj::LoadObj(&attrib, &shapes, &materials, &warn, &err,
                              "../models/texture-filename-with-whitespace.obj",
                              gMtlBasePath);

  if (!warn.empty()) {
    std::cout << "WARN: " << warn << std::endl;
  }

  if (!err.empty()) {
    std::cerr << "ERR: " << err << std::endl;
  }

  TEST_CHECK(true == ret);
  TEST_CHECK(err.empty());
  TEST_CHECK(2 < materials.size());

  TEST_CHECK(0 == materials[0].diffuse_texname.compare("texture 01.png"));
  TEST_CHECK(0 == materials[1].bump_texname.compare("bump 01.png"));
  TEST_CHECK(FloatEquals(2.0f, materials[1].bump_texopt.bump_multiplier));
}

void test_smoothing_group_issue162() {
  tinyobj::attrib_t attrib;
  std::vector<tinyobj::shape_t> shapes;
  std::vector<tinyobj::material_t> materials;

  std::string warn;
  std::string err;
  bool ret =
      tinyobj::LoadObj(&attrib, &shapes, &materials, &warn, &err,
                       "../models/issue-162-smoothing-group.obj", gMtlBasePath);

  if (!warn.empty()) {
    std::cout << "WARN: " << warn << std::endl;
  }

  if (!err.empty()) {
    std::cerr << "ERR: " << err << std::endl;
  }

  TEST_CHECK(true == ret);
  TEST_CHECK(2 == shapes.size());

  TEST_CHECK(2 == shapes[0].mesh.smoothing_group_ids.size());
  TEST_CHECK(1 == shapes[0].mesh.smoothing_group_ids[0]);
  TEST_CHECK(1 == shapes[0].mesh.smoothing_group_ids[1]);

  TEST_CHECK(10 == shapes[1].mesh.smoothing_group_ids.size());
  TEST_CHECK(0 == shapes[1].mesh.smoothing_group_ids[0]);
  TEST_CHECK(0 == shapes[1].mesh.smoothing_group_ids[1]);
  TEST_CHECK(3 == shapes[1].mesh.smoothing_group_ids[2]);
  TEST_CHECK(3 == shapes[1].mesh.smoothing_group_ids[3]);
  TEST_CHECK(4 == shapes[1].mesh.smoothing_group_ids[4]);
  TEST_CHECK(4 == shapes[1].mesh.smoothing_group_ids[5]);
  TEST_CHECK(0 == shapes[1].mesh.smoothing_group_ids[6]);
  TEST_CHECK(0 == shapes[1].mesh.smoothing_group_ids[7]);
  TEST_CHECK(6 == shapes[1].mesh.smoothing_group_ids[8]);
  TEST_CHECK(6 == shapes[1].mesh.smoothing_group_ids[9]);
}

void test_invalid_face_definition() {
  tinyobj::attrib_t attrib;
  std::vector<tinyobj::shape_t> shapes;
  std::vector<tinyobj::material_t> materials;

  std::string warn;
  std::string err;
  bool ret =
      tinyobj::LoadObj(&attrib, &shapes, &materials, &warn, &err,
                       "../models/invalid-face-definition.obj", gMtlBasePath);

  if (!warn.empty()) {
    std::cout << "WARN: " << warn << std::endl;
  }

  if (!err.empty()) {
    std::cerr << "ERR: " << err << std::endl;
  }

  TEST_CHECK(true == ret);
  TEST_CHECK(1 == shapes.size());
  TEST_CHECK(0 == shapes[0].mesh.indices.size());
}

void test_Empty_mtl_basedir_issue177() {
  tinyobj::attrib_t attrib;
  std::vector<tinyobj::shape_t> shapes;
  std::vector<tinyobj::material_t> materials;

  std::string warn;
  std::string err;

  // A case where the user explicitly provides an empty string
  // Win32 specific?
  const char* userBaseDir = "";
  bool ret = tinyobj::LoadObj(&attrib, &shapes, &materials, &warn, &err,
                              "issue-177.obj", userBaseDir);

  // if mtl loading fails, we get an warning message here
  ret &= warn.empty();

  if (!warn.empty()) {
    std::cout << "WARN: " << warn << std::endl;
  }

  if (!err.empty()) {
    std::cerr << "ERR: " << err << std::endl;
  }

  TEST_CHECK(true == ret);
}

void test_line_primitive() {
  tinyobj::attrib_t attrib;
  std::vector<tinyobj::shape_t> shapes;
  std::vector<tinyobj::material_t> materials;

  std::string warn;
  std::string err;
  bool ret = tinyobj::LoadObj(&attrib, &shapes, &materials, &warn, &err,
                              "../models/line-prim.obj", gMtlBasePath);

  if (!warn.empty()) {
    std::cout << "WARN: " << warn << std::endl;
  }

  if (!err.empty()) {
    std::cerr << "ERR: " << err << std::endl;
  }

  TEST_CHECK(true == ret);
  TEST_CHECK(1 == shapes.size());
  TEST_CHECK(8 == shapes[0].lines.indices.size());
  TEST_CHECK(2 == shapes[0].lines.num_line_vertices.size());
}

void test_points_primitive() {
  tinyobj::attrib_t attrib;
  std::vector<tinyobj::shape_t> shapes;
  std::vector<tinyobj::material_t> materials;

  std::string warn;
  std::string err;
  bool ret = tinyobj::LoadObj(&attrib, &shapes, &materials, &warn, &err,
                              "../models/points-prim.obj", gMtlBasePath);

  if (!warn.empty()) {
    std::cout << "WARN: " << warn << std::endl;
  }

  if (!err.empty()) {
    std::cerr << "ERR: " << err << std::endl;
  }

  TEST_CHECK(true == ret);
  TEST_CHECK(1 == shapes.size());
  TEST_CHECK(8 == shapes[0].points.indices.size());
}

void test_multiple_group_names() {
  tinyobj::attrib_t attrib;
  std::vector<tinyobj::shape_t> shapes;
  std::vector<tinyobj::material_t> materials;

  std::string warn;
  std::string err;
  bool ret = tinyobj::LoadObj(&attrib, &shapes, &materials, &warn, &err,
                              "../models/cube.obj", gMtlBasePath);

  if (!warn.empty()) {
    std::cout << "WARN: " << warn << std::endl;
  }

  if (!err.empty()) {
    std::cerr << "ERR: " << err << std::endl;
  }

  TEST_CHECK(true == ret);
  TEST_CHECK(6 == shapes.size());
  TEST_CHECK(0 == shapes[0].name.compare("front cube"));
  TEST_CHECK(0 == shapes[1].name.compare("back cube"));  // multiple whitespaces
                                                         // are aggregated as
                                                         // single white space.
}

void test_initialize_all_texopts() {
  tinyobj::attrib_t attrib;
  std::vector<tinyobj::shape_t> shapes;
  std::vector<tinyobj::material_t> materials;

  std::string warn;
  std::string err;
  bool ret = tinyobj::LoadObj(&attrib, &shapes, &materials, &warn, &err,
                              "../models/cornell_box.obj", gMtlBasePath, false);

  TEST_CHECK(ret == true);
  TEST_CHECK(0 < materials.size());

#define TEST_CHECK_DEFAULT_TEXOPT(texopt)                \
  TEST_CHECK(tinyobj::TEXTURE_TYPE_NONE == texopt.type); \
  TEST_CHECK(0.0 == texopt.brightness);                  \
  TEST_CHECK(1.0 == texopt.contrast);                    \
  TEST_CHECK(false == texopt.clamp);                     \
  TEST_CHECK(true == texopt.blendu);                     \
  TEST_CHECK(true == texopt.blendv);                     \
  TEST_CHECK(1.0 == texopt.bump_multiplier);             \
  for (int j = 0; j < 3; j++) {                          \
    TEST_CHECK(0.0 == texopt.origin_offset[j]);          \
    TEST_CHECK(1.0 == texopt.scale[j]);                  \
    TEST_CHECK(0.0 == texopt.turbulence[j]);             \
  }
  for (size_t i = 0; i < materials.size(); i++) {
    TEST_CHECK_DEFAULT_TEXOPT(materials[i].ambient_texopt);
    TEST_CHECK_DEFAULT_TEXOPT(materials[i].diffuse_texopt);
    TEST_CHECK_DEFAULT_TEXOPT(materials[i].specular_texopt);
    TEST_CHECK_DEFAULT_TEXOPT(materials[i].specular_highlight_texopt);
    TEST_CHECK_DEFAULT_TEXOPT(materials[i].bump_texopt);
    TEST_CHECK_DEFAULT_TEXOPT(materials[i].displacement_texopt);
    TEST_CHECK_DEFAULT_TEXOPT(materials[i].alpha_texopt);
    TEST_CHECK_DEFAULT_TEXOPT(materials[i].reflection_texopt);
    TEST_CHECK_DEFAULT_TEXOPT(materials[i].roughness_texopt);
    TEST_CHECK_DEFAULT_TEXOPT(materials[i].metallic_texopt);
    TEST_CHECK_DEFAULT_TEXOPT(materials[i].sheen_texopt);
    TEST_CHECK_DEFAULT_TEXOPT(materials[i].emissive_texopt);
    TEST_CHECK_DEFAULT_TEXOPT(materials[i].normal_texopt);
  }
#undef TEST_CHECK_DEFAULT_TEXOPT
}

void test_colorspace_issue184() {
  tinyobj::attrib_t attrib;
  std::vector<tinyobj::shape_t> shapes;
  std::vector<tinyobj::material_t> materials;

  std::string warn;
  std::string err;
  bool ret =
      tinyobj::LoadObj(&attrib, &shapes, &materials, &warn, &err,
                       "../models/colorspace-issue-184.obj", gMtlBasePath);

  if (!warn.empty()) {
    std::cout << "WARN: " << warn << std::endl;
  }

  if (!err.empty()) {
    std::cerr << "ERR: " << err << std::endl;
  }

  TEST_CHECK(true == ret);
  TEST_CHECK(1 == shapes.size());
  TEST_CHECK(1 == materials.size());
  TEST_CHECK(0 == materials[0].diffuse_texopt.colorspace.compare("sRGB"));
  TEST_CHECK(0 == materials[0].specular_texopt.colorspace.size());
  TEST_CHECK(0 == materials[0].bump_texopt.colorspace.compare("linear"));
}

void test_leading_decimal_dots_issue201() {
  tinyobj::attrib_t attrib;
  std::vector<tinyobj::shape_t> shapes;
  std::vector<tinyobj::material_t> materials;

  std::string warn;
  std::string err;
  bool ret = tinyobj::LoadObj(&attrib, &shapes, &materials, &warn, &err,
                              "../models/leading-decimal-dot-issue-201.obj",
                              gMtlBasePath);

  if (!warn.empty()) {
    std::cout << "WARN: " << warn << std::endl;
  }

  if (!err.empty()) {
    std::cerr << "ERR: " << err << std::endl;
  }

  TEST_CHECK(true == ret);
  TEST_CHECK(1 == shapes.size());
  TEST_CHECK(1 == materials.size());
  TEST_CHECK(FloatEquals(0.8e-1f, attrib.vertices[0]));
  TEST_CHECK(FloatEquals(-.7e+2f, attrib.vertices[1]));
  TEST_CHECK(FloatEquals(.575869f, attrib.vertices[3]));
  TEST_CHECK(FloatEquals(-.666304f, attrib.vertices[4]));
  TEST_CHECK(FloatEquals(.940448f, attrib.vertices[6]));
}

void test_mtl_default_search_path_v2_API_issue208() {
  tinyobj::ObjReader reader;

  bool ret = reader.ParseFromFile("../models/cornell_box.obj");

  std::cout << "WARN: " << reader.Warning() << "\n";

  TEST_CHECK(ret == true);
  TEST_CHECK(reader.Warning().empty());
}

void test_leading_zero_in_exponent_notation_issue210() {
  tinyobj::attrib_t attrib;
  std::vector<tinyobj::shape_t> shapes;
  std::vector<tinyobj::material_t> materials;

  std::string warn;
  std::string err;
  bool ret = tinyobj::LoadObj(
      &attrib, &shapes, &materials, &warn, &err,
      "../models/leading-zero-in-exponent-notation-issue-210.obj",
      gMtlBasePath);

  if (!warn.empty()) {
    std::cout << "WARN: " << warn << std::endl;
  }

  if (!err.empty()) {
    std::cerr << "ERR: " << err << std::endl;
  }

  TEST_CHECK(true == ret);
  TEST_CHECK(1 == shapes.size());
  TEST_CHECK(1 == materials.size());
  TEST_CHECK(FloatEquals(0.8e-001f, attrib.vertices[0]));
  TEST_CHECK(FloatEquals(-.7e+02f, attrib.vertices[1]));

  std::cout << "exp " << 0.8e-01 << std::endl;
  std::cout << "bora " << attrib.vertices[0] << std::endl;
}

void test_usemtl_then_o_issue235() {
  tinyobj::attrib_t attrib;
  std::vector<tinyobj::shape_t> shapes;
  std::vector<tinyobj::material_t> materials;

  std::string warn;
  std::string err;
  bool ret = tinyobj::LoadObj(
      &attrib, &shapes, &materials, &warn, &err,
      "../models/issue-235-usemtl-then-o.obj",
      gMtlBasePath);

  if (!warn.empty()) {
    std::cout << "WARN: " << warn << std::endl;
  }

  if (!err.empty()) {
    std::cerr << "ERR: " << err << std::endl;
  }

  TEST_CHECK(true == ret);
  TEST_CHECK(2 == shapes.size());
  TEST_CHECK(2 == materials.size());
  TEST_CHECK(4 == shapes[1].mesh.indices[0].vertex_index);
}

void test_mtl_searchpaths_issue244() {
  tinyobj::attrib_t attrib;
  std::vector<tinyobj::shape_t> shapes;
  std::vector<tinyobj::material_t> materials;

  // .mtl is located at ./assets/issue-244.mtl
#if _WIN32
  std::string search_paths("../;../models;./assets");
#else
  std::string search_paths("../:../models:./assets");
#endif

  std::string warn;
  std::string err;
  bool ret = tinyobj::LoadObj(
      &attrib, &shapes, &materials, &warn, &err,
      "../models/issue-244-mtl-searchpaths.obj",
      search_paths.c_str());

  TEST_CHECK(warn.empty());

  if (!warn.empty()) {
    std::cout << "WARN: " << warn << std::endl;
  }

  if (!err.empty()) {
    std::cerr << "ERR: " << err << std::endl;
  }

  TEST_CHECK(true == ret);
  TEST_CHECK(2 == shapes.size());
  TEST_CHECK(2 == materials.size());
  TEST_CHECK(4 == shapes[1].mesh.indices[0].vertex_index);
}

void test_usemtl_whitespace_issue246() {
  tinyobj::attrib_t attrib;
  std::vector<tinyobj::shape_t> shapes;
  std::vector<tinyobj::material_t> materials;

  std::string warn;
  std::string err;
  bool ret = tinyobj::LoadObj(
      &attrib, &shapes, &materials, &warn, &err,
      "../models/issue-246-usemtl-whitespace.obj",
      gMtlBasePath);

  TEST_CHECK(warn.empty());

  if (!warn.empty()) {
    std::cout << "WARN: " << warn << std::endl;
  }

  if (!err.empty()) {
    std::cerr << "ERR: " << err << std::endl;
  }

  TEST_CHECK(true == ret);
  TEST_CHECK(1 == shapes.size());
  TEST_CHECK(1 == materials.size());
  TEST_CHECK(0 == shapes[0].mesh.material_ids[0]);
}

void test_texres_texopt_issue248() {
  tinyobj::attrib_t attrib;
  std::vector<tinyobj::shape_t> shapes;
  std::vector<tinyobj::material_t> materials;

  std::string warn;
  std::string err;
  bool ret = tinyobj::LoadObj(
      &attrib, &shapes, &materials, &warn, &err,
      "../models/issue-248-texres-texopt.obj",
      gMtlBasePath);

  TEST_CHECK(warn.empty());

  if (!warn.empty()) {
    std::cout << "WARN: " << warn << std::endl;
  }

  if (!err.empty()) {
    std::cerr << "ERR: " << err << std::endl;
  }

  TEST_CHECK(true == ret);
  TEST_CHECK(1 < materials.size());
  TEST_CHECK(512 == materials[0].diffuse_texopt.texture_resolution);
  TEST_CHECK("input.jpg" == materials[0].diffuse_texname);
}

void test_mtl_filename_with_whitespace_issue46() {
  tinyobj::attrib_t attrib;
  std::vector<tinyobj::shape_t> shapes;
  std::vector<tinyobj::material_t> materials;

  std::string warn;
  std::string err;
  bool ret =
      tinyobj::LoadObj(&attrib, &shapes, &materials, &warn, &err,
                       "../models/mtl filename with whitespace issue46.obj",
                       gMtlBasePath);

  if (!warn.empty()) {
    std::cout << "WARN: " << warn << std::endl;
  }

  if (!err.empty()) {
    std::cerr << "ERR: " << err << std::endl;
  }
  TEST_CHECK(true == ret);
  TEST_CHECK(1 == materials.size());
  TEST_CHECK("green" == materials[0].name);
}

void test_face_missing_issue295() {
  tinyobj::attrib_t attrib;
  std::vector<tinyobj::shape_t> shapes;
  std::vector<tinyobj::material_t> materials;

  std::string warn;
  std::string err;
  bool ret = tinyobj::LoadObj(
      &attrib, &shapes, &materials, &warn, &err,
      "../models/issue-295-trianguation-failure.obj",
      gMtlBasePath, /* triangualte */true);

  TEST_CHECK(warn.empty());

  if (!warn.empty()) {
    std::cout << "WARN: " << warn << std::endl;
  }

  if (!err.empty()) {
    std::cerr << "ERR: " << err << std::endl;
  }

  TEST_CHECK(true == ret);
  TEST_CHECK(1 == shapes.size());

  // 14 quad faces are triangulated into 28 triangles.
  TEST_CHECK(28 == shapes[0].mesh.num_face_vertices.size());
  TEST_CHECK(28 == shapes[0].mesh.smoothing_group_ids.size());
  TEST_CHECK(28 == shapes[0].mesh.material_ids.size());
  TEST_CHECK((3 * 28) == shapes[0].mesh.indices.size()); // 28 triangle faces x 3
}

// Fuzzer test.
// Just check if it does not crash.
// Disable by default since Windows filesystem can't create filename of afl
// testdata
#if 0

void test_afl000000", "[AFL]() {
  tinyobj::attrib_t attrib;
  std::vector<tinyobj::shape_t> shapes;
  std::vector<tinyobj::material_t> materials;

  std::string err;
  bool ret = tinyobj::LoadObj(&attrib, &shapes, &materials, &err, "./afl/id:000000,sig:11,src:000000,op:havoc,rep:128", gMtlBasePath);

  TEST_CHECK(true == ret);
}

void test_afl000001", "[AFL]() {
  tinyobj::attrib_t attrib;
  std::vector<tinyobj::shape_t> shapes;
  std::vector<tinyobj::material_t> materials;

  std::string err;
  bool ret = tinyobj::LoadObj(&attrib, &shapes, &materials, &err, "./afl/id:000001,sig:11,src:000000,op:havoc,rep:64", gMtlBasePath);

  TEST_CHECK(true == ret);
}
#endif

#if 0
int
main(
  int argc,
  char **argv)
{
  if (argc > 1) {
    const char* basepath = NULL;
    if (argc > 2) {
      basepath = argv[2];
    }
    assert(true == TestLoadObj(argv[1], basepath));
  } else {
    //assert(true == TestLoadObj("cornell_box.obj"));
    //assert(true == TestLoadObj("cube.obj"));
    assert(true == TestStreamLoadObj());
    assert(true == TestLoadObj("catmark_torus_creases0.obj", NULL, false));
  }

  return 0;
}
#endif

TEST_LIST = {
    {"cornell_box", test_cornell_box},
    {"catmark_torus_creases0", test_catmark_torus_creases0},
    {"pbr", test_pbr},
    {"stream_load", test_stream_load},
    {"stream_load_from_file_skipping_materials",
     test_stream_load_from_file_skipping_materials},
    {"stream_load_from_file_with_materials",
     test_stream_load_from_file_with_materials},
    {"trailing_whitespace_in_mtl_issue92",
     test_trailing_whitespace_in_mtl_issue92},
    {"transmittance_filter_issue95", test_transmittance_filter_issue95},
    {"transmittance_filter_Tf_issue95", test_transmittance_filter_Tf_issue95},
    {"transmittance_filter_Kt_issue95", test_transmittance_filter_Kt_issue95},
    {"usemtl_at_last_line_issue104", test_usemtl_at_last_line_issue104},
    {"texture_opts_issue85", test_texture_opts_issue85},
    {"mtllib_multiple_filenames_issue112",
     test_mtllib_multiple_filenames_issue112},
    {"tr_and_d_issue43", test_tr_and_d_issue43},
    {"refl", test_refl},
    {"map_bump", test_map_Bump},
    {"g_ignored_issue138", test_g_ignored_issue138},
    {"vertex_col_ext_issue144", test_vertex_col_ext_issue144},
    {"norm_texopts", test_norm_texopts},
    {"zero_face_idx_value_issue140", test_zero_face_idx_value_issue140},
    {"texture_name_whitespace_issue145", test_texture_name_whitespace_issue145},
    {"smoothing_group_issue162", test_smoothing_group_issue162},
    {"invalid_face_definition", test_invalid_face_definition},
    {"Empty_mtl_basedir_issue177", test_Empty_mtl_basedir_issue177},
    {"line_primitive", test_line_primitive},
    {"points_primitive", test_points_primitive},
    {"multiple_group_names", test_multiple_group_names},
    {"initialize_all_texopts", test_initialize_all_texopts},
    {"colorspace_issue184", test_colorspace_issue184},
    {"leading_decimal_dots_issue201", test_leading_decimal_dots_issue201},
    {"mtl_default_search_path_v2_API_issue208",
     test_mtl_default_search_path_v2_API_issue208},
    {"leading_zero_in_exponent_notation_issue210",
     test_leading_zero_in_exponent_notation_issue210},
    {"usemtl_then_o_issue235",
     test_usemtl_then_o_issue235},
    {"mtl_searchpaths_issue244",
     test_mtl_searchpaths_issue244},
    {"usemtl_whitespece_issue246",
     test_usemtl_whitespace_issue246},
    {"texres_texopt_issue248",
     test_texres_texopt_issue248},
    {"test_mtl_filename_with_whitespace_issue46",
     test_mtl_filename_with_whitespace_issue46},
    {"test_face_missing_issue295",
     test_face_missing_issue295},
    {NULL, NULL}};
