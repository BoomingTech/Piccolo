import sys
import tinyobjloader

is_numpy_available = False
try:
    import numpy

    is_numpy_available = True
except:
    print(
        "NumPy not installed. Do not use numpy_*** API. If you encounter slow performance, see a performance tips for non-numpy API https://github.com/tinyobjloader/tinyobjloader/issues/275"
    )

filename = "../models/cornell_box.obj"

if len(sys.argv) > 1:
    filename = sys.argv[1]


reader = tinyobjloader.ObjReader()

# Load .obj(and .mtl) using default configuration
ret = reader.ParseFromFile(filename)

# Optionally you can set custom `config`
# config = tinyobj.ObjReaderConfig()
# config.triangulate = False
# ret = reader.ParseFromFile(filename, config)

if ret == False:
    print("Failed to load : ", filename)
    print("Warn:", reader.Warning())
    print("Err:", reader.Error())
    sys.exit(-1)

if reader.Warning():
    print("Warn:", reader.Warning())

attrib = reader.GetAttrib()
print("len(attrib.vertices) = ", len(attrib.vertices))
print("len(attrib.vertex_weights) = ", len(attrib.vertex_weights))
print("len(attrib.normals) = ", len(attrib.normals))
print("len(attrib.texcoords) = ", len(attrib.texcoords))
print("len(attrib.colors) = ", len(attrib.colors))
print("len(attrib.skin_weights) = ", len(attrib.skin_weights))

# vertex data must be `xyzxyzxyz...`
assert len(attrib.vertices) % 3 == 0

# normal data must be `xyzxyzxyz...`
assert len(attrib.normals) % 3 == 0

# texcoords data must be `uvuvuv...`
assert len(attrib.texcoords) % 2 == 0

# colors data must be `rgbrgbrgb...`
assert len(attrib.texcoords) % 3 == 0

# Performance note
# (direct?) array access through member variable is quite slow.
# https://github.com/tinyobjloader/tinyobjloader/issues/275#issuecomment-753465833
#
# We encourage first copy(?) varible to Python world:
#
# vertices = attrib.vertices
#
# for i in range(...)
#   v = vertices[i]
#
# Or please consider using numpy_*** interface(e.g. numpy_vertices())

for i, v in enumerate(attrib.vertices):
    print("v[{}] = {}".format(i, v))

# vw is filled with 1.0 if [w] component is not present in `v` line in .obj
for i, w in enumerate(attrib.vertex_weights):
    print("vweight[{}] = {}".format(i, w))

for i, v in enumerate(attrib.normals):
    print("vn[{}] = {}".format(i, v))

for i, v in enumerate(attrib.texcoords):
    print("vt[{}] = {}".format(i, v))

for i, v in enumerate(attrib.colors):
    print("vcol[{}] = {}".format(i, v))

if len(attrib.skin_weights):
    print("num skin weights", len(attrib.skin_weights))

    for i, skin in enumerate(attrib.skin_weights):
        print("skin_weight[{}]".format(i))
        print("  vertex_id = ", skin.vertex_id)
        print("  len(weights) = ", len(skin.weightValues))
        for k, w in enumerate(skin.weightValues):
            print("    [{}] joint_id: {}, weight: {}".format(k, w.joint_id, w.weight))

if is_numpy_available:
    print("numpy_v = {}".format(attrib.numpy_vertices()))
    print("numpy_vn = {}".format(attrib.numpy_normals()))
    print("numpy_vt = {}".format(attrib.numpy_texcoords()))
    print("numpy_vcol = {}".format(attrib.numpy_colors()))

materials = reader.GetMaterials()
print("Num materials: ", len(materials))
for m in materials:
    print(m.name)
    print(m.diffuse)
    print(m.diffuse_texname)
    # Partial update(array indexing) does not work
    # m.diffuse[1] = 1.0

    # Update with full object assignment works
    m.diffuse = [1, 2, 3]
    print(m.diffuse)

    # print(m.shininess)
    # print(m.illum)

shapes = reader.GetShapes()
print("Num shapes: ", len(shapes))
for shape in shapes:
    print(shape.name)
    print("len(num_indices) = {}".format(len(shape.mesh.indices)))
    for i, idx in enumerate(shape.mesh.indices):
        print("[{}] v_idx {}".format(i, idx.vertex_index))
        print("[{}] vn_idx {}".format(i, idx.normal_index))
        print("[{}] vt_idx {}".format(i, idx.texcoord_index))
    print("material_ids = {}".format(shape.mesh.material_ids))

    # faster access to indices
    a = shape.mesh.vertex_indices()
    print("vertex_indices", shape.mesh.vertex_indices())

    if is_numpy_available:
        print("numpy_indices = {}".format(shape.mesh.numpy_indices()))
        print("numpy_num_face_vertices = {}".format(shape.mesh.numpy_num_face_vertices()))
        print("numpy_material_ids = {}".format(shape.mesh.numpy_material_ids()))
