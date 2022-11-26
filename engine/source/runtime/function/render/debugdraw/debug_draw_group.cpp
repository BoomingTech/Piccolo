#include "debug_draw_group.h"
#include <vector>
#include "runtime/function/global/global_context.h"
#include "runtime/function/render/render_system.h"

namespace Piccolo
{
    DebugDrawGroup::~DebugDrawGroup() { clear(); }
    void DebugDrawGroup::initialize()
    {
    }

    void DebugDrawGroup::clear()
    {
        std::lock_guard<std::mutex> guard(m_mutex);
        clearData();
    }

    void DebugDrawGroup::clearData()
    {
        m_points.clear();
        m_lines.clear();
        m_triangles.clear();
        m_quads.clear();
        m_boxes.clear();
        m_cylinders.clear();
        m_spheres.clear();
        m_capsules.clear();
        m_texts.clear();
    }

    void DebugDrawGroup::setName(const std::string& name) { m_name = name; }

    const std::string& DebugDrawGroup::getName() const{return m_name;}

    void DebugDrawGroup::addPoint(const Vector3& position, const Vector4& color, const float life_time, const bool no_depth_test)
    {
        std::lock_guard<std::mutex> guard(m_mutex);
        DebugDrawPoint point;
        point.m_vertex.color = color;
        point.setTime(life_time);
        point.m_fill_mode = _FillMode_wireframe;
        point.m_vertex.pos = position;
        point.m_no_depth_test = no_depth_test;
        m_points.push_back(point);
    }

    void DebugDrawGroup::addLine(const Vector3& point0, 
                                 const Vector3& point1,
                                 const Vector4& color0, 
                                 const Vector4& color1, 
                                 const float    life_time,
                                 const bool     no_depth_test)
    {
        std::lock_guard<std::mutex> guard(m_mutex);
        DebugDrawLine line;
        line.setTime(life_time);
        line.m_fill_mode = _FillMode_wireframe;
        line.m_no_depth_test = no_depth_test;

        line.m_vertex[0].pos     = point0;
        line.m_vertex[0].color = color0;

        line.m_vertex[1].pos     = point1;
        line.m_vertex[1].color = color1;

        m_lines.push_back(line);
    }

    void DebugDrawGroup::addTriangle(const Vector3& point0,
                                     const Vector3& point1,
                                     const Vector3& point2,
                                     const Vector4& color0,
                                     const Vector4& color1,
                                     const Vector4& color2,
                                     const float    life_time,
                                     const bool     no_depth_test,
                                     const FillMode fillmod)
    {
        std::lock_guard<std::mutex> guard(m_mutex);
        DebugDrawTriangle triangle;
        triangle.setTime(life_time);
        triangle.m_fill_mode = fillmod;
        triangle.m_no_depth_test = no_depth_test;

        triangle.m_vertex[0].pos   = point0;
        triangle.m_vertex[0].color = color0;

        triangle.m_vertex[1].pos   = point1;
        triangle.m_vertex[1].color = color1;

        triangle.m_vertex[2].pos   = point2;
        triangle.m_vertex[2].color = color2;
        
        m_triangles.push_back(triangle);
        

    }

    void DebugDrawGroup::addQuad(const Vector3& point0,
                                 const Vector3& point1,
                                 const Vector3& point2,
                                 const Vector3& point3,
                                 const Vector4& color0,
                                 const Vector4& color1,
                                 const Vector4& color2,
                                 const Vector4& color3,
                                 const float    life_time,
                                 const bool     no_depth_test,
                                 const FillMode fillmode)
    {
        std::lock_guard<std::mutex> guard(m_mutex);
        if (fillmode == _FillMode_wireframe)
        {
            DebugDrawQuad quad;

            quad.m_vertex[0].pos   = point0;
            quad.m_vertex[0].color = color0;

            quad.m_vertex[1].pos   = point1;
            quad.m_vertex[1].color = color1;
            
            quad.m_vertex[2].pos   = point2;
            quad.m_vertex[2].color = color2;
            
            quad.m_vertex[3].pos   = point3;
            quad.m_vertex[3].color = color3;

            quad.setTime(life_time);
            quad.m_no_depth_test = no_depth_test;

            m_quads.push_back(quad);
        }
        else
        {
            DebugDrawTriangle triangle;
            triangle.setTime(life_time);
            triangle.m_fill_mode         = _FillMode_solid;
            triangle.m_no_depth_test   = no_depth_test;

            triangle.m_vertex[0].pos     = point0;
            triangle.m_vertex[0].color   = color0;
            triangle.m_vertex[1].pos     = point1;
            triangle.m_vertex[1].color   = color1;
            triangle.m_vertex[2].pos     = point2;
            triangle.m_vertex[2].color   = color2;
            m_triangles.push_back(triangle);

            triangle.m_vertex[0].pos     = point0;
            triangle.m_vertex[0].color = color0;
            triangle.m_vertex[1].pos     = point2;
            triangle.m_vertex[1].color = color2;
            triangle.m_vertex[2].pos     = point3;
            triangle.m_vertex[2].color = color3;
            m_triangles.push_back(triangle);
        }
    }

    void DebugDrawGroup::addBox(const Vector3& center_point,
                                const Vector3& half_extends,
                                const Vector4& rotate,
                                const Vector4& color,
                                const float    life_time,
                                const bool     no_depth_test)
    {
        std::lock_guard<std::mutex> guard(m_mutex);
        DebugDrawBox box;
        box.m_center_point = center_point;
        box.m_half_extents = half_extends;
        box.m_rotate = rotate;
        box.m_color = color;
        box.m_no_depth_test = no_depth_test;
        box.setTime(life_time);

        m_boxes.push_back(box);
    }

    void DebugDrawGroup::addSphere(const Vector3& center,
                                   const float    radius,
                                   const Vector4& color,
                                   const float    life_time,
                                   const bool     no_depth_test)
    {
        std::lock_guard<std::mutex> guard(m_mutex);
        DebugDrawSphere sphere;
        sphere.m_center = center;
        sphere.m_radius = radius;
        sphere.m_color = color;
        sphere.m_no_depth_test = no_depth_test;
        sphere.setTime(life_time);

        m_spheres.push_back(sphere);
    }

    void DebugDrawGroup::addCylinder(const Vector3& center, 
                                     const float    radius, 
                                     const float    height, 
                                     const Vector4& rotate,
                                     const Vector4& color, 
                                     const float    life_time, 
                                     const bool     no_depth_test)
    {
        std::lock_guard<std::mutex> guard(m_mutex);
        DebugDrawCylinder cylinder;
        cylinder.m_radius = radius;
        cylinder.m_center = center;
        cylinder.m_height = height;
        cylinder.m_rotate = rotate;
        cylinder.m_color = color;
        cylinder.m_no_depth_test = no_depth_test;
        cylinder.setTime(life_time);

        m_cylinders.push_back(cylinder);
    }

    void DebugDrawGroup::addCapsule(const Vector3& center,
                                    const Vector4& rotation,
                                    const Vector3& scale, 
                                    const float    radius, 
                                    const float    height, 
                                    const Vector4& color, 
                                    const float    life_time,
                                    const bool     no_depth_test)
    {
        std::lock_guard<std::mutex> guard(m_mutex);
        DebugDrawCapsule capsule;
        capsule.m_center = center;
        capsule.m_rotation = rotation;
        capsule.m_scale = scale;
        capsule.m_radius = radius;
        capsule.m_height = height;
        capsule.m_color = color;
        capsule.m_no_depth_test = no_depth_test;
        capsule.setTime(life_time);

        m_capsules.push_back(capsule);
    }

    void DebugDrawGroup::addText(const std::string& content,
                                 const Vector4&     color,
                                 const Vector3&     coordinate,
                                 const int          size,
                                 const bool         is_screen_text,
                                 const float        life_time)
    {
        std::lock_guard<std::mutex> guard(m_mutex);
        DebugDrawText text;
        text.m_content = content;
        text.m_color = color;
        text.m_coordinate = coordinate;
        text.m_size = size;
        text.m_is_screen_text = is_screen_text;
        text.setTime(life_time);
        m_texts.push_back(text);
    }

    void DebugDrawGroup::removeDeadPrimitives(float delta_time)
    {
        for (std::list<DebugDrawPoint>::iterator point = m_points.begin(); point != m_points.end();)
        {
            if (point->isTimeOut(delta_time)) {
                point = m_points.erase(point);
            }
            else {
                point++;
            }
        }
        for (std::list<DebugDrawLine>::iterator line = m_lines.begin(); line != m_lines.end();)
        {
            if (line->isTimeOut(delta_time)) {
                line = m_lines.erase(line);
            }
            else {
                line++;
            }
        }
        for (std::list<DebugDrawTriangle>::iterator triangle = m_triangles.begin(); triangle != m_triangles.end();)
        {
            if (triangle->isTimeOut(delta_time)) {
                triangle = m_triangles.erase(triangle);
            }
            else {
                triangle++;
            }
        }
        for (std::list<DebugDrawQuad>::iterator quad = m_quads.begin(); quad != m_quads.end();)
        {
            if (quad->isTimeOut(delta_time)) {
                quad = m_quads.erase(quad);
            }
            else {
                quad++;
            }
        }
        for (std::list<DebugDrawBox>::iterator box = m_boxes.begin(); box != m_boxes.end();)
        {
            if (box->isTimeOut(delta_time)) {
                box = m_boxes.erase(box);
            }
            else {
                box++;
            }
        }
        for (std::list<DebugDrawCylinder>::iterator cylinder = m_cylinders.begin(); cylinder != m_cylinders.end();)
        {
            if (cylinder->isTimeOut(delta_time)) {
                cylinder = m_cylinders.erase(cylinder);
            }
            else {
                cylinder++;
            }
        }
        for (std::list<DebugDrawSphere>::iterator sphere = m_spheres.begin(); sphere != m_spheres.end();)
        {
            if (sphere->isTimeOut(delta_time)) {
                sphere = m_spheres.erase(sphere);
            }
            else {
                sphere++;
            }
        }
        for (std::list<DebugDrawCapsule>::iterator capsule = m_capsules.begin(); capsule != m_capsules.end();)
        {
            if (capsule->isTimeOut(delta_time)) {
                capsule = m_capsules.erase(capsule);
            }
            else {
                capsule++;
            }
        }
        for (std::list<DebugDrawText>::iterator text = m_texts.begin(); text != m_texts.end();)
        {
            if (text->isTimeOut(delta_time)) {
                text = m_texts.erase(text);
            }
            else {
                text++;
            }
        }
    }

    void DebugDrawGroup::mergeFrom(DebugDrawGroup* group)
    {
        std::lock_guard<std::mutex> guard(m_mutex);
        std::lock_guard<std::mutex> guard_2(group->m_mutex);
        m_points.insert(m_points.end(), group->m_points.begin(), group->m_points.end());
        m_lines.insert(m_lines.end(), group->m_lines.begin(), group->m_lines.end());
        m_triangles.insert(m_triangles.end(), group->m_triangles.begin(), group->m_triangles.end());
        m_quads.insert(m_quads.end(), group->m_quads.begin(), group->m_quads.end());
        m_boxes.insert(m_boxes.end(), group->m_boxes.begin(), group->m_boxes.end());
        m_cylinders.insert(m_cylinders.end(), group->m_cylinders.begin(), group->m_cylinders.end());
        m_spheres.insert(m_spheres.end(), group->m_spheres.begin(), group->m_spheres.end());
        m_capsules.insert(m_capsules.end(), group->m_capsules.begin(), group->m_capsules.end());
        m_texts.insert(m_texts.end(), group->m_texts.begin(), group->m_texts.end());
    }

    size_t DebugDrawGroup::getPointCount(bool no_depth_test) const
    {
        size_t count = 0;
        for (const DebugDrawPoint point : m_points)
        {
            if (point.m_no_depth_test == no_depth_test)count++;
        }
        return count;
    }

    size_t DebugDrawGroup::getLineCount(bool no_depth_test) const
    {
        size_t line_count = 0;
        for (const DebugDrawLine line : m_lines)
        {
            if (line.m_no_depth_test == no_depth_test)line_count++;
        }
        for (const DebugDrawTriangle triangle : m_triangles)
        {
            if (triangle.m_fill_mode == FillMode::_FillMode_wireframe && triangle.m_no_depth_test == no_depth_test)
            {
                line_count += 3;
            }
        }
        for (const DebugDrawQuad quad : m_quads)
        {
            if (quad.m_fill_mode == FillMode::_FillMode_wireframe && quad.m_no_depth_test == no_depth_test)
            {
                line_count += 4;
            }
        }
        for (const DebugDrawBox box : m_boxes)
        {
            if (box.m_no_depth_test == no_depth_test)line_count += 12;
        }
        return line_count;
    }

    size_t DebugDrawGroup::getTriangleCount(bool no_depth_test) const
    {
        size_t triangle_count = 0;
        for (const DebugDrawTriangle triangle : m_triangles)
        {
            if (triangle.m_fill_mode == FillMode::_FillMode_solid && triangle.m_no_depth_test == no_depth_test)
            {
                triangle_count++;
            }
        }
        return triangle_count;
    }

    size_t DebugDrawGroup::getUniformDynamicDataCount() const
    {
        return m_spheres.size() + m_cylinders.size() + m_capsules.size();
    }

    void DebugDrawGroup::writePointData(std::vector<DebugDrawVertex>& vertexs, bool no_depth_test)
    {
        size_t vertexs_count = getPointCount(no_depth_test);
        vertexs.resize(vertexs_count);

        size_t current_index = 0;
        for (DebugDrawPoint point : m_points)
        {
            if (point.m_no_depth_test == no_depth_test)vertexs[current_index++] = point.m_vertex;
        }
    }

    void DebugDrawGroup::writeLineData(std::vector<DebugDrawVertex> &vertexs, bool no_depth_test)
    {
        size_t vertexs_count = getLineCount(no_depth_test) * 2;
        vertexs.resize(vertexs_count);

        size_t current_index = 0;
        for (DebugDrawLine line : m_lines)
        {
            if (line.m_fill_mode == FillMode::_FillMode_wireframe && line.m_no_depth_test == no_depth_test)
            {
                vertexs[current_index++] = line.m_vertex[0];
                vertexs[current_index++] = line.m_vertex[1];
            }
        }
        for (DebugDrawTriangle triangle : m_triangles)
        {
            if (triangle.m_fill_mode == FillMode::_FillMode_wireframe && triangle.m_no_depth_test == no_depth_test)
            {
                std::vector<size_t> indies = { 0,1, 1,2, 2,0 };
                for (size_t i : indies)
                {
                    vertexs[current_index++] = triangle.m_vertex[i];
                }
            }
        }
        for (DebugDrawQuad quad : m_quads)
        {
            if (quad.m_fill_mode == FillMode::_FillMode_wireframe && quad.m_no_depth_test == no_depth_test)
            {
                std::vector<size_t> indies = { 0,1, 1,2, 2,3, 3,0 };
                for (size_t i : indies)
                {
                    vertexs[current_index++] = quad.m_vertex[i];
                }
            }
        }
        for (DebugDrawBox box : m_boxes)
        {
            if (box.m_no_depth_test == no_depth_test)
            {
                std::vector<DebugDrawVertex> verts_4d(8);
                float f[2] = { -1.0f,1.0f };
                for (size_t i = 0; i < 8; i++)
                {
                    Vector3 v(f[i & 1] * box.m_half_extents.x, f[(i >> 1) & 1] * box.m_half_extents.y, f[(i >> 2) & 1] * box.m_half_extents.z);
                    Vector3 uv, uuv;
                    Vector3 qvec(box.m_rotate.x, box.m_rotate.y, box.m_rotate.z);
                    uv = qvec.crossProduct(v);
                    uuv = qvec.crossProduct(uv);
                    uv *= (2.0f * box.m_rotate.w);
                    uuv *= 2.0f;
                    verts_4d[i].pos = v + uv + uuv + box.m_center_point;
                    verts_4d[i].color = box.m_color;
                }
                std::vector<size_t> indies = { 0,1, 1,3, 3,2, 2,0, 4,5, 5,7, 7,6, 6,4, 0,4, 1,5, 3,7, 2,6 };
                for (size_t i : indies)
                {
                    vertexs[current_index++] = verts_4d[i];
                }
            }
        }
    }

    void DebugDrawGroup::writeTriangleData(std::vector<DebugDrawVertex>& vertexs, bool no_depth_test)
    {
        size_t vertexs_count = getTriangleCount(no_depth_test) * 3;
        vertexs.resize(vertexs_count);

        size_t current_index = 0;
        for (DebugDrawTriangle triangle : m_triangles)
        {
            if (triangle.m_fill_mode == FillMode::_FillMode_solid && triangle.m_no_depth_test == no_depth_test)
            {
                vertexs[current_index++] = triangle.m_vertex[0];
                vertexs[current_index++] = triangle.m_vertex[1];
                vertexs[current_index++] = triangle.m_vertex[2];
            }
        }
    }

    void DebugDrawGroup::writeTextData(std::vector<DebugDrawVertex>& vertexs, DebugDrawFont* font, Matrix4x4 m_proj_view_matrix)
    {
        RHISwapChainDesc swapChainDesc = g_runtime_global_context.m_render_system->getRHI()->getSwapchainInfo();
        uint32_t screenWidth = swapChainDesc.viewport->width;
        uint32_t screenHeight = swapChainDesc.viewport->height;

        size_t vertexs_count = getTextCharacterCount() * 6;
        vertexs.resize(vertexs_count);

        size_t current_index = 0;
        for(DebugDrawText text : m_texts)
        {
            float absoluteW = text.m_size, absoluteH = text.m_size * 2;
            float w = absoluteW / (1.0f * screenWidth / 2.0f), h = absoluteH / (1.0f * screenHeight / 2.0f);
            Vector3 coordinate = text.m_coordinate;
            if (!text.m_is_screen_text)
            {
                Vector4 tempCoord(coordinate.x, coordinate.y, coordinate.z, 1.0f);
                tempCoord = m_proj_view_matrix * tempCoord;
                coordinate = Vector3(tempCoord.x / tempCoord.w, tempCoord.y / tempCoord.w, 0.0f);
            }
            float x = coordinate.x, y = coordinate.y;
            for (unsigned char character : text.m_content)
            {
                if (character == '\n')
                {
                    y += h;
                    x = coordinate.x;
                }
                else
                {

                    float x1, x2, y1, y2;
                    font->getCharacterTextureRect(character, x1, y1, x2, y2);

                    float cx1, cx2, cy1, cy2;
                    cx1 = 0 + x; cx2 = w + x;
                    cy1 = 0 + y; cy2 = h + y;

                    vertexs[current_index].pos = Vector3(cx1, cy1, 0.0f);
                    vertexs[current_index].color = text.m_color;
                    vertexs[current_index++].texcoord = Vector2(x1, y1);

                    vertexs[current_index].pos = Vector3(cx1, cy2, 0.0f);
                    vertexs[current_index].color = text.m_color;
                    vertexs[current_index++].texcoord = Vector2(x1, y2);

                    vertexs[current_index].pos = Vector3(cx2, cy2, 0.0f);
                    vertexs[current_index].color = text.m_color;
                    vertexs[current_index++].texcoord = Vector2(x2, y2);

                    vertexs[current_index].pos = Vector3(cx1, cy1, 0.0f);
                    vertexs[current_index].color = text.m_color;
                    vertexs[current_index++].texcoord = Vector2(x1, y1);

                    vertexs[current_index].pos = Vector3(cx2, cy2, 0.0f);
                    vertexs[current_index].color = text.m_color;
                    vertexs[current_index++].texcoord = Vector2(x2, y2);

                    vertexs[current_index].pos = Vector3(cx2, cy1, 0.0f);
                    vertexs[current_index].color = text.m_color;
                    vertexs[current_index++].texcoord = Vector2(x2, y1);

                    x += w;
                }
            }
        }
    }

    void DebugDrawGroup::writeUniformDynamicDataToCache(std::vector<std::pair<Matrix4x4, Vector4> >& datas)
    {
        // cache uniformDynamic data ,first has_depth_test ,second no_depth_test
        size_t data_count = getUniformDynamicDataCount() * 3;
        datas.resize(data_count);

        bool no_depth_tests[] = { false,true };
        for (int32_t i = 0; i < 2; i++)
        {
            bool no_depth_test = no_depth_tests[i];

            size_t current_index = 0;
            for (DebugDrawSphere sphere : m_spheres)
            {
                if (sphere.m_no_depth_test == no_depth_test)
                {
                    Matrix4x4 model = Matrix4x4::IDENTITY;

                    Matrix4x4 tmp = Matrix4x4::IDENTITY;
                    tmp.makeTrans(sphere.m_center);
                    model = model * tmp;
                    tmp = Matrix4x4::buildScaleMatrix(sphere.m_radius, sphere.m_radius, sphere.m_radius);
                    model = model * tmp;
                    datas[current_index++] = std::make_pair(model, sphere.m_color);
                }
            }
            for (DebugDrawCylinder cylinder : m_cylinders)
            {
                if (cylinder.m_no_depth_test == no_depth_test)
                {
                    Matrix4x4 model = Matrix4x4::IDENTITY;

                    //rolate
                    float w = cylinder.m_rotate.x;
                    float x = cylinder.m_rotate.y;
                    float y = cylinder.m_rotate.z;
                    float z = cylinder.m_rotate.w;
                    Matrix4x4 tmp = Matrix4x4::IDENTITY;
                    tmp.makeTrans(cylinder.m_center);
                    model = model * tmp;
                    
                    tmp = Matrix4x4::buildScaleMatrix(cylinder.m_radius, cylinder.m_radius, cylinder.m_height / 2.0f);
                    model = model * tmp;
            
                    Matrix4x4 ro = Matrix4x4::IDENTITY;
                    ro[0][0] = 1.0f - 2.0f * y * y - 2.0f * z * z; ro[0][1] = 2.0f * x * y + 2.0f * w * z;        ro[0][2] = 2.0f * x * z - 2.0f * w * y;
                    ro[1][0] = 2.0f * x * y - 2.0f * w * z;        ro[1][1] = 1.0f - 2.0f * x * x - 2.0f * z * z; ro[1][2] = 2.0f * y * z + 2.0f * w * x;
                    ro[2][0] = 2.0f * x * z + 2.0f * w * y;        ro[2][1] = 2.0f * y * z - 2.0f * w * x;        ro[2][2] = 1.0f - 2.0f * x * x - 2.0f * y * y;
                    model = model * ro;

                    datas[current_index++] = std::make_pair(model, cylinder.m_color);
                }
            }
            for (DebugDrawCapsule capsule : m_capsules)
            {
                if (capsule.m_no_depth_test == no_depth_test)
                {
                    Matrix4x4 model1 = Matrix4x4::IDENTITY;
                    Matrix4x4 model2 = Matrix4x4::IDENTITY;
                    Matrix4x4 model3 = Matrix4x4::IDENTITY;

                    Matrix4x4 tmp = Matrix4x4::IDENTITY;
                    tmp.makeTrans(capsule.m_center);
                    model1 = model1 * tmp;
                    model2 = model2 * tmp;
                    model3 = model3 * tmp;

                    tmp = Matrix4x4::buildScaleMatrix(capsule.m_scale.x, capsule.m_scale.y, capsule.m_scale.z);
                    model1 = model1 * tmp;
                    model2 = model2 * tmp;
                    model3 = model3 * tmp;

                    //rolate
                    float w = capsule.m_rotation.x;
                    float x = capsule.m_rotation.y;
                    float y = capsule.m_rotation.z;
                    float z = capsule.m_rotation.w;
                    Matrix4x4 ro = Matrix4x4::IDENTITY;
                    ro[0][0] = 1.0f - 2.0f * y * y - 2.0f * z * z; ro[0][1] = 2.0f * x * y + 2.0f * w * z;        ro[0][2] = 2.0f * x * z - 2.0f * w * y;
                    ro[1][0] = 2.0f * x * y - 2.0f * w * z;        ro[1][1] = 1.0f - 2.0f * x * x - 2.0f * z * z; ro[1][2] = 2.0f * y * z + 2.0f * w * x;
                    ro[2][0] = 2.0f * x * z + 2.0f * w * y;        ro[2][1] = 2.0f * y * z - 2.0f * w * x;        ro[2][2] = 1.0f - 2.0f * x * x - 2.0f * y * y;
                    model1 = model1 * ro;
                    model2 = model2 * ro;
                    model3 = model3 * ro;

                    tmp.makeTrans(Vector3(0.0f, 0.0f, capsule.m_height / 2.0f - capsule.m_radius));
                    model1 = model1 * tmp;

                    tmp = Matrix4x4::buildScaleMatrix(1.0f, 1.0f, capsule.m_height / (capsule.m_radius * 2.0f));
                    model2 = model2 * tmp;

                    tmp.makeTrans(Vector3(0.0f, 0.0f, -(capsule.m_height / 2.0f - capsule.m_radius)));
                    model3 = model3 * tmp;

                    tmp = Matrix4x4::buildScaleMatrix(capsule.m_radius, capsule.m_radius, capsule.m_radius);
                    model1 = model1 * tmp;
                    model2 = model2 * tmp;
                    model3 = model3 * tmp;

                    datas[current_index++] = std::make_pair(model1, capsule.m_color);
                    datas[current_index++] = std::make_pair(model2, capsule.m_color);
                    datas[current_index++] = std::make_pair(model3, capsule.m_color);
                }
            }
        }
    }

    size_t DebugDrawGroup::getSphereCount(bool no_depth_test) const
    {
        size_t count = 0;
        for (const DebugDrawSphere sphere : m_spheres)
        {
            if (sphere.m_no_depth_test == no_depth_test)count++;
        }
        return count; 
    }
    size_t DebugDrawGroup::getCylinderCount(bool no_depth_test) const
    {
        size_t count = 0;
        for (const DebugDrawCylinder cylinder : m_cylinders)
        {
            if (cylinder.m_no_depth_test == no_depth_test)count++;
        }
        return count;
    }
    size_t DebugDrawGroup::getCapsuleCount(bool no_depth_test) const
    {
        size_t count = 0;
        for (const DebugDrawCapsule capsule : m_capsules)
        {
            if (capsule.m_no_depth_test == no_depth_test)count++;
        }
        return count;
    }
    size_t DebugDrawGroup::getTextCharacterCount() const
    {
        size_t count = 0;
        for (const DebugDrawText text : m_texts)
        {
            for (unsigned char character : text.m_content)
            {
                if (character != '\n')count++;
            }
        }
        return count;
    }
}