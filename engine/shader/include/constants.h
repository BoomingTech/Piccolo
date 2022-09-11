#define m_max_point_light_count 15
#define m_max_point_light_geom_vertices 90 // 90 = 2 * 3 * m_max_point_light_count
#define m_mesh_per_drawcall_max_instance_count 64
#define m_mesh_vertex_blending_max_joint_count 1024
#define CHAOS_LAYOUT_MAJOR row_major
layout(CHAOS_LAYOUT_MAJOR) buffer;
layout(CHAOS_LAYOUT_MAJOR) uniform;