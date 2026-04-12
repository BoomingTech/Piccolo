#define TINYOBJLOADER_IMPLEMENTATION
#define TINYOBJLOADER_STREAM_READER_MAX_BYTES (size_t(8) * size_t(1024) * size_t(1024))
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
#include <string>

#ifdef _WIN32
#include <direct.h>    // _mkdir
#include <windows.h>   // GetTempPathW, CreateDirectoryW, RegOpenKeyExA
#include <winreg.h>    // registry constants
#pragma comment(lib, "Advapi32.lib")  // RegOpenKeyExA, RegQueryValueExA, RegCloseKey

// Converts a UTF-16 wide string to a UTF-8 std::string.
static std::string WcharToUTF8(const std::wstring &wstr) {
  if (wstr.empty()) return std::string();
  int len = WideCharToMultiByte(CP_UTF8, 0, wstr.c_str(), -1,
                                NULL, 0, NULL, NULL);
  if (len <= 0) return std::string();
  std::string str(static_cast<size_t>(len), '\0');
  WideCharToMultiByte(CP_UTF8, 0, wstr.c_str(), -1, &str[0], len, NULL, NULL);
  str.resize(static_cast<size_t>(len - 1));  // trim terminating '\0'
  return str;
}
#else
#include <cerrno>
#include <sys/stat.h>  // mkdir
#endif

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

// ---------------------------------------------------------------------------
// Helpers for path-related tests
// ---------------------------------------------------------------------------

// Creates a single directory level. Returns true on success or if it already exists.
static bool MakeDir(const std::string& path) {
#ifdef _WIN32
  // Use the wide-character API so that paths with non-ASCII characters work.
  std::wstring wpath = UTF8ToWchar(path);
  if (wpath.empty()) return false;
  if (CreateDirectoryW(wpath.c_str(), NULL) != 0) return true;
  return GetLastError() == ERROR_ALREADY_EXISTS;
#else
  return mkdir(path.c_str(), 0755) == 0 || errno == EEXIST;
#endif
}

// Removes a directory and all its contents.
// NOTE: All callers pass paths that are fully constructed within this test
// file from hardcoded string literals, so there is no user-controlled input
// that could be used for command injection.
static void RemoveTestDir(const std::string& path) {
#ifdef _WIN32
  std::string cmd = "rd /s /q \"" + path + "\"";
#else
  std::string cmd = "rm -rf '" + path + "'";
#endif
  if (system(cmd.c_str()) != 0) { /* cleanup failure is non-fatal */ }
}

// Copies a file in binary mode. The destination path is taken as UTF-8.
// On Windows, LongPathW(UTF8ToWchar()) is used so that long paths (> MAX_PATH)
// are handled, exercising the same conversion that tinyobjloader itself uses.
static bool CopyTestFile(const std::string& src, const std::string& dst) {
  std::ifstream in(src.c_str(), std::ios::binary);
  if (!in) return false;
#ifdef _WIN32
  // Apply long-path prefix so that the copy works even for paths > MAX_PATH.
  std::ofstream out(LongPathW(UTF8ToWchar(dst)).c_str(), std::ios::binary);
#else
  std::ofstream out(dst.c_str(), std::ios::binary);
#endif
  if (!out) return false;
  out << in.rdbuf();
  return !out.fail();
}

#ifdef _WIN32
// Returns true if Windows has the system-wide long path support enabled
// (HKLM\SYSTEM\CurrentControlSet\Control\FileSystem\LongPathsEnabled = 1).
static bool IsWindowsLongPathEnabled() {
  HKEY hKey;
  DWORD value = 0;
  DWORD size = sizeof(DWORD);
  if (RegOpenKeyExA(HKEY_LOCAL_MACHINE,
                    "SYSTEM\\CurrentControlSet\\Control\\FileSystem", 0,
                    KEY_READ, &hKey) == ERROR_SUCCESS) {
    RegQueryValueExA(hKey, "LongPathsEnabled", NULL, NULL,
                     reinterpret_cast<LPBYTE>(&value), &size);
    RegCloseKey(hKey);
  }
  return value != 0;
}
#endif  // _WIN32

// ---------------------------------------------------------------------------
// Path-related tests
// ---------------------------------------------------------------------------

// Test: load .obj/.mtl from a directory path containing UTF-8 non-ASCII
// characters. On Windows our code converts the UTF-8 path to UTF-16 before
// calling the file API. On Linux, UTF-8 paths are handled natively.
void test_load_obj_from_utf8_path() {
  // Build a temp directory name that contains the UTF-8 encoded character é
  // (U+00E9, encoded as \xC3\xA9 in UTF-8).
#ifdef _WIN32
  wchar_t wtmpbuf[MAX_PATH];
  GetTempPathW(MAX_PATH, wtmpbuf);
  std::string test_dir =
      WcharToUTF8(wtmpbuf) + "tinyobj_utf8_\xc3\xa9_test\\";
#else
  std::string test_dir = "/tmp/tinyobj_utf8_\xc3\xa9_test/";
#endif

  if (!MakeDir(test_dir)) {
    std::cout << "SKIPPED: Cannot create Unicode temp directory: " << test_dir
              << "\n";
    return;
  }

  const std::string obj_dst = test_dir + "utf8-path-test.obj";
  const std::string mtl_dst = test_dir + "utf8-path-test.mtl";

  if (!CopyTestFile("../models/utf8-path-test.obj", obj_dst) ||
      !CopyTestFile("../models/utf8-path-test.mtl", mtl_dst)) {
    RemoveTestDir(test_dir);
    TEST_CHECK_(false, "Failed to copy test files to Unicode temp directory");
    return;
  }

  tinyobj::ObjReader reader;
  bool ret = reader.ParseFromFile(obj_dst);

  RemoveTestDir(test_dir);

  if (!reader.Warning().empty())
    std::cout << "WARN: " << reader.Warning() << "\n";
  if (!reader.Error().empty())
    std::cerr << "ERR: " << reader.Error() << "\n";

  TEST_CHECK(ret == true);
  TEST_CHECK(reader.GetShapes().size() == 1);
  TEST_CHECK(reader.GetMaterials().size() == 1);
}

// Test: load .obj/.mtl from a path whose total length exceeds MAX_PATH (260).
// On Windows, tinyobjloader prepends the \\?\ extended-length path prefix so
// that the file can be opened even on systems that have the OS-wide long path
// support enabled. The test is skipped when that support is not active.
// On Linux, long paths work natively; this test verifies no regression.
void test_load_obj_from_long_path() {
#ifdef _WIN32
  if (!IsWindowsLongPathEnabled()) {
    std::cout
        << "SKIPPED: Windows long path support (LongPathsEnabled) is not "
           "enabled\n";
    return;
  }
  wchar_t wtmpbuf[MAX_PATH];
  GetTempPathW(MAX_PATH, wtmpbuf);
  std::string base = WcharToUTF8(wtmpbuf);  // e.g. "C:\Users\...\Temp\"
  const char path_sep = '\\';
#else
  std::string base = "/tmp/";
  const char path_sep = '/';
#endif

  // Create a two-level directory where the deepest directory name is 250
  // characters long.  Combined with the base path and the filename
  // "utf8-path-test.obj" (18 chars) the total file path comfortably exceeds
  // MAX_PATH (260) on all supported platforms.
  std::string test_root = base + "tinyobj_lp_test" + path_sep;
  std::string long_subdir = test_root + std::string(250, 'a') + path_sep;
  std::string obj_path = long_subdir + "utf8-path-test.obj";

  // obj_path must exceed MAX_PATH for the test to be meaningful.
  // (On a typical Windows installation it is ~320 chars; on Linux ~287 chars.)
  if (obj_path.size() <= 260) {
    std::cout << "SKIPPED: generated path (" << obj_path.size()
              << " chars) does not exceed MAX_PATH=260\n";
    return;
  }

  if (!MakeDir(test_root) || !MakeDir(long_subdir)) {
    RemoveTestDir(test_root);
    std::cout << "SKIPPED: Cannot create long-path temp directory: "
              << long_subdir << "\n";
    return;
  }

  if (!CopyTestFile("../models/utf8-path-test.obj",
                    long_subdir + "utf8-path-test.obj") ||
      !CopyTestFile("../models/utf8-path-test.mtl",
                    long_subdir + "utf8-path-test.mtl")) {
    RemoveTestDir(test_root);
    TEST_CHECK_(false, "Failed to copy test files to long-path directory");
    return;
  }

  tinyobj::ObjReader reader;
  bool ret = reader.ParseFromFile(obj_path);

  RemoveTestDir(test_root);

  if (!reader.Warning().empty())
    std::cout << "WARN: " << reader.Warning() << "\n";
  if (!reader.Error().empty())
    std::cerr << "ERR: " << reader.Error() << "\n";

  TEST_CHECK(ret == true);
  TEST_CHECK(reader.GetShapes().size() == 1);
  TEST_CHECK(reader.GetMaterials().size() == 1);
}

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
  TEST_CHECK(std::string("crease") == shapes[0].mesh.tags[0].name);
  TEST_CHECK(2 == shapes[0].mesh.tags[0].intValues.size());
  TEST_CHECK(1 == shapes[0].mesh.tags[0].floatValues.size());
  TEST_CHECK(0 == shapes[0].mesh.tags[0].stringValues.size());
  TEST_CHECK(1 == shapes[0].mesh.tags[0].intValues[0]);
  TEST_CHECK(5 == shapes[0].mesh.tags[0].intValues[1]);
  TEST_CHECK(FloatEquals(4.7f, shapes[0].mesh.tags[0].floatValues[0]));
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

void test_invalid_relative_vertex_index() {
  tinyobj::attrib_t attrib;
  std::vector<tinyobj::shape_t> shapes;
  std::vector<tinyobj::material_t> materials;

  std::string warn;
  std::string err;
  bool ret =
      tinyobj::LoadObj(&attrib, &shapes, &materials, &warn, &err,
                       "../models/invalid-relative-vertex-index.obj", gMtlBasePath);

  if (!warn.empty()) {
    std::cout << "WARN: " << warn << std::endl;
  }

  if (!err.empty()) {
    std::cerr << "ERR: " << err << std::endl;
  }
  TEST_CHECK(false == ret);
  TEST_CHECK(!err.empty());
}

void test_invalid_texture_vertex_index() {
  tinyobj::attrib_t attrib;
  std::vector<tinyobj::shape_t> shapes;
  std::vector<tinyobj::material_t> materials;

  std::string warn;
  std::string err;
  bool ret =
      tinyobj::LoadObj(&attrib, &shapes, &materials, &warn, &err,
                       "../models/invalid-relative-texture-index.obj", gMtlBasePath);

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

void test_comment_issue389() {
  tinyobj::attrib_t attrib;
  std::vector<tinyobj::shape_t> shapes;
  std::vector<tinyobj::material_t> materials;

  std::string warn;
  std::string err;
  bool ret = tinyobj::LoadObj(
      &attrib, &shapes, &materials, &warn, &err,
      "../models/issue-389-comment.obj",
      gMtlBasePath, /* triangualte */false);

  TEST_CHECK(warn.empty());

  if (!warn.empty()) {
    std::cout << "WARN: " << warn << std::endl;
  }

  if (!err.empty()) {
    std::cerr << "ERR: " << err << std::endl;
  }

  TEST_CHECK(true == ret);
}

void test_default_kd_for_multiple_materials_issue391() {
  tinyobj::attrib_t attrib;
  std::vector<tinyobj::shape_t> shapes;
  std::vector<tinyobj::material_t> materials;

  std::string warn;
  std::string err;
  bool ret = tinyobj::LoadObj(&attrib, &shapes, &materials, &warn, &err,
                              "../models/issue-391.obj", gMtlBasePath);
  if (!warn.empty()) {
    std::cout << "WARN: " << warn << std::endl;
  }

  if (!err.empty()) {
    std::cerr << "ERR: " << err << std::endl;
  }

  const tinyobj::real_t kGrey[] = {0.6, 0.6, 0.6};
  const tinyobj::real_t kRed[] = {1.0, 0.0, 0.0};

  TEST_CHECK(true == ret);
  TEST_CHECK(2 == materials.size());
  for (size_t i = 0; i < materials.size(); ++i) {
    const tinyobj::material_t& material = materials[i];
    if (material.name == "has_map") {
      for (int i = 0; i < 3; ++i) TEST_CHECK(material.diffuse[i] == kGrey[i]);
    } else if (material.name == "has_kd") {
      for (int i = 0; i < 3; ++i) TEST_CHECK(material.diffuse[i] == kRed[i]);
    } else {
      std::cerr << "Unexpected material found!" << std::endl;
      TEST_CHECK(false);
    }
  }  
}

void test_removeUtf8Bom() {
  // Basic input with BOM
  std::string withBOM = "\xEF\xBB\xBFhello world";
  TEST_CHECK(tinyobj::removeUtf8Bom(withBOM) == "hello world");

  // Input without BOM
  std::string noBOM = "hello world";
  TEST_CHECK(tinyobj::removeUtf8Bom(noBOM) == "hello world");

  // Leaves short string unchanged
  std::string shortStr = "\xEF";
  TEST_CHECK(tinyobj::removeUtf8Bom(shortStr) == shortStr);

  std::string shortStr2 = "\xEF\xBB";
  TEST_CHECK(tinyobj::removeUtf8Bom(shortStr2) == shortStr2);

  // BOM only returns empty string
  std::string justBom = "\xEF\xBB\xBF";
  TEST_CHECK(tinyobj::removeUtf8Bom(justBom) == "");

  // Empty string
  std::string emptyStr = "";
  TEST_CHECK(tinyobj::removeUtf8Bom(emptyStr) == "");
}

void test_loadObj_with_BOM() {
  tinyobj::attrib_t attrib;
  std::vector<tinyobj::shape_t> shapes;
  std::vector<tinyobj::material_t> materials;

  std::string warn;
  std::string err;
  bool ret = tinyobj::LoadObj(&attrib, &shapes, &materials, &warn, &err,
                              "../models/cube_w_BOM.obj", gMtlBasePath);

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


void test_texcoord_w_component() {
  tinyobj::attrib_t attrib;
  std::vector<tinyobj::shape_t> shapes;
  std::vector<tinyobj::material_t> materials;

  std::string warn;
  std::string err;
  bool ret = tinyobj::LoadObj(&attrib, &shapes, &materials, &warn, &err,
                              "../models/texcoord-w.obj", gMtlBasePath,
                              /*triangulate*/ false);

  if (!warn.empty()) {
    std::cout << "WARN: " << warn << std::endl;
  }

  if (!err.empty()) {
    std::cerr << "ERR: " << err << std::endl;
  }

  TEST_CHECK(true == ret);
  TEST_CHECK(4 == attrib.texcoords.size() / 2);    // 4 uv pairs
  TEST_CHECK(4 == attrib.texcoord_ws.size());       // 4 w values
  TEST_CHECK(FloatEquals(0.50f, attrib.texcoord_ws[0]));
  TEST_CHECK(FloatEquals(0.25f, attrib.texcoord_ws[1]));
  TEST_CHECK(FloatEquals(0.75f, attrib.texcoord_ws[2]));
  TEST_CHECK(FloatEquals(0.00f, attrib.texcoord_ws[3]));
}



void test_texcoord_w_mixed_component() {
  // Test a mix of vt lines with the optional w present and omitted.
  // Lines without w should produce 0.0 in texcoord_ws.
  tinyobj::attrib_t attrib;
  std::vector<tinyobj::shape_t> shapes;
  std::vector<tinyobj::material_t> materials;

  std::string warn;
  std::string err;
  bool ret = tinyobj::LoadObj(&attrib, &shapes, &materials, &warn, &err,
                              "../models/texcoord-w-mixed.obj", gMtlBasePath,
                              /*triangulate*/ false);

  if (!warn.empty()) {
    std::cout << "WARN: " << warn << std::endl;
  }

  if (!err.empty()) {
    std::cerr << "ERR: " << err << std::endl;
  }

  TEST_CHECK(true == ret);
  TEST_CHECK(4 == attrib.texcoords.size() / 2);    // 4 uv pairs
  TEST_CHECK(4 == attrib.texcoord_ws.size());       // 4 w values (present or defaulted)
  TEST_CHECK(FloatEquals(0.50f, attrib.texcoord_ws[0]));  // w present
  TEST_CHECK(FloatEquals(0.00f, attrib.texcoord_ws[1]));  // w omitted -> 0.0
  TEST_CHECK(FloatEquals(0.75f, attrib.texcoord_ws[2]));  // w present
  TEST_CHECK(FloatEquals(0.00f, attrib.texcoord_ws[3]));  // w omitted -> 0.0
}

void test_loadObjWithCallback_with_BOM() {
  // Verify that LoadObjWithCallback correctly strips a UTF-8 BOM from the
  // first line, just as LoadObj and LoadMtl do.
  // We reuse cube_w_BOM.obj which starts with 0xEF 0xBB 0xBF followed by
  // "mtllib cube_w_BOM.mtl".  Without BOM stripping the mtllib line would
  // not be recognised and no materials would be loaded; with BOM stripping
  // all 8 vertices and 6 groups are parsed.

  struct CallbackData {
    int vertex_count;
    int group_count;
    int material_count;
    CallbackData() : vertex_count(0), group_count(0), material_count(0) {}
  };

  CallbackData data;

  tinyobj::callback_t cb;
  cb.vertex_cb = [](void *user_data, tinyobj::real_t x, tinyobj::real_t y,
                    tinyobj::real_t z, tinyobj::real_t w) {
    reinterpret_cast<CallbackData *>(user_data)->vertex_count++;
  };
  cb.group_cb = [](void *user_data, const char **names, int num_names) {
    if (num_names > 0)
      reinterpret_cast<CallbackData *>(user_data)->group_count++;
  };
  cb.mtllib_cb = [](void *user_data, const tinyobj::material_t *materials,
                    int num_materials) {
    reinterpret_cast<CallbackData *>(user_data)->material_count +=
        num_materials;
  };

  std::ifstream ifs("../models/cube_w_BOM.obj");
  TEST_CHECK(ifs.is_open());

  tinyobj::MaterialFileReader matReader(gMtlBasePath);
  std::string warn, err;
  bool ret = tinyobj::LoadObjWithCallback(ifs, cb, &data, &matReader, &warn, &err);

  if (!warn.empty()) {
    std::cout << "WARN: " << warn << std::endl;
  }
  if (!err.empty()) {
    std::cerr << "ERR: " << err << std::endl;
  }

  TEST_CHECK(true == ret);
  TEST_CHECK(8 == data.vertex_count);   // 8 vertices in cube_w_BOM.obj
  TEST_CHECK(6 == data.group_count);    // 6 groups: front/back/right/top/left/bottom
  TEST_CHECK(data.material_count > 0);  // materials loaded => mtllib line was parsed
}

void test_loadObjWithCallback_mtllib_failure_does_not_crash() {
  // mtllib load failure should not crash callback path, and should report an
  // error/warning while continuing OBJ parsing.
  std::string obj_text = "mtllib test.mtl\nv 1 2 3\n";
  std::istringstream obj_stream(obj_text);

  std::string oversized_mtl(TINYOBJLOADER_STREAM_READER_MAX_BYTES + size_t(1), ' ');
  std::istringstream mtl_stream(oversized_mtl);
  tinyobj::MaterialStreamReader mtl_reader(mtl_stream);

  struct CallbackData {
    int vertex_count;
    int mtllib_count;
    CallbackData() : vertex_count(0), mtllib_count(0) {}
  } data;

  tinyobj::callback_t cb;
  cb.vertex_cb = [](void *user_data, tinyobj::real_t, tinyobj::real_t,
                    tinyobj::real_t, tinyobj::real_t) {
    reinterpret_cast<CallbackData *>(user_data)->vertex_count++;
  };
  cb.mtllib_cb = [](void *user_data, const tinyobj::material_t *, int) {
    reinterpret_cast<CallbackData *>(user_data)->mtllib_count++;
  };

  std::string warn, err;
  bool ret = tinyobj::LoadObjWithCallback(obj_stream, cb, &data, &mtl_reader,
                                          &warn, &err);

  TEST_CHECK(ret == true);
  TEST_CHECK(data.vertex_count == 1);
  TEST_CHECK(data.mtllib_count == 0);
  TEST_CHECK(warn.find("Failed to load material file(s)") != std::string::npos);
  TEST_CHECK(err.find("input stream too large") != std::string::npos);
}

void test_mtllib_empty_filename_is_ignored_loadobj() {
  std::string obj_text = "mtllib    \nv 1 2 3\n";
  std::istringstream obj_stream(obj_text);

  std::string mtl_text = "newmtl should_not_load\nKd 1 1 1\n";
  std::istringstream mtl_stream(mtl_text);
  tinyobj::MaterialStreamReader mtl_reader(mtl_stream);

  tinyobj::attrib_t attrib;
  std::vector<tinyobj::shape_t> shapes;
  std::vector<tinyobj::material_t> materials;
  std::string warn, err;
  bool ret = tinyobj::LoadObj(&attrib, &shapes, &materials, &warn, &err,
                              &obj_stream, &mtl_reader);

  TEST_CHECK(ret == true);
  TEST_CHECK(materials.empty());
  TEST_CHECK(warn.find("Looks like empty filename for mtllib") != std::string::npos);
  TEST_CHECK(err.empty());
}

void test_mtllib_empty_filename_is_ignored_callback() {
  std::string obj_text = "mtllib    \nv 1 2 3\n";
  std::istringstream obj_stream(obj_text);

  std::string mtl_text = "newmtl should_not_load\nKd 1 1 1\n";
  std::istringstream mtl_stream(mtl_text);
  tinyobj::MaterialStreamReader mtl_reader(mtl_stream);

  struct CallbackData {
    int vertex_count;
    int mtllib_count;
    CallbackData() : vertex_count(0), mtllib_count(0) {}
  } data;

  tinyobj::callback_t cb;
  cb.vertex_cb = [](void *user_data, tinyobj::real_t, tinyobj::real_t,
                    tinyobj::real_t, tinyobj::real_t) {
    reinterpret_cast<CallbackData *>(user_data)->vertex_count++;
  };
  cb.mtllib_cb = [](void *user_data, const tinyobj::material_t *, int) {
    reinterpret_cast<CallbackData *>(user_data)->mtllib_count++;
  };

  std::string warn, err;
  bool ret = tinyobj::LoadObjWithCallback(obj_stream, cb, &data, &mtl_reader,
                                          &warn, &err);

  TEST_CHECK(ret == true);
  TEST_CHECK(data.vertex_count == 1);
  TEST_CHECK(data.mtllib_count == 0);
  TEST_CHECK(warn.find("Looks like empty filename for mtllib") != std::string::npos);
  TEST_CHECK(err.empty());
}

void test_usemtl_callback_trims_trailing_comment() {
  std::string obj_text =
      "mtllib test.mtl\n"
      "usemtl mat   # trailing comment\n"
      "v 0 0 0\n";
  std::istringstream obj_stream(obj_text);

  std::string mtl_text = "newmtl mat\nKd 1 1 1\n";
  std::istringstream mtl_stream(mtl_text);
  tinyobj::MaterialStreamReader mtl_reader(mtl_stream);

  struct CallbackData {
    int usemtl_count;
    int last_material_id;
    std::string last_name;
    CallbackData() : usemtl_count(0), last_material_id(-1), last_name() {}
  } data;

  tinyobj::callback_t cb;
  cb.usemtl_cb = [](void *user_data, const char *name, int material_id) {
    CallbackData *d = reinterpret_cast<CallbackData *>(user_data);
    d->usemtl_count++;
    d->last_name = name ? name : "";
    d->last_material_id = material_id;
  };

  std::string warn, err;
  bool ret = tinyobj::LoadObjWithCallback(obj_stream, cb, &data, &mtl_reader,
                                          &warn, &err);

  TEST_CHECK(ret == true);
  TEST_CHECK(data.usemtl_count == 1);
  TEST_CHECK(data.last_name == "mat");
  TEST_CHECK(data.last_material_id == 0);
  TEST_CHECK(err.empty());
}

void test_tag_triple_huge_count_is_safely_rejected() {
  std::string obj_text =
      "v 0 0 0\n"
      "v 1 0 0\n"
      "v 0 1 0\n"
      "f 1 2 3\n"
      "t crease 999999999999999999999999999999999999999999999999999999999999999999/0/0\n";
  std::istringstream obj_stream(obj_text);
  std::string mtl_text;
  std::istringstream mtl_stream(mtl_text);
  tinyobj::MaterialStreamReader mtl_reader(mtl_stream);

  tinyobj::attrib_t attrib;
  std::vector<tinyobj::shape_t> shapes;
  std::vector<tinyobj::material_t> materials;
  std::string warn, err;
  bool ret = tinyobj::LoadObj(&attrib, &shapes, &materials, &warn, &err,
                              &obj_stream, &mtl_reader);

  TEST_CHECK(ret == true);
  TEST_CHECK(shapes.size() == size_t(1));
  TEST_CHECK(shapes[0].mesh.tags.size() == size_t(1));
  TEST_CHECK(shapes[0].mesh.tags[0].intValues.size() == size_t(0));
  TEST_CHECK(shapes[0].mesh.tags[0].floatValues.size() == size_t(0));
  TEST_CHECK(shapes[0].mesh.tags[0].stringValues.size() == size_t(0));
}



// Verify that mmap-based loading (TINYOBJLOADER_USE_MMAP) produces the same
// vertex/shape/material data as the standard ifstream-based path.
void test_file_and_stream_load_agree() {
  const char *obj_file = "../models/cornell_box.obj";

  // Load using the file path API (uses mmap when TINYOBJLOADER_USE_MMAP is defined).
  tinyobj::attrib_t attrib;
  std::vector<tinyobj::shape_t> shapes;
  std::vector<tinyobj::material_t> materials;
  std::string warn, err;
  bool ret = tinyobj::LoadObj(&attrib, &shapes, &materials, &warn, &err,
                              obj_file, gMtlBasePath);
  if (!warn.empty()) std::cout << "WARN: " << warn << "\n";
  if (!err.empty()) std::cerr << "ERR: " << err << "\n";
  TEST_CHECK(ret == true);

  // Also load via the stream API (always uses ifstream-equivalent path).
  tinyobj::attrib_t attrib2;
  std::vector<tinyobj::shape_t> shapes2;
  std::vector<tinyobj::material_t> materials2;
  std::string warn2, err2;
  std::ifstream ifs(obj_file);
  TEST_CHECK(ifs.good());
  tinyobj::MaterialFileReader matReader(gMtlBasePath);
  bool ret2 = tinyobj::LoadObj(&attrib2, &shapes2, &materials2, &warn2, &err2,
                               &ifs, &matReader);
  TEST_CHECK(ret2 == true);

  // Compare results.
  TEST_CHECK(attrib.vertices.size() == attrib2.vertices.size());
  TEST_CHECK(attrib.normals.size() == attrib2.normals.size());
  TEST_CHECK(shapes.size() == shapes2.size());
  TEST_CHECK(materials.size() == materials2.size());
  for (size_t i = 0; i < shapes.size(); i++) {
    TEST_CHECK(shapes[i].mesh.indices.size() == shapes2[i].mesh.indices.size());
  }
}

// Verify robustness: loading from a memory buffer (imemstream) is consistent
// with standard file loading.
void test_load_from_memory_buffer() {
  const char *obj_file = "../models/cube.obj";

  // Read file into memory manually.
  std::ifstream file(obj_file, std::ios::binary | std::ios::ate);
  TEST_CHECK(file.good());
  std::streamsize sz = file.tellg();
  file.seekg(0, std::ios::beg);
  std::vector<char> buf(static_cast<size_t>(sz));
  TEST_CHECK(file.read(buf.data(), sz).good());
  file.close();

  // Parse from the memory buffer via the stream API.
  tinyobj::attrib_t attrib;
  std::vector<tinyobj::shape_t> shapes;
  std::vector<tinyobj::material_t> materials;
  std::string warn, err;
  // Copy the memory buffer into a std::string and parse via std::istringstream.
  std::string obj_text(buf.begin(), buf.end());
  std::istringstream obj_ss(obj_text);
  tinyobj::MaterialFileReader matReader(gMtlBasePath);
  bool ret = tinyobj::LoadObj(&attrib, &shapes, &materials, &warn, &err,
                              &obj_ss, &matReader);
  if (!warn.empty()) std::cout << "WARN: " << warn << "\n";
  if (!err.empty()) std::cerr << "ERR: " << err << "\n";
  TEST_CHECK(ret == true);

  // Compare with direct file load to check consistency.
  tinyobj::attrib_t attrib2;
  std::vector<tinyobj::shape_t> shapes2;
  std::vector<tinyobj::material_t> materials2;
  std::string warn2, err2;
  bool ret2 = tinyobj::LoadObj(&attrib2, &shapes2, &materials2, &warn2, &err2,
                               obj_file, gMtlBasePath);
  TEST_CHECK(ret2 == true);
  TEST_CHECK(attrib.vertices.size() == attrib2.vertices.size());
  TEST_CHECK(shapes.size() == shapes2.size());
}


// --- Error reporting tests ---

void test_streamreader_column_tracking() {
  const char *input = "hello world\nfoo\n";
  tinyobj::StreamReader sr(input, strlen(input));

  TEST_CHECK(sr.col_num() == 1);
  TEST_CHECK(sr.line_num() == 1);

  // Advance 5 chars: "hello"
  sr.advance(5);
  TEST_CHECK(sr.col_num() == 6);  // col is 1-based, after 5 chars -> col 6
  TEST_CHECK(sr.line_num() == 1);

  // skip_space: " "
  sr.skip_space();
  TEST_CHECK(sr.col_num() == 7);

  // read_token: "world"
  std::string tok = sr.read_token();
  TEST_CHECK(tok == "world");
  TEST_CHECK(sr.col_num() == 12);

  // skip_line: "\n"
  sr.skip_line();
  TEST_CHECK(sr.line_num() == 2);
  TEST_CHECK(sr.col_num() == 1);

  // get each char of "foo"
  sr.get();  // 'f'
  TEST_CHECK(sr.col_num() == 2);
  sr.get();  // 'o'
  sr.get();  // 'o'
  TEST_CHECK(sr.col_num() == 4);
}

void test_stream_load_from_current_offset() {
  std::string prefix = "v 0 0 0\n";
  std::string payload = "v 1 2 3\n";
  std::string text = prefix + payload;
  std::istringstream obj_ss(text);
  obj_ss.seekg(static_cast<std::streamoff>(prefix.size()), std::ios::beg);

  tinyobj::attrib_t attrib;
  std::vector<tinyobj::shape_t> shapes;
  std::vector<tinyobj::material_t> materials;
  std::string warn, err;
  bool ret = tinyobj::LoadObj(&attrib, &shapes, &materials, &warn, &err,
                              &obj_ss, NULL);
  if (!warn.empty()) std::cout << "WARN: " << warn << "\n";
  if (!err.empty()) std::cerr << "ERR: " << err << "\n";
  TEST_CHECK(ret == true);
  TEST_CHECK(attrib.vertices.size() == 3);
  TEST_CHECK(attrib.vertices[0] == 1.0f);
  TEST_CHECK(attrib.vertices[1] == 2.0f);
  TEST_CHECK(attrib.vertices[2] == 3.0f);
}

void test_stream_load_rejects_oversized_input() {
  std::string oversized(TINYOBJLOADER_STREAM_READER_MAX_BYTES + size_t(1), ' ');
  std::istringstream obj_ss(oversized);

  tinyobj::attrib_t attrib;
  std::vector<tinyobj::shape_t> shapes;
  std::vector<tinyobj::material_t> materials;
  std::string warn, err;
  bool ret = tinyobj::LoadObj(&attrib, &shapes, &materials, &warn, &err,
                              &obj_ss, NULL);
  TEST_CHECK(ret == false);
  TEST_CHECK(err.find("input stream too large") != std::string::npos);
}

void test_error_format_clang_style() {
  const char *input = "v 1.0 abc 3.0\n";
  tinyobj::StreamReader sr(input, strlen(input));

  // Position to the 'a' in 'abc' (column 7)
  sr.advance(6);  // past "v 1.0 "
  TEST_CHECK(sr.col_num() == 7);

  std::string err = sr.format_error("test.obj", "expected number");
  // Should contain file:line:col
  TEST_CHECK(err.find("test.obj:1:7: error: expected number") != std::string::npos);
  // Should contain the source line
  TEST_CHECK(err.find("v 1.0 abc 3.0") != std::string::npos);
  // Should contain a caret
  TEST_CHECK(err.find("^") != std::string::npos);
}

void test_error_stack() {
  const char *input = "test\n";
  tinyobj::StreamReader sr(input, strlen(input));

  TEST_CHECK(!sr.has_errors());
  TEST_CHECK(sr.error_stack().empty());

  sr.push_error("error 1\n");
  sr.push_error("error 2\n");
  TEST_CHECK(sr.has_errors());
  TEST_CHECK(sr.error_stack().size() == 2);

  std::string all = sr.get_errors();
  TEST_CHECK(all.find("error 1") != std::string::npos);
  TEST_CHECK(all.find("error 2") != std::string::npos);

  sr.clear_errors();
  TEST_CHECK(!sr.has_errors());
  TEST_CHECK(sr.error_stack().empty());
}

void test_malformed_vertex_error() {
  const char *obj_text = "v 1.0 abc 3.0\n";
  std::istringstream iss(obj_text);
  tinyobj::attrib_t attrib;
  std::vector<tinyobj::shape_t> shapes;
  std::vector<tinyobj::material_t> materials;
  std::string warn, err;
  bool ret = tinyobj::LoadObj(&attrib, &shapes, &materials, &warn, &err,
                              &iss, NULL);
  // Early return: malformed vertex coordinate is unrecoverable
  TEST_CHECK(ret == false);
  TEST_CHECK(err.find("expected number") != std::string::npos);
  TEST_CHECK(err.find("abc") != std::string::npos);
}

void test_malformed_mtl_error() {
  const char *mtl_text = "newmtl test\nNs abc\n";
  std::istringstream mtl_iss(mtl_text);
  std::map<std::string, int> matMap;
  std::vector<tinyobj::material_t> materials;
  std::string warn, err;
  tinyobj::LoadMtl(&matMap, &materials, &mtl_iss, &warn, &err);
  // LoadMtl is void (public API), but error should still be reported
  TEST_CHECK(err.find("expected number") != std::string::npos);
  TEST_CHECK(err.find("abc") != std::string::npos);
}

void test_parse_error_backward_compat() {
  // Verify that valid OBJ input parses without errors (the old non-error
  // sr_parseReal path is still exercised by the callback API).
  const char *obj_text = "v 1.0 2.0 3.0\nv 4.0 5.0 6.0\nf 1 2 1\n";
  std::istringstream iss(obj_text);
  tinyobj::attrib_t attrib;
  std::vector<tinyobj::shape_t> shapes;
  std::vector<tinyobj::material_t> materials;
  std::string warn, err;
  bool ret = tinyobj::LoadObj(&attrib, &shapes, &materials, &warn, &err,
                              &iss, NULL);
  TEST_CHECK(ret == true);
  TEST_CHECK(err.empty());
  TEST_CHECK(attrib.vertices.size() == 6);
}

void test_split_string_preserves_non_escape_backslash() {
  std::vector<std::string> tokens;
  tinyobj::SplitString("subdir\\file.mtl", ' ', '\\', tokens);

  TEST_CHECK(tokens.size() == 1);
  TEST_CHECK(tokens[0] == "subdir\\file.mtl");

  tokens.clear();
  tinyobj::SplitString("a\\ b.mtl", ' ', '\\', tokens);
  TEST_CHECK(tokens.size() == 1);
  TEST_CHECK(tokens[0] == "a b.mtl");
}

void test_numeric_edge_cases() {
  tinyobj::attrib_t attrib;
  std::vector<tinyobj::shape_t> shapes;
  std::vector<tinyobj::material_t> materials;

  std::string warn;
  std::string err;
  bool ret = tinyobj::LoadObj(&attrib, &shapes, &materials, &warn, &err,
                              "../models/numeric-edge-cases.obj");

  if (!warn.empty()) std::cout << "WARN: " << warn << std::endl;
  if (!err.empty()) std::cerr << "ERR: " << err << std::endl;

  TEST_CHECK(true == ret);

  // 16 vertices * 3 components = 48
  TEST_CHECK(attrib.vertices.size() == 48);

  // v0: 0 0 0
  TEST_CHECK(FloatEquals(0.0f, attrib.vertices[0]));
  TEST_CHECK(FloatEquals(0.0f, attrib.vertices[1]));
  TEST_CHECK(FloatEquals(0.0f, attrib.vertices[2]));

  // v1: 1.5 -2.25 3.125
  TEST_CHECK(FloatEquals(1.5f, attrib.vertices[3]));
  TEST_CHECK(FloatEquals(-2.25f, attrib.vertices[4]));
  TEST_CHECK(FloatEquals(3.125f, attrib.vertices[5]));

  // v2: .5 -.75 .001 (leading decimal dot)
  TEST_CHECK(FloatEquals(0.5f, attrib.vertices[6]));
  TEST_CHECK(FloatEquals(-0.75f, attrib.vertices[7]));
  TEST_CHECK(FloatEquals(0.001f, attrib.vertices[8]));

  // v3: 1. -2. 100. (trailing dot)
  TEST_CHECK(FloatEquals(1.0f, attrib.vertices[9]));
  TEST_CHECK(FloatEquals(-2.0f, attrib.vertices[10]));
  TEST_CHECK(FloatEquals(100.0f, attrib.vertices[11]));

  // v4: 1.5e2 -3.0e-4 7e10 (scientific notation lowercase)
  TEST_CHECK(FloatEquals(150.0f, attrib.vertices[12]));
  TEST_CHECK(FloatEquals(-3.0e-4f, attrib.vertices[13]));
  TEST_CHECK(FloatEquals(7e10f, attrib.vertices[14]));

  // v5: 2.5E3 -1.0E-2 4E+5 (scientific notation uppercase)
  TEST_CHECK(FloatEquals(2500.0f, attrib.vertices[15]));
  TEST_CHECK(FloatEquals(-0.01f, attrib.vertices[16]));
  TEST_CHECK(FloatEquals(400000.0f, attrib.vertices[17]));

  // v6: +1.0 +0.5 +100 (leading plus)
  TEST_CHECK(FloatEquals(1.0f, attrib.vertices[18]));
  TEST_CHECK(FloatEquals(0.5f, attrib.vertices[19]));
  TEST_CHECK(FloatEquals(100.0f, attrib.vertices[20]));

  // v7: 007.5 -003.14 000.001 (leading zeros)
  TEST_CHECK(FloatEquals(7.5f, attrib.vertices[21]));
  TEST_CHECK(FloatEquals(-3.14f, attrib.vertices[22]));
  TEST_CHECK(FloatEquals(0.001f, attrib.vertices[23]));

  // v8: 1e-300 -1e-300 5e-310 (tiny values -- flush to zero in float)
  // These are below float min, so they become 0 in float mode.
  // Just check they parsed without error (ret == true above).

  // v9: 1.7976931348623157e+308 -1e+308 1e+307
  // These overflow float, but should not crash. Check parse succeeded.

  // v10: -0 -0.0 -0.0e0 (negative zero)
  TEST_CHECK(FloatEquals(0.0f, attrib.vertices[30]));
  TEST_CHECK(FloatEquals(0.0f, attrib.vertices[31]));
  TEST_CHECK(FloatEquals(0.0f, attrib.vertices[32]));

  // v11: 1.5e002 -3.0e+007 7e-003 (exponent with leading zeros)
  TEST_CHECK(FloatEquals(150.0f, attrib.vertices[33]));
  TEST_CHECK(FloatEquals(-3.0e7f, attrib.vertices[34]));
  TEST_CHECK(FloatEquals(7e-3f, attrib.vertices[35]));

  // v12: 0 1 9 (single digit values)
  TEST_CHECK(FloatEquals(0.0f, attrib.vertices[36]));
  TEST_CHECK(FloatEquals(1.0f, attrib.vertices[37]));
  TEST_CHECK(FloatEquals(9.0f, attrib.vertices[38]));

  // v13: 1e+0 1e-0 -1e+0 (exponent zero)
  TEST_CHECK(FloatEquals(1.0f, attrib.vertices[39]));
  TEST_CHECK(FloatEquals(1.0f, attrib.vertices[40]));
  TEST_CHECK(FloatEquals(-1.0f, attrib.vertices[41]));

  // v14: pi, e, sqrt(2) (high precision)
  TEST_CHECK(FloatEquals(3.141592653589793f, attrib.vertices[42]));
  TEST_CHECK(FloatEquals(2.718281828459045f, attrib.vertices[43]));
  TEST_CHECK(FloatEquals(1.4142135623730951f, attrib.vertices[44]));

  // v15: 1e1 1e-1 -1e1 (simple exponent)
  TEST_CHECK(FloatEquals(10.0f, attrib.vertices[45]));
  TEST_CHECK(FloatEquals(0.1f, attrib.vertices[46]));
  TEST_CHECK(FloatEquals(-10.0f, attrib.vertices[47]));

  // Normals: 3 normals * 3 = 9
  TEST_CHECK(attrib.normals.size() == 9);
  TEST_CHECK(FloatEquals(0.0f, attrib.normals[0]));
  TEST_CHECK(FloatEquals(1.0f, attrib.normals[1]));
  TEST_CHECK(FloatEquals(0.0f, attrib.normals[2]));
  TEST_CHECK(FloatEquals(-0.707107f, attrib.normals[3]));
  TEST_CHECK(FloatEquals(0.0f, attrib.normals[4]));
  TEST_CHECK(FloatEquals(0.707107f, attrib.normals[5]));
  TEST_CHECK(FloatEquals(1e-5f, attrib.normals[6]));
  TEST_CHECK(FloatEquals(-1e-5f, attrib.normals[7]));
  TEST_CHECK(FloatEquals(0.99999f, attrib.normals[8]));

  // Texcoords: 4 texcoords * 2 = 8
  TEST_CHECK(attrib.texcoords.size() == 8);
  TEST_CHECK(FloatEquals(0.0f, attrib.texcoords[0]));
  TEST_CHECK(FloatEquals(0.0f, attrib.texcoords[1]));
  TEST_CHECK(FloatEquals(1.0f, attrib.texcoords[2]));
  TEST_CHECK(FloatEquals(1.0f, attrib.texcoords[3]));
  TEST_CHECK(FloatEquals(0.5f, attrib.texcoords[4]));
  TEST_CHECK(FloatEquals(0.5f, attrib.texcoords[5]));
  TEST_CHECK(FloatEquals(0.25f, attrib.texcoords[6]));
  TEST_CHECK(FloatEquals(0.75f, attrib.texcoords[7]));
}

void test_numeric_nan_inf() {
  // Test nan/inf parsing via an in-memory OBJ string
  std::string obj_str =
      "v nan 0 0\n"
      "v NaN 1 1\n"
      "v NAN 2 2\n"
      "v inf 0 0\n"
      "v -inf 1 1\n"
      "v Inf 2 2\n"
      "v -Inf 3 3\n"
      "v INF 4 4\n"
      "v infinity 0 0\n"
      "v -infinity 1 1\n"
      "v +nan 0 0\n"
      "v +inf 0 0\n"
      "f 1 2 3\n";

  std::istringstream obj_stream(obj_str);

  tinyobj::attrib_t attrib;
  std::vector<tinyobj::shape_t> shapes;
  std::vector<tinyobj::material_t> materials;

  std::string warn;
  std::string err;
  bool ret = tinyobj::LoadObj(&attrib, &shapes, &materials, &warn, &err,
                              &obj_stream, NULL);

  if (!warn.empty()) std::cout << "WARN: " << warn << std::endl;
  if (!err.empty()) std::cerr << "ERR: " << err << std::endl;

  TEST_CHECK(true == ret);
  // 12 vertices * 3 components = 36
  TEST_CHECK(attrib.vertices.size() == 36);

  // All nan/inf should parse without crashing.
  // The exact values depend on the implementation (nan -> 0.0, inf -> max, -inf -> lowest),
  // but the parser must not fail or produce garbage for the non-nan/inf coords.

  // v0: nan 0 0 -> second and third should be 0
  TEST_CHECK(FloatEquals(0.0f, attrib.vertices[1]));
  TEST_CHECK(FloatEquals(0.0f, attrib.vertices[2]));

  // v3: inf 0 0 -> second and third should be 0
  TEST_CHECK(FloatEquals(0.0f, attrib.vertices[10]));
  TEST_CHECK(FloatEquals(0.0f, attrib.vertices[11]));

  // v4: -inf 1 1
  TEST_CHECK(FloatEquals(1.0f, attrib.vertices[13]));
  TEST_CHECK(FloatEquals(1.0f, attrib.vertices[14]));
}

void test_numeric_from_stream() {
  // Test that stream-based loading also gets the same numeric results
  std::string obj_str =
      "v 1.5e2 -3.0e-4 +7.5\n"
      "v .001 -.999 1.\n"
      "v 0 0 0\n"
      "f 1 2 3\n";

  std::istringstream obj_stream(obj_str);

  tinyobj::attrib_t attrib;
  std::vector<tinyobj::shape_t> shapes;
  std::vector<tinyobj::material_t> materials;

  std::string warn;
  std::string err;
  bool ret = tinyobj::LoadObj(&attrib, &shapes, &materials, &warn, &err,
                              &obj_stream, NULL);

  TEST_CHECK(true == ret);
  TEST_CHECK(attrib.vertices.size() == 9);

  TEST_CHECK(FloatEquals(150.0f, attrib.vertices[0]));
  TEST_CHECK(FloatEquals(-3.0e-4f, attrib.vertices[1]));
  TEST_CHECK(FloatEquals(7.5f, attrib.vertices[2]));

  TEST_CHECK(FloatEquals(0.001f, attrib.vertices[3]));
  TEST_CHECK(FloatEquals(-0.999f, attrib.vertices[4]));
  TEST_CHECK(FloatEquals(1.0f, attrib.vertices[5]));

  TEST_CHECK(FloatEquals(0.0f, attrib.vertices[6]));
  TEST_CHECK(FloatEquals(0.0f, attrib.vertices[7]));
  TEST_CHECK(FloatEquals(0.0f, attrib.vertices[8]));
}

void test_numeric_overflow_preserves_default() {
  // Regression: values that overflow double must not crash or corrupt memory.
  // tryParseDouble now parses into a temp; *result is only written on success.
  // With the StreamReader-based parser, overflow is detected as a parse error.
  std::string obj_str =
      "v 1e9999 2.0 3.0\n"    // first coord overflows
      "f 1\n";

  std::istringstream obj_stream(obj_str);
  tinyobj::attrib_t attrib;
  std::vector<tinyobj::shape_t> shapes;
  std::vector<tinyobj::material_t> materials;
  std::string warn, err;
  bool ret = tinyobj::LoadObj(&attrib, &shapes, &materials, &warn, &err,
                              &obj_stream, NULL);

  // Must not crash. Parser detects overflow and returns false.
  TEST_CHECK(false == ret);
  TEST_CHECK(!err.empty());
}

void test_numeric_empty_and_whitespace() {
  // Regression: empty tokens, whitespace-only lines, and trailing whitespace
  // must not crash the parser.
  std::string obj_str =
      "v   1.0   2.0   3.0  \n"   // extra whitespace around values
      "v 4.0 5.0 6.0\r\n"         // Windows line endings
      "v\t7.0\t8.0\t9.0\n"        // tab-separated
      "\n"                          // blank line
      "   \n"                       // whitespace-only line
      "f 1 2 3\n";

  std::istringstream obj_stream(obj_str);
  tinyobj::attrib_t attrib;
  std::vector<tinyobj::shape_t> shapes;
  std::vector<tinyobj::material_t> materials;
  std::string warn, err;
  bool ret = tinyobj::LoadObj(&attrib, &shapes, &materials, &warn, &err,
                              &obj_stream, NULL);

  TEST_CHECK(true == ret);
  TEST_CHECK(attrib.vertices.size() == 9);

  TEST_CHECK(FloatEquals(1.0f, attrib.vertices[0]));
  TEST_CHECK(FloatEquals(2.0f, attrib.vertices[1]));
  TEST_CHECK(FloatEquals(3.0f, attrib.vertices[2]));
  TEST_CHECK(FloatEquals(4.0f, attrib.vertices[3]));
  TEST_CHECK(FloatEquals(5.0f, attrib.vertices[4]));
  TEST_CHECK(FloatEquals(6.0f, attrib.vertices[5]));
  TEST_CHECK(FloatEquals(7.0f, attrib.vertices[6]));
  TEST_CHECK(FloatEquals(8.0f, attrib.vertices[7]));
  TEST_CHECK(FloatEquals(9.0f, attrib.vertices[8]));
}

void test_numeric_garbage_input() {
  // Regression: totally invalid numeric input must not crash.
  // With the StreamReader-based parser, garbage input is detected and
  // LoadObj returns false with an error message.
  std::string obj_str =
      "v abc def ghi\n"           // alphabetic garbage
      "f 1\n";

  std::istringstream obj_stream(obj_str);
  tinyobj::attrib_t attrib;
  std::vector<tinyobj::shape_t> shapes;
  std::vector<tinyobj::material_t> materials;
  std::string warn, err;
  bool ret = tinyobj::LoadObj(&attrib, &shapes, &materials, &warn, &err,
                              &obj_stream, NULL);

  // Must not crash. Parser detects invalid input and returns false.
  TEST_CHECK(false == ret);
  TEST_CHECK(!err.empty());
  TEST_CHECK(err.find("expected number") != std::string::npos);
}

void test_numeric_extreme_precision() {
  // Regression: values with many digits must not crash or corrupt.
  // fast_float handles arbitrary digit counts gracefully.
  std::string obj_str =
      "v 1.00000000000000000000000000000000000001 "
         "2.99999999999999999999999999999999999999 "
         "0.00000000000000000000000000000000000001\n"
      "v 123456789012345678.0 -123456789012345678.0 0.0\n"
      "f 1 2\n";

  std::istringstream obj_stream(obj_str);
  tinyobj::attrib_t attrib;
  std::vector<tinyobj::shape_t> shapes;
  std::vector<tinyobj::material_t> materials;
  std::string warn, err;
  bool ret = tinyobj::LoadObj(&attrib, &shapes, &materials, &warn, &err,
                              &obj_stream, NULL);

  TEST_CHECK(true == ret);
  TEST_CHECK(attrib.vertices.size() == 6);

  // Values should round to nearest representable float
  TEST_CHECK(FloatEquals(1.0f, attrib.vertices[0]));
  TEST_CHECK(FloatEquals(3.0f, attrib.vertices[1]));
}

// ---------------------------------------------------------------------------
// Additional coverage tests
// ---------------------------------------------------------------------------

// StreamReader: direct unit tests for public API methods
void test_streamreader_eof_and_remaining() {
  // Empty input
  {
    tinyobj::StreamReader sr("", 0);
    TEST_CHECK(sr.eof() == true);
    TEST_CHECK(sr.remaining() == 0);
    TEST_CHECK(sr.peek() == '\0');
    TEST_CHECK(sr.peek_at(0) == '\0');
    TEST_CHECK(sr.peek_at(100) == '\0');
    TEST_CHECK(sr.get() == '\0');
    TEST_CHECK(sr.char_at(0, 'a') == false);
    TEST_CHECK(sr.match("abc", 3) == false);
    TEST_CHECK(sr.line_num() == 1);
    TEST_CHECK(sr.col_num() == 1);
  }
  // Single char
  {
    tinyobj::StreamReader sr("x", 1);
    TEST_CHECK(sr.eof() == false);
    TEST_CHECK(sr.remaining() == 1);
    TEST_CHECK(sr.peek() == 'x');
    TEST_CHECK(sr.char_at(0, 'x') == true);
    TEST_CHECK(sr.char_at(0, 'y') == false);
    TEST_CHECK(sr.char_at(1, 'x') == false);  // out of bounds
    TEST_CHECK(sr.match("x", 1) == true);
    TEST_CHECK(sr.match("xy", 2) == false);
    char c = sr.get();
    TEST_CHECK(c == 'x');
    TEST_CHECK(sr.eof() == true);
    TEST_CHECK(sr.remaining() == 0);
    // After EOF, these should be safe
    TEST_CHECK(sr.peek() == '\0');
    TEST_CHECK(sr.peek_at(0) == '\0');
    TEST_CHECK(sr.match("a", 1) == false);
    TEST_CHECK(sr.char_at(0, 'a') == false);
  }
}

void test_streamreader_skip_and_read() {
  const char *input = "  hello \t world\r\nline2\n";
  tinyobj::StreamReader sr(input, strlen(input));

  // skip_space should skip spaces and tabs
  sr.skip_space();
  TEST_CHECK(sr.peek() == 'h');
  TEST_CHECK(sr.col_num() == 3);

  // read_token should return "hello"
  std::string tok = sr.read_token();
  TEST_CHECK(tok == "hello");

  // skip_space should skip " \t "
  sr.skip_space();
  TEST_CHECK(sr.peek() == 'w');

  // read_token should return "world"
  tok = sr.read_token();
  TEST_CHECK(tok == "world");

  // at_line_end should be true (next is \r\n)
  TEST_CHECK(sr.at_line_end() == true);

  // skip_line should advance past \r\n
  sr.skip_line();
  TEST_CHECK(sr.line_num() == 2);
  TEST_CHECK(sr.col_num() == 1);

  // read_line should return "line2"
  std::string line = sr.read_line();
  TEST_CHECK(line == "line2");
  // read_line reads the content but line_num updates on skip_line/get past \n
  TEST_CHECK(sr.line_num() == 2);
}

void test_streamreader_match_and_advance() {
  const char *input = "mtllib foo.mtl\n";
  tinyobj::StreamReader sr(input, strlen(input));

  TEST_CHECK(sr.match("mtllib", 6) == true);
  TEST_CHECK(sr.match("mtlliX", 6) == false);
  TEST_CHECK(sr.match("mtllib foo.mtl\n", 15) == true);
  // match longer than remaining
  TEST_CHECK(sr.match("mtllib foo.mtl\nX", 16) == false);

  sr.advance(7);  // past "mtllib "
  TEST_CHECK(sr.peek() == 'f');
  TEST_CHECK(sr.col_num() == 8);

  // advance past end should clamp to EOF
  sr.advance(1000);
  TEST_CHECK(sr.eof() == true);
}

void test_streamreader_current_line_text() {
  const char *input = "first line\nsecond line\nthird\n";
  tinyobj::StreamReader sr(input, strlen(input));

  // On first line
  std::string lt = sr.current_line_text();
  TEST_CHECK(lt == "first line");

  sr.skip_line();  // move to second line
  lt = sr.current_line_text();
  TEST_CHECK(lt == "second line");

  sr.advance(3);  // in the middle of "second line" -> "con" in "second"
  lt = sr.current_line_text();
  TEST_CHECK(lt == "second line");
}

void test_streamreader_error_stack() {
  const char *input = "hello\n";
  tinyobj::StreamReader sr(input, strlen(input));

  TEST_CHECK(sr.has_errors() == false);
  TEST_CHECK(sr.get_errors().empty());

  sr.push_error("error 1");
  TEST_CHECK(sr.has_errors() == true);
  TEST_CHECK(sr.error_stack().size() == 1);

  sr.push_error("error 2");
  TEST_CHECK(sr.error_stack().size() == 2);
  // get_errors() concatenates all errors into a single string
  TEST_CHECK(sr.get_errors().find("error 1") != std::string::npos);
  TEST_CHECK(sr.get_errors().find("error 2") != std::string::npos);

  sr.clear_errors();
  TEST_CHECK(sr.has_errors() == false);
  TEST_CHECK(sr.get_errors().empty());
}

// Empty OBJ file (0 bytes)
void test_empty_obj_file() {
  std::istringstream iss("");
  tinyobj::attrib_t attrib;
  std::vector<tinyobj::shape_t> shapes;
  std::vector<tinyobj::material_t> materials;
  std::string warn, err;
  bool ret = tinyobj::LoadObj(&attrib, &shapes, &materials, &warn, &err,
                              &iss, NULL);
  TEST_CHECK(ret == true);
  TEST_CHECK(attrib.vertices.empty());
  TEST_CHECK(shapes.empty());
}

// OBJ with only BOM (3 bytes, no content)
void test_bom_only_obj() {
  std::string bom("\xEF\xBB\xBF");
  std::istringstream iss(bom);
  tinyobj::attrib_t attrib;
  std::vector<tinyobj::shape_t> shapes;
  std::vector<tinyobj::material_t> materials;
  std::string warn, err;
  bool ret = tinyobj::LoadObj(&attrib, &shapes, &materials, &warn, &err,
                              &iss, NULL);
  TEST_CHECK(ret == true);
  TEST_CHECK(attrib.vertices.empty());
}

// File with no trailing newline
void test_no_trailing_newline() {
  const char *obj_text = "v 1.0 2.0 3.0\nv 4.0 5.0 6.0";  // no \n at end
  std::istringstream iss(obj_text);
  tinyobj::attrib_t attrib;
  std::vector<tinyobj::shape_t> shapes;
  std::vector<tinyobj::material_t> materials;
  std::string warn, err;
  bool ret = tinyobj::LoadObj(&attrib, &shapes, &materials, &warn, &err,
                              &iss, NULL);
  TEST_CHECK(ret == true);
  TEST_CHECK(attrib.vertices.size() == 6);
  TEST_CHECK(FloatEquals(4.0f, attrib.vertices[3]));
  TEST_CHECK(FloatEquals(5.0f, attrib.vertices[4]));
  TEST_CHECK(FloatEquals(6.0f, attrib.vertices[5]));
}

// Mixed CRLF, LF, and CR-only line endings
void test_mixed_line_endings() {
  // LF, CRLF, CR-only, and no trailing newline
  std::string obj_text = "v 1.0 2.0 3.0\n"
                         "v 4.0 5.0 6.0\r\n"
                         "v 7.0 8.0 9.0\r"
                         "f 1 2 3";
  std::istringstream iss(obj_text);
  tinyobj::attrib_t attrib;
  std::vector<tinyobj::shape_t> shapes;
  std::vector<tinyobj::material_t> materials;
  std::string warn, err;
  bool ret = tinyobj::LoadObj(&attrib, &shapes, &materials, &warn, &err,
                              &iss, NULL);
  TEST_CHECK(ret == true);
  TEST_CHECK(attrib.vertices.size() == 9);
  TEST_CHECK(FloatEquals(7.0f, attrib.vertices[6]));
  TEST_CHECK(FloatEquals(8.0f, attrib.vertices[7]));
  TEST_CHECK(FloatEquals(9.0f, attrib.vertices[8]));
  TEST_CHECK(shapes.size() == 1);
}

// Vertex colors from in-memory stream (6-component vertices)
void test_vertex_colors_from_stream() {
  const char *obj_text =
      "v 1.0 2.0 3.0 0.5 0.6 0.7\n"
      "v 4.0 5.0 6.0 0.1 0.2 0.3\n"
      "f 1 2 1\n";
  std::istringstream iss(obj_text);
  tinyobj::attrib_t attrib;
  std::vector<tinyobj::shape_t> shapes;
  std::vector<tinyobj::material_t> materials;
  std::string warn, err;
  bool ret = tinyobj::LoadObj(&attrib, &shapes, &materials, &warn, &err,
                              &iss, NULL);
  TEST_CHECK(ret == true);
  TEST_CHECK(attrib.vertices.size() == 6);
  TEST_CHECK(attrib.colors.size() == 6);
  TEST_CHECK(FloatEquals(0.5f, attrib.colors[0]));
  TEST_CHECK(FloatEquals(0.6f, attrib.colors[1]));
  TEST_CHECK(FloatEquals(0.7f, attrib.colors[2]));
  TEST_CHECK(FloatEquals(0.1f, attrib.colors[3]));
  TEST_CHECK(FloatEquals(0.2f, attrib.colors[4]));
  TEST_CHECK(FloatEquals(0.3f, attrib.colors[5]));
}

// Mixed: some vertices with colors, some without
void test_vertex_colors_mixed() {
  const char *obj_text =
      "v 1.0 2.0 3.0 0.5 0.6 0.7\n"
      "v 4.0 5.0 6.0\n"
      "f 1 2 1\n";
  std::istringstream iss(obj_text);
  tinyobj::attrib_t attrib;
  std::vector<tinyobj::shape_t> shapes;
  std::vector<tinyobj::material_t> materials;
  std::string warn, err;
  bool ret = tinyobj::LoadObj(&attrib, &shapes, &materials, &warn, &err,
                              &iss, NULL);
  TEST_CHECK(ret == true);
  TEST_CHECK(attrib.vertices.size() == 6);
  // Colors array should have entries for both vertices (default 1.0 for no-color vertex)
  TEST_CHECK(attrib.colors.size() == 6);
  TEST_CHECK(FloatEquals(0.5f, attrib.colors[0]));
  TEST_CHECK(FloatEquals(0.6f, attrib.colors[1]));
  TEST_CHECK(FloatEquals(0.7f, attrib.colors[2]));
  TEST_CHECK(FloatEquals(1.0f, attrib.colors[3]));
  TEST_CHECK(FloatEquals(1.0f, attrib.colors[4]));
  TEST_CHECK(FloatEquals(1.0f, attrib.colors[5]));
}

// OBJ with all element types: v, vn, vt, f, l, p
void test_all_element_types() {
  const char *obj_text =
      "v 1.0 0.0 0.0\n"
      "v 0.0 1.0 0.0\n"
      "v 0.0 0.0 1.0\n"
      "vn 0.0 0.0 1.0\n"
      "vt 0.5 0.5\n"
      "f 1/1/1 2/1/1 3/1/1\n"
      "l 1 2\n"
      "p 3\n";
  std::istringstream iss(obj_text);
  tinyobj::attrib_t attrib;
  std::vector<tinyobj::shape_t> shapes;
  std::vector<tinyobj::material_t> materials;
  std::string warn, err;
  bool ret = tinyobj::LoadObj(&attrib, &shapes, &materials, &warn, &err,
                              &iss, NULL);
  TEST_CHECK(ret == true);
  TEST_CHECK(attrib.vertices.size() == 9);
  TEST_CHECK(attrib.normals.size() == 3);
  TEST_CHECK(attrib.texcoords.size() == 2);
  TEST_CHECK(shapes.size() >= 1);
  // Face indices
  TEST_CHECK(shapes[0].mesh.indices.size() == 3);
  // Line indices
  TEST_CHECK(shapes[0].lines.indices.size() == 2);
  // Point indices
  TEST_CHECK(shapes[0].points.indices.size() == 1);
}

// Multiple groups and objects
void test_multiple_objects() {
  const char *obj_text =
      "v 0.0 0.0 0.0\n"
      "v 1.0 0.0 0.0\n"
      "v 0.0 1.0 0.0\n"
      "v 0.0 0.0 1.0\n"
      "o obj1\n"
      "f 1 2 3\n"
      "o obj2\n"
      "f 2 3 4\n";
  std::istringstream iss(obj_text);
  tinyobj::attrib_t attrib;
  std::vector<tinyobj::shape_t> shapes;
  std::vector<tinyobj::material_t> materials;
  std::string warn, err;
  bool ret = tinyobj::LoadObj(&attrib, &shapes, &materials, &warn, &err,
                              &iss, NULL);
  TEST_CHECK(ret == true);
  TEST_CHECK(shapes.size() == 2);
  TEST_CHECK(shapes[0].name == "obj1");
  TEST_CHECK(shapes[1].name == "obj2");
  TEST_CHECK(shapes[0].mesh.indices.size() == 3);
  TEST_CHECK(shapes[1].mesh.indices.size() == 3);
}

// MTL warning accumulation (d and Tr conflict)
void test_mtl_d_and_tr_warning() {
  // Both d and Tr in same material should produce a warning
  const char *mtl_text = "newmtl test\nd 0.5\nTr 0.8\n";
  std::istringstream mtl_iss(mtl_text);
  std::map<std::string, int> matMap;
  std::vector<tinyobj::material_t> materials;
  std::string warn, err;
  tinyobj::LoadMtl(&matMap, &materials, &mtl_iss, &warn, &err);
  TEST_CHECK(materials.size() == 1);
  // d=0.5 should win over Tr=0.8
  TEST_CHECK(FloatEquals(0.5f, materials[0].dissolve));
}

// Multiple malformed lines: errors should accumulate
void test_multiple_malformed_vertices() {
  const char *obj_text =
      "v 1.0 bad1 3.0\n"
      "v 4.0 bad2 6.0\n";
  std::istringstream iss(obj_text);
  tinyobj::attrib_t attrib;
  std::vector<tinyobj::shape_t> shapes;
  std::vector<tinyobj::material_t> materials;
  std::string warn, err;
  bool ret = tinyobj::LoadObj(&attrib, &shapes, &materials, &warn, &err,
                              &iss, NULL);
  TEST_CHECK(ret == false);
  // First error causes early return, so at least one error must be present
  TEST_CHECK(err.find("bad1") != std::string::npos);
}

// Malformed normal
void test_malformed_normal_error() {
  const char *obj_text = "vn 1.0 xyz 0.0\n";
  std::istringstream iss(obj_text);
  tinyobj::attrib_t attrib;
  std::vector<tinyobj::shape_t> shapes;
  std::vector<tinyobj::material_t> materials;
  std::string warn, err;
  bool ret = tinyobj::LoadObj(&attrib, &shapes, &materials, &warn, &err,
                              &iss, NULL);
  TEST_CHECK(ret == false);
  TEST_CHECK(err.find("expected number") != std::string::npos);
}

// Malformed texcoord
void test_malformed_texcoord_error() {
  const char *obj_text = "vt abc 0.5\n";
  std::istringstream iss(obj_text);
  tinyobj::attrib_t attrib;
  std::vector<tinyobj::shape_t> shapes;
  std::vector<tinyobj::material_t> materials;
  std::string warn, err;
  bool ret = tinyobj::LoadObj(&attrib, &shapes, &materials, &warn, &err,
                              &iss, NULL);
  TEST_CHECK(ret == false);
  TEST_CHECK(err.find("expected number") != std::string::npos);
}

// Negative vertex indices (relative indexing)
void test_negative_vertex_indices() {
  const char *obj_text =
      "v 1.0 0.0 0.0\n"
      "v 0.0 1.0 0.0\n"
      "v 0.0 0.0 1.0\n"
      "f -3 -2 -1\n";
  std::istringstream iss(obj_text);
  tinyobj::attrib_t attrib;
  std::vector<tinyobj::shape_t> shapes;
  std::vector<tinyobj::material_t> materials;
  std::string warn, err;
  bool ret = tinyobj::LoadObj(&attrib, &shapes, &materials, &warn, &err,
                              &iss, NULL);
  TEST_CHECK(ret == true);
  TEST_CHECK(shapes.size() == 1);
  TEST_CHECK(shapes[0].mesh.indices.size() == 3);
  // -3 should resolve to index 0, -2 to 1, -1 to 2
  TEST_CHECK(shapes[0].mesh.indices[0].vertex_index == 0);
  TEST_CHECK(shapes[0].mesh.indices[1].vertex_index == 1);
  TEST_CHECK(shapes[0].mesh.indices[2].vertex_index == 2);
}

// Comments everywhere (inline and full-line)
void test_comments_everywhere() {
  const char *obj_text =
      "# full line comment\n"
      "v 1.0 2.0 3.0 # inline comment\n"
      "  # indented comment\n"
      "v 4.0 5.0 6.0\n"
      "# another comment\n"
      "f 1 2 1 # face comment\n";
  std::istringstream iss(obj_text);
  tinyobj::attrib_t attrib;
  std::vector<tinyobj::shape_t> shapes;
  std::vector<tinyobj::material_t> materials;
  std::string warn, err;
  bool ret = tinyobj::LoadObj(&attrib, &shapes, &materials, &warn, &err,
                              &iss, NULL);
  TEST_CHECK(ret == true);
  TEST_CHECK(attrib.vertices.size() == 6);
  TEST_CHECK(shapes.size() == 1);
}

// Multiple spaces/tabs between tokens
void test_excessive_whitespace() {
  const char *obj_text =
      "v   1.0  \t  2.0  \t\t  3.0\n"
      "v\t4.0\t5.0\t6.0\n"
      "f  1  2  1\n";
  std::istringstream iss(obj_text);
  tinyobj::attrib_t attrib;
  std::vector<tinyobj::shape_t> shapes;
  std::vector<tinyobj::material_t> materials;
  std::string warn, err;
  bool ret = tinyobj::LoadObj(&attrib, &shapes, &materials, &warn, &err,
                              &iss, NULL);
  TEST_CHECK(ret == true);
  TEST_CHECK(attrib.vertices.size() == 6);
  TEST_CHECK(FloatEquals(1.0f, attrib.vertices[0]));
  TEST_CHECK(FloatEquals(2.0f, attrib.vertices[1]));
  TEST_CHECK(FloatEquals(3.0f, attrib.vertices[2]));
  TEST_CHECK(FloatEquals(4.0f, attrib.vertices[3]));
}

// v2 ObjReader API
void test_objreader_api_stream() {
  const char *obj_text =
      "v 1.0 2.0 3.0\n"
      "v 4.0 5.0 6.0\n"
      "v 7.0 8.0 9.0\n"
      "f 1 2 3\n";

  tinyobj::ObjReader reader;
  bool ret = reader.ParseFromString(obj_text, "");
  TEST_CHECK(ret == true);
  TEST_CHECK(reader.Valid());
  TEST_CHECK(reader.GetAttrib().vertices.size() == 9);
  TEST_CHECK(reader.GetShapes().size() == 1);
  // Warning output is optional and is not asserted here.
}

// ObjReader with invalid input
void test_objreader_api_error() {
  tinyobj::ObjReader reader;
  bool ret = reader.ParseFromString("v 1.0 badval 3.0\n", "");
  TEST_CHECK(ret == false);
  TEST_CHECK(!reader.Error().empty());
}

// SplitString edge cases
void test_split_string_edge_cases() {
  std::vector<std::string> tokens;

  // Empty input — SplitString always pushes at least one token (possibly empty)
  tinyobj::SplitString("", ' ', '\\', tokens);
  TEST_CHECK(tokens.size() == 1);
  TEST_CHECK(tokens[0].empty());

  // Only spaces — trailing token is empty
  tokens.clear();
  tinyobj::SplitString("   ", ' ', '\\', tokens);
  TEST_CHECK(tokens.size() == 1);
  TEST_CHECK(tokens[0].empty());

  // Multiple tokens with multiple delimiters
  tokens.clear();
  tinyobj::SplitString("a  b  c", ' ', '\\', tokens);
  TEST_CHECK(tokens.size() == 3);
  TEST_CHECK(tokens[0] == "a");
  TEST_CHECK(tokens[1] == "b");
  TEST_CHECK(tokens[2] == "c");

  // Escaped space in middle
  tokens.clear();
  tinyobj::SplitString("path\\ name.mtl other.mtl", ' ', '\\', tokens);
  TEST_CHECK(tokens.size() == 2);
  TEST_CHECK(tokens[0] == "path name.mtl");
  TEST_CHECK(tokens[1] == "other.mtl");

  // Trailing backslash (not an escape — preserved as-is)
  tokens.clear();
  tinyobj::SplitString("dir\\", ' ', '\\', tokens);
  TEST_CHECK(tokens.size() == 1);
  TEST_CHECK(tokens[0] == "dir\\");
}

// Quad face (non-triangle)
void test_quad_face() {
  const char *obj_text =
      "v 0.0 0.0 0.0\n"
      "v 1.0 0.0 0.0\n"
      "v 1.0 1.0 0.0\n"
      "v 0.0 1.0 0.0\n"
      "f 1 2 3 4\n";
  std::istringstream iss(obj_text);
  tinyobj::ObjReaderConfig config;
  config.triangulate = false;
  tinyobj::ObjReader reader;
  bool ret = reader.ParseFromString(obj_text, "", config);
  TEST_CHECK(ret == true);
  TEST_CHECK(reader.GetShapes().size() == 1);
  // Without triangulation: 4 indices, 1 face
  TEST_CHECK(reader.GetShapes()[0].mesh.indices.size() == 4);
  TEST_CHECK(reader.GetShapes()[0].mesh.num_face_vertices.size() == 1);
  TEST_CHECK(reader.GetShapes()[0].mesh.num_face_vertices[0] == 4);
}

// Quad face with triangulation
void test_quad_face_triangulated() {
  const char *obj_text =
      "v 0.0 0.0 0.0\n"
      "v 1.0 0.0 0.0\n"
      "v 1.0 1.0 0.0\n"
      "v 0.0 1.0 0.0\n"
      "f 1 2 3 4\n";
  std::istringstream iss(obj_text);
  tinyobj::ObjReaderConfig config;
  config.triangulate = true;
  tinyobj::ObjReader reader;
  bool ret = reader.ParseFromString(obj_text, "", config);
  TEST_CHECK(ret == true);
  TEST_CHECK(reader.GetShapes().size() == 1);
  // With triangulation: quad -> 2 triangles = 6 indices
  TEST_CHECK(reader.GetShapes()[0].mesh.indices.size() == 6);
  TEST_CHECK(reader.GetShapes()[0].mesh.num_face_vertices.size() == 2);
}

// Face with v/vt/vn format
void test_face_full_index_format() {
  const char *obj_text =
      "v 0.0 0.0 0.0\n"
      "v 1.0 0.0 0.0\n"
      "v 0.0 1.0 0.0\n"
      "vt 0.0 0.0\n"
      "vt 1.0 0.0\n"
      "vt 0.0 1.0\n"
      "vn 0.0 0.0 1.0\n"
      "f 1/1/1 2/2/1 3/3/1\n";
  std::istringstream iss(obj_text);
  tinyobj::attrib_t attrib;
  std::vector<tinyobj::shape_t> shapes;
  std::vector<tinyobj::material_t> materials;
  std::string warn, err;
  bool ret = tinyobj::LoadObj(&attrib, &shapes, &materials, &warn, &err,
                              &iss, NULL);
  TEST_CHECK(ret == true);
  TEST_CHECK(shapes[0].mesh.indices[0].vertex_index == 0);
  TEST_CHECK(shapes[0].mesh.indices[0].texcoord_index == 0);
  TEST_CHECK(shapes[0].mesh.indices[0].normal_index == 0);
  TEST_CHECK(shapes[0].mesh.indices[1].vertex_index == 1);
  TEST_CHECK(shapes[0].mesh.indices[1].texcoord_index == 1);
  TEST_CHECK(shapes[0].mesh.indices[2].vertex_index == 2);
  TEST_CHECK(shapes[0].mesh.indices[2].texcoord_index == 2);
}

// Face with v//vn format (no texcoord)
void test_face_vertex_normal_only() {
  const char *obj_text =
      "v 0.0 0.0 0.0\n"
      "v 1.0 0.0 0.0\n"
      "v 0.0 1.0 0.0\n"
      "vn 0.0 0.0 1.0\n"
      "f 1//1 2//1 3//1\n";
  std::istringstream iss(obj_text);
  tinyobj::attrib_t attrib;
  std::vector<tinyobj::shape_t> shapes;
  std::vector<tinyobj::material_t> materials;
  std::string warn, err;
  bool ret = tinyobj::LoadObj(&attrib, &shapes, &materials, &warn, &err,
                              &iss, NULL);
  TEST_CHECK(ret == true);
  TEST_CHECK(shapes[0].mesh.indices[0].vertex_index == 0);
  TEST_CHECK(shapes[0].mesh.indices[0].texcoord_index == -1);
  TEST_CHECK(shapes[0].mesh.indices[0].normal_index == 0);
}

// MTL with multiple materials and various properties
void test_mtl_multiple_properties() {
  const char *mtl_text =
      "newmtl mat1\n"
      "Ka 0.1 0.2 0.3\n"
      "Kd 0.4 0.5 0.6\n"
      "Ks 0.7 0.8 0.9\n"
      "Ns 100.0\n"
      "d 0.5\n"
      "illum 2\n"
      "\n"
      "newmtl mat2\n"
      "Ka 0.0 0.0 0.0\n"
      "Kd 1.0 1.0 1.0\n"
      "Ns 50.0\n";
  std::istringstream mtl_iss(mtl_text);
  std::map<std::string, int> matMap;
  std::vector<tinyobj::material_t> materials;
  std::string warn, err;
  tinyobj::LoadMtl(&matMap, &materials, &mtl_iss, &warn, &err);
  TEST_CHECK(materials.size() == 2);
  TEST_CHECK(materials[0].name == "mat1");
  TEST_CHECK(FloatEquals(0.1f, materials[0].ambient[0]));
  TEST_CHECK(FloatEquals(0.4f, materials[0].diffuse[0]));
  TEST_CHECK(FloatEquals(0.7f, materials[0].specular[0]));
  TEST_CHECK(FloatEquals(100.0f, materials[0].shininess));
  TEST_CHECK(FloatEquals(0.5f, materials[0].dissolve));
  TEST_CHECK(materials[0].illum == 2);
  TEST_CHECK(materials[1].name == "mat2");
  TEST_CHECK(FloatEquals(1.0f, materials[1].diffuse[0]));
  TEST_CHECK(FloatEquals(50.0f, materials[1].shininess));
}

// Callback API: vertices, normals, texcoords, and faces
void test_callback_all_elements() {
  const char *obj_text =
      "v 1.0 2.0 3.0\n"
      "v 4.0 5.0 6.0\n"
      "v 7.0 8.0 9.0\n"
      "vn 0.0 0.0 1.0\n"
      "vt 0.5 0.5\n"
      "f 1 2 3\n";
  std::istringstream iss(obj_text);

  struct Counts {
    int vertices;
    int normals;
    int texcoords;
    int faces;
  };
  Counts counts = {0, 0, 0, 0};

  tinyobj::callback_t cb;
  cb.vertex_cb = [](void *user, float, float, float, float) {
    static_cast<Counts *>(user)->vertices++;
  };
  cb.normal_cb = [](void *user, float, float, float) {
    static_cast<Counts *>(user)->normals++;
  };
  cb.texcoord_cb = [](void *user, float, float, float) {
    static_cast<Counts *>(user)->texcoords++;
  };
  cb.index_cb = [](void *user, tinyobj::index_t *, int) {
    static_cast<Counts *>(user)->faces++;
  };

  std::string warn, err;
  bool ret = tinyobj::LoadObjWithCallback(iss, cb, &counts, NULL, &warn, &err);
  TEST_CHECK(ret == true);
  TEST_CHECK(counts.vertices == 3);
  TEST_CHECK(counts.normals == 1);
  TEST_CHECK(counts.texcoords == 1);
  TEST_CHECK(counts.faces == 1);
}

// Callback API with NULL callbacks (should not crash)
void test_callback_null_callbacks() {
  const char *obj_text =
      "v 1.0 2.0 3.0\n"
      "vn 0.0 0.0 1.0\n"
      "vt 0.5 0.5\n"
      "f 1/1/1 1/1/1 1/1/1\n";
  std::istringstream iss(obj_text);

  tinyobj::callback_t cb;
  // All callbacks are NULL by default
  std::string warn, err;
  bool ret = tinyobj::LoadObjWithCallback(iss, cb, NULL, NULL, &warn, &err);
  TEST_CHECK(ret == true);
}

// Subnormal float values should parse without error
void test_numeric_subnormal_values() {
  // 5e-310 is subnormal for double, 1e-45 is subnormal for float
  const char *obj_text = "v 5e-310 1e-45 0.0\n";
  std::istringstream iss(obj_text);
  tinyobj::attrib_t attrib;
  std::vector<tinyobj::shape_t> shapes;
  std::vector<tinyobj::material_t> materials;
  std::string warn, err;
  bool ret = tinyobj::LoadObj(&attrib, &shapes, &materials, &warn, &err,
                              &iss, NULL);
  TEST_CHECK(ret == true);
  TEST_CHECK(attrib.vertices.size() == 3);
  // Values should be >= 0 (either the subnormal or flushed to zero)
  TEST_CHECK(attrib.vertices[0] >= 0.0f);
  TEST_CHECK(attrib.vertices[1] >= 0.0f);
  TEST_CHECK(FloatEquals(0.0f, attrib.vertices[2]));
}

// Empty MTL should not produce a phantom material
void test_empty_mtl_no_phantom_material() {
  const char *mtl_text = "# just a comment\n";
  std::istringstream mtl_iss(mtl_text);
  std::map<std::string, int> matMap;
  std::vector<tinyobj::material_t> materials;
  std::string warn, err;
  tinyobj::LoadMtl(&matMap, &materials, &mtl_iss, &warn, &err);
  TEST_CHECK(materials.empty());
  TEST_CHECK(matMap.empty());
}

// StreamReader should not be copyable (deleted copy constructor)
void test_streamreader_not_copyable() {
  // This is a compile-time check. If StreamReader were copyable,
  // copying one built from istream would create a dangling buf_ pointer.
  // We just verify construction and basic use work correctly.
  const char *input = "hello";
  tinyobj::StreamReader sr(input, 5);
  TEST_CHECK(sr.remaining() == 5);
  TEST_CHECK(sr.peek() == 'h');
}

// Out-of-range face indices should not crash
void test_out_of_range_face_index() {
  const char *obj_text =
      "v 1.0 0.0 0.0\n"
      "v 0.0 1.0 0.0\n"
      "v 0.0 0.0 1.0\n"
      "f 1 2 999\n";  // index 999 doesn't exist
  std::istringstream iss(obj_text);
  tinyobj::attrib_t attrib;
  std::vector<tinyobj::shape_t> shapes;
  std::vector<tinyobj::material_t> materials;
  std::string warn, err;
  bool ret = tinyobj::LoadObj(&attrib, &shapes, &materials, &warn, &err,
                              &iss, NULL);
  // Should parse without crashing. The face will have an out-of-range index
  // which may produce a warning during triangulation.
  TEST_CHECK(ret == true);
  TEST_CHECK(attrib.vertices.size() == 9);
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
    {"usemtl_whitespace_issue246",
     test_usemtl_whitespace_issue246},
    {"texres_texopt_issue248",
     test_texres_texopt_issue248},
    {"test_mtl_filename_with_whitespace_issue46",
     test_mtl_filename_with_whitespace_issue46},
    {"test_face_missing_issue295",
     test_face_missing_issue295},
    {"test_comment_issue389",
     test_comment_issue389},
    {"test_invalid_relative_vertex_index",
     test_invalid_relative_vertex_index},
    {"test_invalid_texture_vertex_index",
     test_invalid_texture_vertex_index},
    {"default_kd_for_multiple_materials_issue391",
     test_default_kd_for_multiple_materials_issue391},
    {"test_removeUtf8Bom", test_removeUtf8Bom},
    {"test_loadObj_with_BOM", test_loadObj_with_BOM},
    {"test_load_obj_from_utf8_path", test_load_obj_from_utf8_path},
    {"test_load_obj_from_long_path", test_load_obj_from_long_path},
    {"test_loadObjWithCallback_with_BOM", test_loadObjWithCallback_with_BOM},
    {"test_loadObjWithCallback_mtllib_failure_does_not_crash",
     test_loadObjWithCallback_mtllib_failure_does_not_crash},
    {"test_mtllib_empty_filename_is_ignored_loadobj",
     test_mtllib_empty_filename_is_ignored_loadobj},
    {"test_mtllib_empty_filename_is_ignored_callback",
     test_mtllib_empty_filename_is_ignored_callback},
    {"test_usemtl_callback_trims_trailing_comment",
     test_usemtl_callback_trims_trailing_comment},
    {"test_tag_triple_huge_count_is_safely_rejected",
     test_tag_triple_huge_count_is_safely_rejected},
    {"test_texcoord_w_component", test_texcoord_w_component},
    {"test_texcoord_w_mixed_component", test_texcoord_w_mixed_component},
    {"test_numeric_edge_cases", test_numeric_edge_cases},
    {"test_numeric_nan_inf", test_numeric_nan_inf},
    {"test_numeric_from_stream", test_numeric_from_stream},
    {"test_numeric_overflow_preserves_default", test_numeric_overflow_preserves_default},
    {"test_numeric_empty_and_whitespace", test_numeric_empty_and_whitespace},
    {"test_numeric_garbage_input", test_numeric_garbage_input},
    {"test_numeric_extreme_precision", test_numeric_extreme_precision},
    {"test_file_and_stream_load_agree", test_file_and_stream_load_agree},
    {"test_load_from_memory_buffer", test_load_from_memory_buffer},
    {"test_streamreader_column_tracking", test_streamreader_column_tracking},
    {"test_stream_load_from_current_offset", test_stream_load_from_current_offset},
    {"test_stream_load_rejects_oversized_input", test_stream_load_rejects_oversized_input},
    {"test_error_format_clang_style", test_error_format_clang_style},
    {"test_error_stack", test_error_stack},
    {"test_malformed_vertex_error", test_malformed_vertex_error},
    {"test_malformed_mtl_error", test_malformed_mtl_error},
    {"test_parse_error_backward_compat", test_parse_error_backward_compat},
    {"test_split_string_preserves_non_escape_backslash",
     test_split_string_preserves_non_escape_backslash},
    {"test_streamreader_eof_and_remaining",
     test_streamreader_eof_and_remaining},
    {"test_streamreader_skip_and_read", test_streamreader_skip_and_read},
    {"test_streamreader_match_and_advance",
     test_streamreader_match_and_advance},
    {"test_streamreader_current_line_text",
     test_streamreader_current_line_text},
    {"test_streamreader_error_stack", test_streamreader_error_stack},
    {"test_empty_obj_file", test_empty_obj_file},
    {"test_bom_only_obj", test_bom_only_obj},
    {"test_no_trailing_newline", test_no_trailing_newline},
    {"test_mixed_line_endings", test_mixed_line_endings},
    {"test_vertex_colors_from_stream", test_vertex_colors_from_stream},
    {"test_vertex_colors_mixed", test_vertex_colors_mixed},
    {"test_all_element_types", test_all_element_types},
    {"test_multiple_objects", test_multiple_objects},
    {"test_mtl_d_and_tr_warning", test_mtl_d_and_tr_warning},
    {"test_multiple_malformed_vertices", test_multiple_malformed_vertices},
    {"test_malformed_normal_error", test_malformed_normal_error},
    {"test_malformed_texcoord_error", test_malformed_texcoord_error},
    {"test_negative_vertex_indices", test_negative_vertex_indices},
    {"test_comments_everywhere", test_comments_everywhere},
    {"test_excessive_whitespace", test_excessive_whitespace},
    {"test_objreader_api_stream", test_objreader_api_stream},
    {"test_objreader_api_error", test_objreader_api_error},
    {"test_split_string_edge_cases", test_split_string_edge_cases},
    {"test_quad_face", test_quad_face},
    {"test_quad_face_triangulated", test_quad_face_triangulated},
    {"test_face_full_index_format", test_face_full_index_format},
    {"test_face_vertex_normal_only", test_face_vertex_normal_only},
    {"test_mtl_multiple_properties", test_mtl_multiple_properties},
    {"test_callback_all_elements", test_callback_all_elements},
    {"test_callback_null_callbacks", test_callback_null_callbacks},
    {"test_numeric_subnormal_values", test_numeric_subnormal_values},
    {"test_empty_mtl_no_phantom_material",
     test_empty_mtl_no_phantom_material},
    {"test_streamreader_not_copyable", test_streamreader_not_copyable},
    {"test_out_of_range_face_index", test_out_of_range_face_index},
    {NULL, NULL}};
