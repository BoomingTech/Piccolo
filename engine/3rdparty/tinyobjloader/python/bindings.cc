#include <pybind11/pybind11.h>
#include <pybind11/stl.h>
#include <pybind11/numpy.h>
#include <cstring>

// Use double precision for better python integration.
#define TINYOBJLOADER_USE_DOUBLE

// define some helper functions for pybind11
#define TINY_OBJ_LOADER_PYTHON_BINDING
#include "../tiny_obj_loader.h"

namespace py = pybind11;

using namespace tinyobj;

PYBIND11_MODULE(tinyobjloader, tobj_module)
{
  tobj_module.doc() = "Python bindings for TinyObjLoader.";

  // register struct
  py::class_<ObjReaderConfig>(tobj_module, "ObjReaderConfig")
    .def(py::init<>())
    .def_readwrite("triangulate", &ObjReaderConfig::triangulate);

  // py::init<>() for default constructor
  py::class_<ObjReader>(tobj_module, "ObjReader")
    .def(py::init<>())
    .def("ParseFromFile", &ObjReader::ParseFromFile, py::arg("filename"), py::arg("option") = ObjReaderConfig())
    .def("ParseFromString", &ObjReader::ParseFromString, py::arg("obj_text"), py::arg("mtl_text"), py::arg("option") = ObjReaderConfig())
    .def("Valid", &ObjReader::Valid)
    .def("GetAttrib", &ObjReader::GetAttrib)
    .def("GetShapes", &ObjReader::GetShapes)
    .def("GetMaterials", &ObjReader::GetMaterials)
    .def("Warning", &ObjReader::Warning)
    .def("Error", &ObjReader::Error);

  py::class_<attrib_t>(tobj_module, "attrib_t")
    .def(py::init<>())
    .def_readonly("vertices", &attrib_t::vertices)
    .def_readonly("vertex_weights", &attrib_t::vertex_weights)
    .def_readonly("skin_weights", &attrib_t::skin_weights)
    .def_readonly("normals", &attrib_t::normals)
    .def_readonly("texcoords", &attrib_t::texcoords)
    .def_readonly("colors", &attrib_t::colors)
    .def("numpy_vertices", [] (attrib_t &instance) {
        auto ret = py::array_t<real_t>(instance.vertices.size());
        py::buffer_info buf = ret.request();
        memcpy(buf.ptr, instance.vertices.data(), instance.vertices.size() * sizeof(real_t));
        return ret;
    })
    .def("numpy_vertex_weights", [] (attrib_t &instance) {
        auto ret = py::array_t<real_t>(instance.vertex_weights.size());
        py::buffer_info buf = ret.request();
        memcpy(buf.ptr, instance.vertex_weights.data(), instance.vertex_weights.size() * sizeof(real_t));
        return ret;
    })
    .def("numpy_normals", [] (attrib_t &instance) {
        auto ret = py::array_t<real_t>(instance.normals.size());
        py::buffer_info buf = ret.request();
        memcpy(buf.ptr, instance.normals.data(), instance.normals.size() * sizeof(real_t));
        return ret;
    })
    .def("numpy_texcoords", [] (attrib_t &instance) {
        auto ret = py::array_t<real_t>(instance.texcoords.size());
        py::buffer_info buf = ret.request();
        memcpy(buf.ptr, instance.texcoords.data(), instance.texcoords.size() * sizeof(real_t));
        return ret;
    })
    .def("numpy_colors", [] (attrib_t &instance) {
        auto ret = py::array_t<real_t>(instance.colors.size());
        py::buffer_info buf = ret.request();
        memcpy(buf.ptr, instance.colors.data(), instance.colors.size() * sizeof(real_t));
        return ret;
    })
    ;

  py::class_<shape_t>(tobj_module, "shape_t")
    .def(py::init<>())
    .def_readwrite("name", &shape_t::name)
    .def_readwrite("mesh", &shape_t::mesh)
    .def_readwrite("lines", &shape_t::lines)
    .def_readwrite("points", &shape_t::points);

  py::class_<index_t>(tobj_module, "index_t")
    .def(py::init<>())
    .def_readwrite("vertex_index", &index_t::vertex_index)
    .def_readwrite("normal_index", &index_t::normal_index)
    .def_readwrite("texcoord_index", &index_t::texcoord_index)
    ;

  // NOTE(syoyo): It looks it is rather difficult to expose assignment by array index to
  // python world for array variable.
  // For example following python scripting does not work well.
  //
  // print(mat.diffuse)
  // >>> [0.1, 0.2, 0.3]
  // mat.diffuse[1] = 1.0
  // print(mat.diffuse)
  // >>> [0.1, 0.2, 0.3]  # No modification
  //
  // https://github.com/pybind/pybind11/issues/1134
  //
  // so, we need to update array variable like this:
  //
  // diffuse = mat.diffuse
  // diffuse[1] = 1.0
  // mat.diffuse = diffuse
  //
  py::class_<material_t>(tobj_module, "material_t")
    .def(py::init<>())
    .def_readwrite("name", &material_t::name)
    .def_property("ambient", &material_t::GetAmbient, &material_t::SetAmbient)
    .def_property("diffuse", &material_t::GetDiffuse, &material_t::SetDiffuse)
    .def_property("specular", &material_t::GetSpecular, &material_t::SetSpecular)
    .def_property("transmittance", &material_t::GetTransmittance, &material_t::SetTransmittance)
    .def_readwrite("shininess", &material_t::shininess)
    .def_readwrite("ior", &material_t::ior)
    .def_readwrite("dissolve", &material_t::dissolve)
    .def_readwrite("illum", &material_t::illum)
    .def_readwrite("ambient_texname", &material_t::ambient_texname)
    .def_readwrite("diffuse_texname", &material_t::diffuse_texname)
    .def_readwrite("specular_texname", &material_t::specular_texname)
    .def_readwrite("specular_highlight_texname", &material_t::specular_highlight_texname)
    .def_readwrite("bump_texname", &material_t::bump_texname)
    .def_readwrite("displacement_texname", &material_t::displacement_texname)
    .def_readwrite("alpha_texname", &material_t::alpha_texname)
    .def_readwrite("reflection_texname", &material_t::reflection_texname)
    // TODO(syoyo): Expose texture parameter
    // PBR
    .def_readwrite("roughness", &material_t::roughness)
    .def_readwrite("metallic", &material_t::metallic)
    .def_readwrite("sheen", &material_t::sheen)
    .def_readwrite("clearcoat_thickness", &material_t::clearcoat_thickness)
    .def_readwrite("clearcoat_roughness", &material_t::clearcoat_roughness)
    .def_readwrite("anisotropy", &material_t::anisotropy)
    .def_readwrite("anisotropy_rotation", &material_t::anisotropy_rotation)

    .def_readwrite("roughness_texname", &material_t::roughness_texname)
    .def_readwrite("metallic_texname", &material_t::metallic_texname)
    .def_readwrite("sheen_texname", &material_t::sheen_texname)
    .def_readwrite("emissive_texname", &material_t::emissive_texname)
    .def_readwrite("normal_texname", &material_t::normal_texname)

    .def("GetCustomParameter", &material_t::GetCustomParameter)
    ;

  py::class_<mesh_t>(tobj_module, "mesh_t", py::buffer_protocol())
    .def(py::init<>())
    .def_readonly("num_face_vertices", &mesh_t::num_face_vertices)
    .def("numpy_num_face_vertices", [] (mesh_t &instance) {
        using T = typename std::remove_reference<decltype(instance.num_face_vertices)>::type::value_type;
        auto ret = py::array_t<T>(instance.num_face_vertices.size());
        py::buffer_info buf = ret.request();
        memcpy(buf.ptr, instance.num_face_vertices.data(), instance.num_face_vertices.size() * sizeof(T));
        return ret;
    })
    .def("vertex_indices", [](mesh_t &self) {
      // NOTE: we cannot use py::buffer_info and py:buffer as a return type.
      // py::memoriview is not suited for vertex indices usecase, since indices data may be used after
      // deleting C++ mesh_t object in Python world.
      //
      // So create a dedicated Python object(std::vector<int>) 
      
      std::vector<int> indices;
      indices.resize(self.indices.size());
      for (size_t i = 0; i < self.indices.size(); i++) {
        indices[i] = self.indices[i].vertex_index;
      }

      return indices;
    })
    .def("normal_indices", [](mesh_t &self) {
      
      std::vector<int> indices;
      indices.resize(self.indices.size());
      for (size_t i = 0; i < self.indices.size(); i++) {
        indices[i] = self.indices[i].normal_index;
      }

      return indices;
    })
    .def("texcoord_indices", [](mesh_t &self) {
      
      std::vector<int> indices;
      indices.resize(self.indices.size());
      for (size_t i = 0; i < self.indices.size(); i++) {
        indices[i] = self.indices[i].texcoord_index;
      }

      return indices;
    })
    .def_readonly("indices", &mesh_t::indices)
    .def("numpy_indices", [] (mesh_t &instance) {
        // Flatten indexes. index_t is composed of 3 ints(vertex_index, normal_index, texcoord_index).
        // numpy_indices = [0, -1, -1, 1, -1, -1, ...]
        // C++11 or later should pack POD struct tightly and does not reorder variables,
        // so we can memcpy to copy data.
        // Still, we check the size of struct and byte offsets of each variable just for sure.
        static_assert(sizeof(index_t) == 12, "sizeof(index_t) must be 12");
        static_assert(offsetof(index_t, vertex_index) == 0, "offsetof(index_t, vertex_index) must be 0");
        static_assert(offsetof(index_t, normal_index) == 4, "offsetof(index_t, normal_index) must be 4");
        static_assert(offsetof(index_t, texcoord_index) == 8, "offsetof(index_t, texcoord_index) must be 8");

        auto ret = py::array_t<int>(instance.indices.size() * 3);
        py::buffer_info buf = ret.request();
        memcpy(buf.ptr, instance.indices.data(), instance.indices.size() * 3 * sizeof(int));
        return ret;
    })
    .def_readonly("material_ids", &mesh_t::material_ids)
    .def("numpy_material_ids", [] (mesh_t &instance) {
        auto ret = py::array_t<int>(instance.material_ids.size());
        py::buffer_info buf = ret.request();
        memcpy(buf.ptr, instance.material_ids.data(), instance.material_ids.size() * sizeof(int));
        return ret;
    });

  py::class_<lines_t>(tobj_module, "lines_t")
    .def(py::init<>())
    .def_readonly("indices", &lines_t::indices)
    .def_readonly("num_line_vertices", &lines_t::num_line_vertices)
    ;

  py::class_<points_t>(tobj_module, "points_t")
    .def(py::init<>())
    .def_readonly("indices", &points_t::indices)
    ;

  py::class_<joint_and_weight_t>(tobj_module, "joint_and_weight_t")
    .def(py::init<>())
    .def_readonly("joint_id", &joint_and_weight_t::joint_id, "Joint index(NOTE: Joint info is provided externally, not from .obj")
    .def_readonly("weight", &joint_and_weight_t::weight, "Weight value(NOTE: weight is not normalized)")
    ;

  py::class_<skin_weight_t>(tobj_module, "skin_weight_t")
    .def(py::init<>())
    .def_readonly("vertex_id", &skin_weight_t::vertex_id)
    .def_readonly("weightValues", &skin_weight_t::weightValues)
    ;

  py::class_<tag_t>(tobj_module, "tag_t")
    .def(py::init<>())
    .def_readonly("name", &tag_t::name)
    .def_readonly("intValues", &tag_t::intValues)  
    .def_readonly("floatValues", &tag_t::floatValues)  
    .def_readonly("stringValues", &tag_t::stringValues)  
    ;
}

