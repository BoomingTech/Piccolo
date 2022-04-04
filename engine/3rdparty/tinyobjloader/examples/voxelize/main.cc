#define VOXELIZER_IMPLEMENTATION
#include "voxelizer.h"

#define TINYOBJLOADER_IMPLEMENTATION
#include "../../tiny_obj_loader.h"

bool Voxelize(const char* filename, float voxelsizex, float voxelsizey, float voxelsizez, float precision)
{
    tinyobj::attrib_t attrib;
    std::vector<tinyobj::shape_t> shapes;
    std::vector<tinyobj::material_t> materials;
    std::string warn;
    std::string err;
    bool ret = tinyobj::LoadObj(&attrib, &shapes, &materials, &warn, &err, filename);

    if (!err.empty()) {
      printf("err: %s\n", err.c_str());
    }

    if (!ret) {
      printf("failed to load : %s\n", filename);
      return false;
    }

    if (shapes.size() == 0) {
      printf("err: # of shapes are zero.\n");
      return false;
    }

    // Only use first shape.
    {
        vx_mesh_t* mesh;
        vx_mesh_t* result;

        mesh = vx_mesh_alloc(attrib.vertices.size(), shapes[0].mesh.indices.size());

        for (size_t f = 0; f < shapes[0].mesh.indices.size(); f++) {
            mesh->indices[f] = shapes[0].mesh.indices[f].vertex_index;
        }

        for (size_t v = 0; v < attrib.vertices.size() / 3; v++) {
            mesh->vertices[v].x = attrib.vertices[3*v+0];
            mesh->vertices[v].y = attrib.vertices[3*v+1];
            mesh->vertices[v].z = attrib.vertices[3*v+2];
        }

        result = vx_voxelize(mesh, voxelsizex, voxelsizey, voxelsizez, precision);

        printf("Number of vertices: %ld\n", result->nvertices);
        printf("Number of indices: %ld\n", result->nindices);
    }
    return true;
}


int
main(
  int argc,
  char** argv)
{
  if (argc < 4) {
    printf("Usage: voxelize input.obj voxelsizex voxelsizey voxelsizez precision\n");
    exit(-1);
  }

  const char* filename = argv[1];
  float voxelsizex = atof(argv[2]);
  float voxelsizey = atof(argv[3]);
  float voxelsizez = atof(argv[4]);
  float prec = atof(argv[5]);
  bool ret = Voxelize(filename, voxelsizex, voxelsizey, voxelsizez, prec);

  return ret ? EXIT_SUCCESS : EXIT_FAILURE;
}

