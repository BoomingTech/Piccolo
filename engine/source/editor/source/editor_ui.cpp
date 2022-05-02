#include "editor/include/editor_ui.h"

#include "editor/include/editor.h"

#include "runtime/core/base/macro.h"
#include "runtime/core/meta/reflection/reflection.h"

#include "runtime/platform/path/path.h"

#include "runtime/resource/asset_manager/asset_manager.h"
#include "runtime/resource/config_manager/config_manager.h"

#include "runtime/engine.h"

#include "runtime/function/framework/component/mesh/mesh_component.h"
#include "runtime/function/framework/component/transform/transform_component.h"

#include "runtime/function/framework/level/level.h"
#include "runtime/function/framework/world/world_manager.h"
#include "runtime/function/input/input_system.h"
#include "runtime/function/render/include/render/render.h"
#include "runtime/function/scene/scene_manager.h"
#include "runtime/function/ui/ui_system.h"

#include <imgui.h>
#include <imgui_internal.h>

namespace Pilot
{
    std::vector<std::pair<std::string, bool>> g_editor_node_state_array;
    int                                       g_node_depth = -1;
    void                                      DrawVecControl(const std::string& label,
                                                             Pilot::Vector3&    values,
                                                             float              resetValue  = 0.0f,
                                                             float              columnWidth = 100.0f);
    void                                      DrawVecControl(const std::string& label,
                                                             Pilot::Quaternion& values,
                                                             float              resetValue  = 0.0f,
                                                             float              columnWidth = 100.0f);

    EditorUI::EditorUI(PilotEditor* editor) : m_editor(editor)
    {
        Path&       path_service            = Path::getInstance();
        const auto& asset_folder            = ConfigManager::getInstance().getAssetFolder();
        m_editor_ui_creator["TreeNodePush"] = [this](std::string name, void* value_ptr) -> void {
            static ImGuiTableFlags flags      = ImGuiTableFlags_Resizable | ImGuiTableFlags_NoSavedSettings;
            bool                   node_state = false;
            g_node_depth++;
            if (g_node_depth > 0)
            {
                if (g_editor_node_state_array[g_node_depth - 1].second)
                {
                    node_state = ImGui::TreeNodeEx(name.c_str(), ImGuiTreeNodeFlags_SpanFullWidth);
                }
                else
                {
                    g_editor_node_state_array.emplace_back(std::pair(name.c_str(), node_state));
                    return;
                }
            }
            else
            {
                node_state = ImGui::TreeNodeEx(name.c_str(), ImGuiTreeNodeFlags_SpanFullWidth);
            }
            g_editor_node_state_array.emplace_back(std::pair(name.c_str(), node_state));
        };
        m_editor_ui_creator["TreeNodePop"] = [this](std::string name, void* value_ptr) -> void {
            if (g_editor_node_state_array[g_node_depth].second)
            {
                ImGui::TreePop();
            }
            g_editor_node_state_array.pop_back();
            g_node_depth--;
        };
        m_editor_ui_creator["Transform"] = [this](std::string name, void* value_ptr) -> void {
            if (g_editor_node_state_array[g_node_depth].second)
            {
                Transform* trans_ptr = static_cast<Transform*>(value_ptr);

                Vector3 degrees_val;

                degrees_val.x = trans_ptr->m_rotation.getRoll(false).valueDegrees();
                degrees_val.y = trans_ptr->m_rotation.getPitch(false).valueDegrees();
                degrees_val.z = trans_ptr->m_rotation.getYaw(false).valueDegrees();

                DrawVecControl("Position", trans_ptr->m_position);
                DrawVecControl("Rotation", degrees_val);
                DrawVecControl("Scale", trans_ptr->m_scale);

                trans_ptr->m_rotation.w = Math::cos(Math::degreesToRadians(degrees_val.y / 2)) *
                                              Math::cos(Math::degreesToRadians(degrees_val.z / 2)) *
                                              Math::cos(Math::degreesToRadians(degrees_val.x / 2)) +
                                          Math::sin(Math::degreesToRadians(degrees_val.y / 2)) *
                                              Math::sin(Math::degreesToRadians(degrees_val.z / 2)) *
                                              Math::sin(Math::degreesToRadians(degrees_val.x / 2));
                trans_ptr->m_rotation.x = Math::sin(Math::degreesToRadians(degrees_val.y / 2)) *
                                              Math::cos(Math::degreesToRadians(degrees_val.z / 2)) *
                                              Math::cos(Math::degreesToRadians(degrees_val.x / 2)) -
                                          Math::cos(Math::degreesToRadians(degrees_val.y / 2)) *
                                              Math::sin(Math::degreesToRadians(degrees_val.z / 2)) *
                                              Math::sin(Math::degreesToRadians(degrees_val.x / 2));
                trans_ptr->m_rotation.y = Math::cos(Math::degreesToRadians(degrees_val.y / 2)) *
                                              Math::sin(Math::degreesToRadians(degrees_val.z / 2)) *
                                              Math::cos(Math::degreesToRadians(degrees_val.x / 2)) +
                                          Math::sin(Math::degreesToRadians(degrees_val.y / 2)) *
                                              Math::cos(Math::degreesToRadians(degrees_val.z / 2)) *
                                              Math::sin(Math::degreesToRadians(degrees_val.x / 2));
                trans_ptr->m_rotation.z = Math::cos(Math::degreesToRadians(degrees_val.y / 2)) *
                                              Math::cos(Math::degreesToRadians(degrees_val.z / 2)) *
                                              Math::sin(Math::degreesToRadians(degrees_val.x / 2)) -
                                          Math::sin(Math::degreesToRadians(degrees_val.y / 2)) *
                                              Math::sin(Math::degreesToRadians(degrees_val.z / 2)) *
                                              Math::cos(Math::degreesToRadians(degrees_val.x / 2));
                trans_ptr->m_rotation.normalise();

                drawSelectedEntityAxis();
            }
        };
        m_editor_ui_creator["int"] = [this](std::string name, void* value_ptr) -> void {
            if (g_node_depth == -1)
            {
                std::string label = "##" + name;
                ImGui::Text("%s", name.c_str());
                ImGui::SameLine();
                ImGui::InputInt(label.c_str(), static_cast<int*>(value_ptr));
            }
            else
            {
                if (g_editor_node_state_array[g_node_depth].second)
                {
                    std::string full_label = "##" + getLeafUINodeParentLabel() + name;
                    ImGui::Text("%s", (name + ":").c_str());
                    ImGui::InputInt(full_label.c_str(), static_cast<int*>(value_ptr));
                }
            }
        };
        m_editor_ui_creator["float"] = [this](std::string name, void* value_ptr) -> void {
            if (g_node_depth == -1)
            {
                std::string label = "##" + name;
                ImGui::Text("%s", name.c_str());
                ImGui::SameLine();
                ImGui::InputFloat(label.c_str(), static_cast<float*>(value_ptr));
            }
            else
            {
                if (g_editor_node_state_array[g_node_depth].second)
                {
                    std::string full_label = "##" + getLeafUINodeParentLabel() + name;
                    ImGui::Text("%s", (name + ":").c_str());
                    ImGui::InputFloat(full_label.c_str(), static_cast<float*>(value_ptr));
                }
            }
        };
        m_editor_ui_creator["Vector3"] = [this](std::string name, void* value_ptr) -> void {
            Vector3* vec_ptr = static_cast<Vector3*>(value_ptr);
            float    val[3]  = {vec_ptr->x, vec_ptr->y, vec_ptr->z};
            if (g_node_depth == -1)
            {
                std::string label = "##" + name;
                ImGui::Text("%s", name.c_str());
                ImGui::SameLine();
                ImGui::DragFloat3(label.c_str(), val);
            }
            else
            {
                if (g_editor_node_state_array[g_node_depth].second)
                {
                    std::string full_label = "##" + getLeafUINodeParentLabel() + name;
                    ImGui::Text("%s", (name + ":").c_str());
                    ImGui::DragFloat3(full_label.c_str(), val);
                }
            }
            vec_ptr->x = val[0];
            vec_ptr->y = val[1];
            vec_ptr->z = val[2];
        };
        m_editor_ui_creator["Quaternion"] = [this](std::string name, void* value_ptr) -> void {
            Quaternion* qua_ptr = static_cast<Quaternion*>(value_ptr);
            float       val[4]  = {qua_ptr->x, qua_ptr->y, qua_ptr->z, qua_ptr->w};
            if (g_node_depth == -1)
            {
                std::string label = "##" + name;
                ImGui::Text("%s", name.c_str());
                ImGui::SameLine();
                ImGui::DragFloat4(label.c_str(), val);
            }
            else
            {
                if (g_editor_node_state_array[g_node_depth].second)
                {
                    std::string full_label = "##" + getLeafUINodeParentLabel() + name;
                    ImGui::Text("%s", (name + ":").c_str());
                    ImGui::DragFloat4(full_label.c_str(), val);
                }
            }
            qua_ptr->x = val[0];
            qua_ptr->y = val[1];
            qua_ptr->z = val[2];
            qua_ptr->w = val[3];
        };
        m_editor_ui_creator["std::string"] = [this, &path_service, &asset_folder](std::string name,
                                                                                  void*       value_ptr) -> void {
            if (g_node_depth == -1)
            {
                std::string label = "##" + name;
                ImGui::Text("%s", name.c_str());
                ImGui::SameLine();
                ImGui::Text("%s", (*static_cast<std::string*>(value_ptr)).c_str());
            }
            else
            {
                if (g_editor_node_state_array[g_node_depth].second)
                {
                    std::string full_label = "##" + getLeafUINodeParentLabel() + name;
                    ImGui::Text("%s", (name + ":").c_str());
                    std::string value_str = *static_cast<std::string*>(value_ptr);
                    if (value_str.find_first_of('/') != std::string::npos)
                    {
                        std::filesystem::path value_path(value_str);
                        if (value_path.is_absolute())
                        {
                            value_path = path_service.getRelativePath(asset_folder, value_path);
                        }
                        value_str = value_path.generic_string();
                        if (value_str.size() >= 2 && value_str[0] == '.' && value_str[1] == '.')
                        {
                            value_str.clear();
                        }
                    }
                    ImGui::Text("%s", value_str.c_str());
                }
            }
        };
    }

    std::string EditorUI::getLeafUINodeParentLabel()
    {
        std::string parent_label;
        int         array_size = g_editor_node_state_array.size();
        for (int index = 0; index < array_size; index++)
        {
            parent_label += g_editor_node_state_array[index].first + "::";
        }
        return parent_label;
    }

    bool EditorUI::isCursorInRect(Vector2 pos, Vector2 size) const
    {
        return pos.x <= m_mouse_x && m_mouse_x <= pos.x + size.x && pos.y <= m_mouse_y && m_mouse_y <= pos.y + size.y;
    }

    GObject* EditorUI::getSelectedGObject() const
    {
        GObject* selected_object = nullptr;
        if (m_selected_gobject_id != PILOT_INVALID_GOBJECT_ID)
        {
            Level* level = WorldManager::getInstance().getCurrentActiveLevel();
            if (level != nullptr)
            {
                selected_object = level->getGObjectByID(m_selected_gobject_id);
            }
        }

        return selected_object;
    }

    void EditorUI::onGObjectSelected(size_t selected_gobject_id)
    {
        if (selected_gobject_id == m_selected_gobject_id)
            return;

        m_selected_gobject_id = selected_gobject_id;

        GObject* selected_gobject = getSelectedGObject();
        if (selected_gobject)
        {
            const TransformComponent* transform_component = selected_gobject->tryGetComponentConst(TransformComponent);
            m_selected_object_matrix                      = transform_component->getMatrix();
        }

        drawSelectedEntityAxis();

        if (m_selected_gobject_id != PILOT_INVALID_GOBJECT_ID)
        {
            LOG_INFO("select game object " + std::to_string(m_selected_gobject_id));
        }
        else
        {
            LOG_INFO("no game object selected");
        }
    }

    void EditorUI::onTick(UIState* uistate)
    {
        showEditorUI();
        processEditorCommand();
    }

    void EditorUI::showEditorUI()
    {

        bool editor_menu_window_open       = true;
        bool asset_window_open             = true;
        bool game_engine_window_open       = true;
        bool file_content_window_open      = true;
        bool detail_window_open            = true;
        bool scene_lights_window_open      = true;
        bool scene_lights_data_window_open = true;

        showEditorMenu(&editor_menu_window_open);
        showEditorWorldObjectsWindow(&asset_window_open);
        showEditorGameWindow(&game_engine_window_open);
        showEditorFileContentWindow(&file_content_window_open);
        showEditorDetailWindow(&detail_window_open);
    }

    void EditorUI::showEditorMenu(bool* p_open)
    {
        ImGuiDockNodeFlags dock_flags   = ImGuiDockNodeFlags_DockSpace;
        ImGuiWindowFlags   window_flags = ImGuiWindowFlags_MenuBar | ImGuiWindowFlags_NoTitleBar |
                                        ImGuiWindowFlags_NoCollapse | ImGuiWindowFlags_NoResize |
                                        ImGuiWindowFlags_NoMove | ImGuiWindowFlags_NoBackground |
                                        ImGuiConfigFlags_NoMouseCursorChange | ImGuiWindowFlags_NoBringToFrontOnFocus;

        const ImGuiViewport* main_viewport = ImGui::GetMainViewport();
        ImGui::SetNextWindowPos(main_viewport->WorkPos, ImGuiCond_Always);
        ImGui::SetNextWindowSize(ImVec2(m_io->m_width, m_io->m_height), ImGuiCond_Always);
        ImGui::SetNextWindowViewport(main_viewport->ID);

        ImGui::Begin("Editor menu", p_open, window_flags);

        ImGuiID main_docking_id = ImGui::GetID("Main Docking");
        if (ImGui::DockBuilderGetNode(main_docking_id) == nullptr)
        {
            ImGui::DockBuilderRemoveNode(main_docking_id);

            ImGui::DockBuilderAddNode(main_docking_id, dock_flags);
            ImGui::DockBuilderSetNodePos(main_docking_id,
                                         ImVec2(main_viewport->WorkPos.x, main_viewport->WorkPos.y + 18.0f));
            ImGui::DockBuilderSetNodeSize(main_docking_id, ImVec2(m_io->m_width, m_io->m_height - 18.0f));

            ImGuiID center = main_docking_id;
            ImGuiID left;
            ImGuiID right = ImGui::DockBuilderSplitNode(center, ImGuiDir_Right, 0.25f, nullptr, &left);

            ImGuiID left_other;
            ImGuiID left_file_content = ImGui::DockBuilderSplitNode(left, ImGuiDir_Down, 0.30f, nullptr, &left_other);

            ImGuiID left_game_engine;
            ImGuiID left_asset =
                ImGui::DockBuilderSplitNode(left_other, ImGuiDir_Left, 0.30f, nullptr, &left_game_engine);

            ImGui::DockBuilderDockWindow("World Objects", left_asset);
            ImGui::DockBuilderDockWindow("Components Details", right);
            ImGui::DockBuilderDockWindow("File Content", left_file_content);
            ImGui::DockBuilderDockWindow("Game Engine", left_game_engine);

            ImGui::DockBuilderFinish(main_docking_id);
        }

        ImGui::DockSpace(main_docking_id);

        if (ImGui::BeginMenuBar())
        {
            if (ImGui::BeginMenu("Menu"))
            {
                if (ImGui::MenuItem("Reload Current Level"))
                {
                    WorldManager::getInstance().reloadCurrentLevel();
                    onGObjectSelected(PILOT_INVALID_GOBJECT_ID);
                }
                if (ImGui::MenuItem("Save Current Level"))
                {
                    WorldManager::getInstance().saveCurrentLevel();
                }
                if (ImGui::MenuItem("Exit"))
                {
                    exit(0);
                }
                ImGui::EndMenu();
            }
            ImGui::EndMenuBar();
        }

        ImGui::End();
    }

    void EditorUI::showEditorWorldObjectsWindow(bool* p_open)
    {
        ImGuiWindowFlags window_flags = ImGuiWindowFlags_None;

        const ImGuiViewport* main_viewport = ImGui::GetMainViewport();

        if (!ImGui::Begin("World Objects", p_open, window_flags))
        {
            ImGui::End();
            return;
        }

        const Level* current_active_level = WorldManager::getInstance().getCurrentActiveLevel();
        if (current_active_level == nullptr)
            return;

        const auto& all_gobjects = current_active_level->getAllGObjects();
        for (auto& id_object_pair : all_gobjects)
        {
            const size_t      object_id = id_object_pair.first;
            GObject*          object    = id_object_pair.second;
            const std::string name      = object->getName();
            if (name.size() > 0)
            {
                if (ImGui::Selectable(name.c_str(), m_selected_gobject_id == object_id))
                {
                    if (m_selected_gobject_id != object_id)
                    {
                        onGObjectSelected(object_id);
                    }
                    else
                    {
                        onGObjectSelected(PILOT_INVALID_GOBJECT_ID);
                    }
                    break;
                }
            }
        }
        ImGui::End();
    }

    void EditorUI::createComponentUI(Reflection::ReflectionInstance& instance)
    {
        Reflection::ReflectionInstance* reflection_instance;
        int count = instance.m_meta.getBaseClassReflectionInstanceList(reflection_instance, instance.m_instance);
        for (int index = 0; index < count; index++)
        {
            createComponentUI(reflection_instance[index]);
        }
        createLeafNodeUI(instance);

        if (count > 0)
            delete[] reflection_instance;
    }

    void EditorUI::createLeafNodeUI(Reflection::ReflectionInstance& instance)
    {
        Reflection::FieldAccessor* fields;
        int                        fields_count = instance.m_meta.getFieldsList(fields);

        for (size_t index = 0; index < fields_count; index++)
        {
            auto fields_count = fields[index];
            if (fields_count.isArrayType())
            {

                Reflection::ArrayAccessor array_accessor;
                if (Reflection::TypeMeta::newArrayAccessorFromName(fields_count.getFieldTypeName(), array_accessor))
                {
                    void* field_instance = fields_count.get(instance.m_instance);
                    int   array_count    = array_accessor.getSize(field_instance);
                    m_editor_ui_creator["TreeNodePush"](
                        std::string(fields_count.getFieldName()) + "[" + std::to_string(array_count) + "]", nullptr);
                    auto item_type_meta_item =
                        Reflection::TypeMeta::newMetaFromName(array_accessor.getElementTypeName());
                    auto item_ui_creator_iterator = m_editor_ui_creator.find(item_type_meta_item.getTypeName());
                    for (int index = 0; index < array_count; index++)
                    {
                        if (item_ui_creator_iterator == m_editor_ui_creator.end())
                        {
                            m_editor_ui_creator["TreeNodePush"]("[" + std::to_string(index) + "]", nullptr);
                            auto object_instance = Reflection::ReflectionInstance(
                                Pilot::Reflection::TypeMeta::newMetaFromName(item_type_meta_item.getTypeName().c_str()),
                                array_accessor.get(index, field_instance));
                            createComponentUI(object_instance);
                            m_editor_ui_creator["TreeNodePop"]("[" + std::to_string(index) + "]", nullptr);
                        }
                        else
                        {
                            if (item_ui_creator_iterator == m_editor_ui_creator.end())
                            {
                                continue;
                            }
                            m_editor_ui_creator[item_type_meta_item.getTypeName()](
                                "[" + std::to_string(index) + "]", array_accessor.get(index, field_instance));
                        }
                    }
                    m_editor_ui_creator["TreeNodePop"](fields_count.getFieldName(), nullptr);
                }
            }
            auto ui_creator_iterator = m_editor_ui_creator.find(fields_count.getFieldTypeName());
            if (ui_creator_iterator == m_editor_ui_creator.end())
            {
                Reflection::TypeMeta field_meta =
                    Reflection::TypeMeta::newMetaFromName(fields_count.getFieldTypeName());
                if (fields_count.getTypeMeta(field_meta))
                {
                    auto child_instance =
                        Reflection::ReflectionInstance(field_meta, fields_count.get(instance.m_instance));
                    m_editor_ui_creator["TreeNodePush"](field_meta.getTypeName(), nullptr);
                    createLeafNodeUI(child_instance);
                    m_editor_ui_creator["TreeNodePop"](field_meta.getTypeName(), nullptr);
                }
                else
                {
                    if (ui_creator_iterator == m_editor_ui_creator.end())
                    {
                        continue;
                    }
                    m_editor_ui_creator[fields_count.getFieldTypeName()](fields_count.getFieldName(),
                                                                         fields_count.get(instance.m_instance));
                }
            }
            else
            {
                m_editor_ui_creator[fields_count.getFieldTypeName()](fields_count.getFieldName(),
                                                                     fields_count.get(instance.m_instance));
            }
        }
        delete[] fields;
    }

    void EditorUI::showEditorDetailWindow(bool* p_open)
    {
        ImGuiWindowFlags window_flags = ImGuiWindowFlags_None;

        const ImGuiViewport* main_viewport = ImGui::GetMainViewport();

        if (!ImGui::Begin("Components Details", p_open, window_flags))
        {
            ImGui::End();
            return;
        }

        GObject* selected_object = getSelectedGObject();
        if (selected_object == nullptr)
        {
            ImGui::End();
            return;
        }

        const std::string& name = selected_object->getName();
        static char        cname[128];
        memset(cname, 0, 128);
        memcpy(cname, name.c_str(), name.size());

        ImGui::Text("Name");
        ImGui::SameLine();
        ImGui::InputText("##Name", cname, IM_ARRAYSIZE(cname), ImGuiInputTextFlags_ReadOnly);

        static ImGuiTableFlags flags                      = ImGuiTableFlags_Resizable | ImGuiTableFlags_NoSavedSettings;
        auto&&                 selected_object_components = selected_object->getComponents();
        for (auto component_ptr : selected_object_components)
        {
            m_editor_ui_creator["TreeNodePush"](("<" + component_ptr.getTypeName() + ">").c_str(), nullptr);
            auto object_instance = Reflection::ReflectionInstance(
                Pilot::Reflection::TypeMeta::newMetaFromName(component_ptr.getTypeName().c_str()),
                component_ptr.operator->());
            createComponentUI(object_instance);
            m_editor_ui_creator["TreeNodePop"](("<" + component_ptr.getTypeName() + ">").c_str(), nullptr);
        }
        ImGui::End();
    }

    void EditorUI::showEditorFileContentWindow(bool* p_open)
    {
        ImGuiWindowFlags window_flags = ImGuiWindowFlags_None;

        const ImGuiViewport* main_viewport = ImGui::GetMainViewport();

        if (!ImGui::Begin("File Content", p_open, window_flags))
        {
            ImGui::End();
            return;
        }

        static ImGuiTableFlags flags = ImGuiTableFlags_BordersV | ImGuiTableFlags_BordersOuterH |
                                       ImGuiTableFlags_Resizable | ImGuiTableFlags_RowBg |
                                       ImGuiTableFlags_NoBordersInBody;

        if (ImGui::BeginTable("File Content", 2, flags))
        {
            ImGui::TableSetupColumn("Name", ImGuiTableColumnFlags_NoHide);
            ImGui::TableSetupColumn("Type", ImGuiTableColumnFlags_WidthFixed);
            ImGui::TableHeadersRow();

            auto current_time = std::chrono::steady_clock::now();
            if (current_time - m_last_file_tree_update > std::chrono::seconds(1))
            {
                m_editor_file_service.buildEngineFileTree();
                m_last_file_tree_update = current_time;
            }
            m_last_file_tree_update = current_time;

            EditorFileNode* editor_root_node = m_editor_file_service.getEditorRootNode();
            buildEditorFileAssstsUITree(editor_root_node);
            ImGui::EndTable();
        }

        // file image list

        ImGui::End();
    }

    void EditorUI::showEditorGameWindow(bool* p_open)
    {
        ImGuiIO&         io           = ImGui::GetIO();
        ImGuiWindowFlags window_flags = ImGuiWindowFlags_NoBackground | ImGuiWindowFlags_MenuBar;

        const ImGuiViewport* main_viewport = ImGui::GetMainViewport();

        if (!ImGui::Begin("Game Engine", p_open, window_flags))
        {
            ImGui::End();
            return;
        }

        static bool trans_button_ckecked  = false;
        static bool rotate_button_ckecked = false;
        static bool scale_button_ckecked  = false;

        switch (m_axis_mode)
        {
            case EditorAxisMode::TranslateMode:
                trans_button_ckecked  = true;
                rotate_button_ckecked = false;
                scale_button_ckecked  = false;
                break;
            case EditorAxisMode::RotateMode:
                trans_button_ckecked  = false;
                rotate_button_ckecked = true;
                scale_button_ckecked  = false;
                break;
            case EditorAxisMode::ScaleMode:
                trans_button_ckecked  = false;
                rotate_button_ckecked = false;
                scale_button_ckecked  = true;
                break;
            default:
                break;
        }

        if (ImGui::BeginMenuBar())
        {
            ImGui::Indent(10.f);
            drawAxisToggleButton("Trans", trans_button_ckecked, EditorAxisMode::TranslateMode);
            ImGui::Unindent();

            ImGui::SameLine();

            drawAxisToggleButton("Rotate", rotate_button_ckecked, EditorAxisMode::RotateMode);

            ImGui::SameLine();

            drawAxisToggleButton("Scale", scale_button_ckecked, EditorAxisMode::ScaleMode);

            ImGui::SameLine();

            float indent_val = 0.0f;
            indent_val       = m_engine_window_size.x - 100.0f * getIndentScale();

            ImGui::Indent(indent_val);
            if (m_is_editor_mode)
            {
                ImGui::PushID("Editor Mode");
                ImGui::Button("Editor Mode");
                if (ImGui::IsItemClicked(ImGuiMouseButton_Left))
                {
                    m_is_editor_mode = !m_is_editor_mode;
                    drawSelectedEntityAxis();
                    g_is_editor_mode = false;
                    m_io->setFocusMode(true);
                }
                ImGui::PopID();
            }
            else
            {
                ImGui::Button("Game Mode");
                if (ImGui::IsItemClicked(ImGuiMouseButton_Left))
                {
                    m_is_editor_mode = !m_is_editor_mode;
                    drawSelectedEntityAxis();
                    g_is_editor_mode = true;
                    SceneManager::getInstance().setMainViewMatrix(m_tmp_uistate->m_editor_camera->getViewMatrix());
                }
            }
            m_io->setEditorMode(m_is_editor_mode);
            ImGui::Unindent();
            ImGui::EndMenuBar();
        }

        if (!m_is_editor_mode)
        {
            ImGui::TextColored(ImVec4(1.0f, 0.0f, 0.0f, 1.0f), "Press Left Alt key to display the mouse cursor!");
        }
        else
        {
            ImGui::TextColored(
                ImVec4(0.0f, 1.0f, 0.0f, 1.0f), "Current editor camera move speed: [%f]", m_camera_speed);
        }

        auto menu_bar_rect = ImGui::GetCurrentWindow()->MenuBarRect();

        Vector2 new_window_pos  = {0.0f, 0.0f};
        Vector2 new_window_size = {0.0f, 0.0f};
        new_window_pos.x        = ImGui::GetWindowPos().x;
        new_window_pos.y        = ImGui::GetWindowPos().y + menu_bar_rect.Min.y;
        new_window_size.x       = ImGui::GetWindowSize().x;
        new_window_size.y       = ImGui::GetWindowSize().y - menu_bar_rect.Min.y;

        // if (new_window_pos != m_engine_window_pos || new_window_size != m_engine_window_size)
        {
#if defined(__MACH__)
            // The dpi_scale is not reactive to DPI changes or monitor switching, it might be a bug from ImGui.
            // Return value from ImGui::GetMainViewport()->DpiScal is always the same as first frame.
            // glfwGetMonitorContentScale and glfwSetWindowContentScaleCallback are more adaptive.
            float dpi_scale = main_viewport->DpiScale;
            m_editor->onWindowChanged(new_window_pos.x * dpi_scale,
                                      new_window_pos.y * dpi_scale,
                                      new_window_size.x * dpi_scale,
                                      new_window_size.y * dpi_scale);
#else
            m_editor->onWindowChanged(new_window_pos.x, new_window_pos.y, new_window_size.x, new_window_size.y);
#endif

            m_engine_window_pos  = new_window_pos;
            m_engine_window_size = new_window_size;
            SceneManager::getInstance().setWindowSize(m_engine_window_size);
        }

        ImGui::End();
    }

    void EditorUI::drawAxisToggleButton(const char* string_id, bool check_state, EditorAxisMode axis_mode)
    {
        if (check_state)
        {
            ImGui::PushID(string_id);
            ImVec4 check_button_color = ImVec4(93.0f / 255.0f, 10.0f / 255.0f, 66.0f / 255.0f, 1.00f);
            ImGui::PushStyleColor(ImGuiCol_Button,
                                  ImVec4(check_button_color.x, check_button_color.y, check_button_color.z, 0.40f));
            ImGui::PushStyleColor(ImGuiCol_ButtonHovered, check_button_color);
            ImGui::PushStyleColor(ImGuiCol_ButtonActive, check_button_color);
            ImGui::Button(string_id);
            ImGui::PopStyleColor(3);
            ImGui::PopID();
        }
        else
        {
            if (ImGui::Button(string_id))
            {
                check_state = true;
                m_axis_mode = axis_mode;
                drawSelectedEntityAxis();
            }
        }
    }

    void EditorUI::buildEditorFileAssstsUITree(EditorFileNode* node)
    {
        ImGui::TableNextRow();
        ImGui::TableNextColumn();
        const bool is_folder = (node->m_child_nodes.size() > 0);
        if (is_folder)
        {
            bool open = ImGui::TreeNodeEx(node->m_file_name.c_str(), ImGuiTreeNodeFlags_SpanFullWidth);
            ImGui::TableNextColumn();
            ImGui::SetNextItemWidth(100.0f);
            ImGui::TextUnformatted(node->m_file_type.c_str());
            if (open)
            {
                for (int child_n = 0; child_n < node->m_child_nodes.size(); child_n++)
                    buildEditorFileAssstsUITree(node->m_child_nodes[child_n].get());
                ImGui::TreePop();
            }
        }
        else
        {
            ImGui::TreeNodeEx(node->m_file_name.c_str(),
                              ImGuiTreeNodeFlags_Leaf | ImGuiTreeNodeFlags_NoTreePushOnOpen |
                                  ImGuiTreeNodeFlags_SpanFullWidth);
            if (ImGui::IsItemClicked() && !ImGui::IsItemToggledOpen())
            {
                onFileContentItemClicked(node);
            }
            ImGui::TableNextColumn();
            ImGui::SetNextItemWidth(100.0f);
            ImGui::TextUnformatted(node->m_file_type.c_str());
        }
    }

    void EditorUI::onFileContentItemClicked(EditorFileNode* node)
    {
        if (node->m_file_type != "object")
            return;

        Level* level = WorldManager::getInstance().getCurrentActiveLevel();
        if (level == nullptr)
            return;

        const unsigned int new_object_index = ++m_new_object_index_map[node->m_file_name];

        ObjectInstanceRes new_object_instance_res;
        new_object_instance_res.m_name =
            "New_" + Path::getInstance().getFilePureName(node->m_file_name) + "_" + std::to_string(new_object_index);
        new_object_instance_res.m_definition =
            AssetManager::getInstance().getFullPath(node->m_file_path).generic_string();

        size_t new_gobject_id = level->createObject(new_object_instance_res);
        if (new_gobject_id != PILOT_INVALID_GOBJECT_ID)
        {
            onGObjectSelected(new_gobject_id);
        }
    }

    void EditorUI::drawSelectedEntityAxis()
    {
        GObject* selected_object = getSelectedGObject();

        if (m_is_editor_mode && selected_object != nullptr)
        {
            const TransformComponent* transform_component = selected_object->tryGetComponentConst(TransformComponent);
            std::vector<RenderMesh>   axis_meshs;

            Vector3    scale;
            Quaternion rotation;
            Vector3    translation;
            transform_component->getMatrix().decomposition(translation, scale, rotation);
            Matrix4x4 translation_matrix = Matrix4x4::getTrans(translation);
            Matrix4x4 scale_matrix       = Matrix4x4::buildScaleMatrix(1.0f, 1.0f, 1.0f);
            Matrix4x4 axis_model_matrix  = translation_matrix * scale_matrix;
            if (m_axis_mode == EditorAxisMode::TranslateMode)
            {
                m_translation_axis.m_model_matrix = axis_model_matrix;
                axis_meshs.push_back(m_translation_axis);
                SceneManager::getInstance().setAxisMesh(axis_meshs);
            }
            else if (m_axis_mode == EditorAxisMode::RotateMode)
            {
                m_rotation_axis.m_model_matrix = axis_model_matrix;
                axis_meshs.push_back(m_rotation_axis);
                SceneManager::getInstance().setAxisMesh(axis_meshs);
            }
            else if (m_axis_mode == EditorAxisMode::ScaleMode)
            {
                axis_model_matrix           = axis_model_matrix * Matrix4x4(rotation);
                m_scale_aixs.m_model_matrix = axis_model_matrix;
                axis_meshs.push_back(m_scale_aixs);
                SceneManager::getInstance().setAxisMesh(axis_meshs);
            }
        }
        else
        {
            std::vector<RenderMesh> axis_meshs;
            SceneManager::getInstance().setAxisMesh(axis_meshs);
        }
    }

    void EditorUI::updateCursorOnAxis(Vector2 cursor_uv)
    {
        if (m_tmp_uistate && m_tmp_uistate->m_editor_camera)
        {
            Vector2 window_size(m_engine_window_size.x, m_engine_window_size.y);
            m_cursor_on_axis = m_editor->onUpdateCursorOnAxis(static_cast<int>(m_axis_mode), cursor_uv, window_size);
        }
    }

    void EditorUI::onReset()
    {
        // to do
    }

    void EditorUI::registerInput()
    {
        m_io->registerOnResetFunc(std::bind(&EditorUI::onReset, this));
        m_io->registerOnCursorPosFunc(
            std::bind(&EditorUI::onCursorPos, this, std::placeholders::_1, std::placeholders::_2));
        m_io->registerOnCursorEnterFunc(std::bind(&EditorUI::onCursorEnter, this, std::placeholders::_1));
        m_io->registerOnScrollFunc(std::bind(&EditorUI::onScroll, this, std::placeholders::_1, std::placeholders::_2));
        m_io->registerOnMouseButtonFunc(
            std::bind(&EditorUI::onMouseButtonClicked, this, std::placeholders::_1, std::placeholders::_2));
        m_io->registerOnWindowCloseFunc(std::bind(&EditorUI::onWindowClosed, this));
        return;
    }

    void EditorUI::processEditorCommand()
    {
        float      camera_speed  = m_camera_speed;
        Quaternion camera_rotate = m_tmp_uistate->m_editor_camera->rotation().inverse();
        Vector3    camera_relative_pos(0, 0, 0);

        unsigned int command = InputSystem::getInstance().getEditorCommand();
        if ((unsigned int)EditorCommand::camera_foward & command)
        {
            camera_relative_pos += camera_rotate * Vector3 {0, camera_speed, 0};
        }
        if ((unsigned int)EditorCommand::camera_back & command)
        {
            camera_relative_pos += camera_rotate * Vector3 {0, -camera_speed, 0};
        }
        if ((unsigned int)EditorCommand::camera_left & command)
        {
            camera_relative_pos += camera_rotate * Vector3 {-camera_speed, 0, 0};
        }
        if ((unsigned int)EditorCommand::camera_right & command)
        {
            camera_relative_pos += camera_rotate * Vector3 {camera_speed, 0, 0};
        }
        if ((unsigned int)EditorCommand::camera_up & command)
        {
            camera_relative_pos += Vector3 {0, 0, camera_speed};
        }
        if ((unsigned int)EditorCommand::camera_down & command)
        {
            camera_relative_pos += Vector3 {0, 0, -camera_speed};
        }
        if ((unsigned int)EditorCommand::delete_object & command)
        {
            onDeleteSelectedGObject();
        }

        m_tmp_uistate->m_editor_camera->move(camera_relative_pos);
    }

    void EditorUI::onDeleteSelectedGObject()
    {
        // delete selected entity
        GObject* selected_object = getSelectedGObject();
        if (selected_object != nullptr)
        {
            Level* current_active_level = WorldManager::getInstance().getCurrentActiveLevel();
            if (current_active_level == nullptr)
                return;

            current_active_level->deleteGObjectByID(m_selected_gobject_id);
        }
        onGObjectSelected(PILOT_INVALID_GOBJECT_ID);
    }

    void EditorUI::onCursorPos(double xpos, double ypos)
    {
        if (!m_is_editor_mode)
            return;

        float angularVelocity =
            180.0f / Math::max(m_engine_window_size.x, m_engine_window_size.y); // 180 degrees while moving full screen
        if (m_mouse_x >= 0.0f && m_mouse_y >= 0.0f)
        {
            if (m_io->isMouseButtonDown(GLFW_MOUSE_BUTTON_RIGHT))
            {
                glfwSetInputMode(m_io->m_window, GLFW_CURSOR, GLFW_CURSOR_DISABLED);
                m_tmp_uistate->m_editor_camera->rotate(Vector2(ypos - m_mouse_y, xpos - m_mouse_x) * angularVelocity);
            }
            else if (m_io->isMouseButtonDown(GLFW_MOUSE_BUTTON_LEFT))
            {
                moveEntity(xpos, ypos, m_mouse_x, m_mouse_y, m_selected_object_matrix);
                glfwSetInputMode(m_io->m_window, GLFW_CURSOR, GLFW_CURSOR_NORMAL);
            }
            else
            {
                glfwSetInputMode(m_io->m_window, GLFW_CURSOR, GLFW_CURSOR_NORMAL);

                if (isCursorInRect(m_engine_window_pos, m_engine_window_size))
                {
                    Vector2 cursor_uv = Vector2((m_mouse_x - m_engine_window_pos.x) / m_engine_window_size.x,
                                                (m_mouse_y - m_engine_window_pos.y) / m_engine_window_size.y);
                    updateCursorOnAxis(cursor_uv);
                }
            }
        }
        m_mouse_x = xpos;
        m_mouse_y = ypos;
    }

    void EditorUI::onCursorEnter(int entered)
    {
        if (!entered) // lost focus
        {
            m_mouse_x = m_mouse_y = -1.0f;
        }
    }

    void EditorUI::onScroll(double xoffset, double yoffset)
    {
        if (!m_is_editor_mode)
        {
            return;
        }

        if (isCursorInRect(m_engine_window_pos, m_engine_window_size))
        {
            if (m_io->isMouseButtonDown(GLFW_MOUSE_BUTTON_RIGHT))
            {
                if (yoffset > 0)
                {
                    m_camera_speed *= 1.2f;
                }
                else
                {
                    m_camera_speed *= 0.8f;
                }
            }
            else
            {
                m_tmp_uistate->m_editor_camera->zoom((float)yoffset *
                                                     2.0f); // wheel scrolled up = zoom in by 2 extra degrees
            }
        }
    }

    void EditorUI::onMouseButtonClicked(int key, int action)
    {
        if (!m_is_editor_mode)
            return;
        if (m_cursor_on_axis != 3)
            return;

        const Level* current_active_level = WorldManager::getInstance().getCurrentActiveLevel();
        if (current_active_level == nullptr)
            return;

        if (isCursorInRect(m_engine_window_pos, m_engine_window_size))
        {
            if (key == GLFW_MOUSE_BUTTON_LEFT)
            {
                Vector2 picked_uv((m_mouse_x - m_engine_window_pos.x) / m_engine_window_size.x,
                                  (m_mouse_y - m_engine_window_pos.y) / m_engine_window_size.y);
                size_t  select_mesh_id = m_editor->getGuidOfPickedMesh(picked_uv);

                size_t gobject_id = SceneManager::getInstance().getGObjectIDByMeshID(select_mesh_id);
                onGObjectSelected(gobject_id);
            }
        }
    }
    void EditorUI::onWindowClosed() { PilotEngine::getInstance().shutdownEngine(); }

    void EditorUI::moveEntity(float     new_mouse_pos_x,
                              float     new_mouse_pos_y,
                              float     last_mouse_pos_x,
                              float     last_mouse_pos_y,
                              Matrix4x4 model_matrix)
    {
        GObject* selected_object = getSelectedGObject();
        if (selected_object == nullptr)
            return;

        float angularVelocity =
            18.0f / Math::max(m_engine_window_size.x, m_engine_window_size.y); // 18 degrees while moving full screen
        Vector2 delta_mouse_move_uv = {(new_mouse_pos_x - last_mouse_pos_x), (new_mouse_pos_y - last_mouse_pos_y)};

        Vector3    model_scale;
        Quaternion model_rotation;
        Vector3    model_translation;
        model_matrix.decomposition(model_translation, model_scale, model_rotation);

        Matrix4x4 axis_model_matrix = Matrix4x4::IDENTITY;
        axis_model_matrix.setTrans(model_translation);

        Matrix4x4 view_matrix = m_tmp_uistate->m_editor_camera->getLookAtMatrix();
        Matrix4x4 proj_matrix = m_tmp_uistate->m_editor_camera->getPersProjMatrix();

        Vector4 model_world_position_4(model_translation, 1.f);

        Vector4 model_origin_clip_position = proj_matrix * view_matrix * model_world_position_4;
        model_origin_clip_position /= model_origin_clip_position.w;
        Vector2 model_origin_clip_uv =
            Vector2((model_origin_clip_position.x + 1) / 2.0f, (model_origin_clip_position.y + 1) / 2.0f);

        Vector4 axis_x_local_position_4(1, 0, 0, 1);
        if (m_axis_mode == EditorAxisMode::ScaleMode)
        {
            axis_x_local_position_4 = Matrix4x4(model_rotation) * axis_x_local_position_4;
        }
        Vector4 axis_x_world_position_4 = axis_model_matrix * axis_x_local_position_4;
        axis_x_world_position_4.w       = 1.0f;
        Vector4 axis_x_clip_position    = proj_matrix * view_matrix * axis_x_world_position_4;
        axis_x_clip_position /= axis_x_clip_position.w;
        Vector2 axis_x_clip_uv((axis_x_clip_position.x + 1) / 2.0f, (axis_x_clip_position.y + 1) / 2.0f);
        Vector2 axis_x_direction_uv = axis_x_clip_uv - model_origin_clip_uv;
        axis_x_direction_uv.normalise();

        Vector4 axis_y_local_position_4(0, 1, 0, 1);
        if (m_axis_mode == EditorAxisMode::ScaleMode)
        {
            axis_y_local_position_4 = Matrix4x4(model_rotation) * axis_y_local_position_4;
        }
        Vector4 axis_y_world_position_4 = axis_model_matrix * axis_y_local_position_4;
        axis_y_world_position_4.w       = 1.0f;
        Vector4 axis_y_clip_position    = proj_matrix * view_matrix * axis_y_world_position_4;
        axis_y_clip_position /= axis_y_clip_position.w;
        Vector2 axis_y_clip_uv((axis_y_clip_position.x + 1) / 2.0f, (axis_y_clip_position.y + 1) / 2.0f);
        Vector2 axis_y_direction_uv = axis_y_clip_uv - model_origin_clip_uv;
        axis_y_direction_uv.normalise();

        Vector4 axis_z_local_position_4(0, 0, 1, 1);
        if (m_axis_mode == EditorAxisMode::ScaleMode)
        {
            axis_z_local_position_4 = Matrix4x4(model_rotation) * axis_z_local_position_4;
        }
        Vector4 axis_z_world_position_4 = axis_model_matrix * axis_z_local_position_4;
        axis_z_world_position_4.w       = 1.0f;
        Vector4 axis_z_clip_position    = proj_matrix * view_matrix * axis_z_world_position_4;
        axis_z_clip_position /= axis_z_clip_position.w;
        Vector2 axis_z_clip_uv((axis_z_clip_position.x + 1) / 2.0f, (axis_z_clip_position.y + 1) / 2.0f);
        Vector2 axis_z_direction_uv = axis_z_clip_uv - model_origin_clip_uv;
        axis_z_direction_uv.normalise();

        TransformComponent* transform_component = selected_object->tryGetComponent(TransformComponent);

        Matrix4x4 new_model_matrix(Matrix4x4::IDENTITY);
        if (m_axis_mode == EditorAxisMode::TranslateMode) // translate
        {
            Vector3 move_vector = {0, 0, 0};
            if (m_cursor_on_axis == 0)
            {
                move_vector.x = delta_mouse_move_uv.dotProduct(axis_x_direction_uv) * angularVelocity;
            }
            else if (m_cursor_on_axis == 1)
            {
                move_vector.y = delta_mouse_move_uv.dotProduct(axis_y_direction_uv) * angularVelocity;
            }
            else if (m_cursor_on_axis == 2)
            {
                move_vector.z = delta_mouse_move_uv.dotProduct(axis_z_direction_uv) * angularVelocity;
            }
            else
            {
                return;
            }

            Matrix4x4 translate_mat;
            translate_mat.makeTransform(move_vector, Vector3::UNIT_SCALE, Quaternion::IDENTITY);
            new_model_matrix = axis_model_matrix * translate_mat;

            new_model_matrix = new_model_matrix * Matrix4x4(model_rotation);
            new_model_matrix =
                new_model_matrix * Matrix4x4::buildScaleMatrix(model_scale.x, model_scale.y, model_scale.z);

            Vector3    new_scale;
            Quaternion new_rotation;
            Vector3    new_translation;
            new_model_matrix.decomposition(new_translation, new_scale, new_rotation);

            Matrix4x4 translation_matrix = Matrix4x4::getTrans(new_translation);
            Matrix4x4 scale_matrix       = Matrix4x4::buildScaleMatrix(1.f, 1.f, 1.f);
            Matrix4x4 axis_model_matrix  = translation_matrix * scale_matrix;

            m_translation_axis.m_model_matrix = axis_model_matrix;
            m_rotation_axis.m_model_matrix    = axis_model_matrix;
            m_scale_aixs.m_model_matrix       = axis_model_matrix;
            std::vector<RenderMesh> axis_meshs;
            axis_meshs.push_back(m_translation_axis);
            SceneManager::getInstance().setAxisMesh(axis_meshs);

            transform_component->setPosition(new_translation);
            transform_component->setRotation(new_rotation);
            transform_component->setScale(new_scale);
        }
        else if (m_axis_mode == EditorAxisMode::RotateMode) // rotate
        {
            float   last_mouse_u = (last_mouse_pos_x - m_engine_window_pos.x) / m_engine_window_size.x;
            float   last_mouse_v = (last_mouse_pos_y - m_engine_window_pos.y) / m_engine_window_size.y;
            Vector2 last_move_vector(last_mouse_u - model_origin_clip_uv.x, last_mouse_v - model_origin_clip_uv.y);
            float   new_mouse_u = (new_mouse_pos_x - m_engine_window_pos.x) / m_engine_window_size.x;
            float   new_mouse_v = (new_mouse_pos_y - m_engine_window_pos.y) / m_engine_window_size.y;
            Vector2 new_move_vector(new_mouse_u - model_origin_clip_uv.x, new_mouse_v - model_origin_clip_uv.y);
            Vector3 delta_mouse_uv_3(delta_mouse_move_uv.x, delta_mouse_move_uv.y, 0);
            float   move_radian;
            Vector3 axis_of_rotation = {0, 0, 0};
            if (m_cursor_on_axis == 0)
            {
                move_radian = (delta_mouse_move_uv * angularVelocity).length();
                if (m_tmp_uistate->m_editor_camera->forward().dotProduct(Vector3::UNIT_X) < 0)
                {
                    move_radian = -move_radian;
                }
                axis_of_rotation.x = 1;
            }
            else if (m_cursor_on_axis == 1)
            {
                move_radian = (delta_mouse_move_uv * angularVelocity).length();
                if (m_tmp_uistate->m_editor_camera->forward().dotProduct(Vector3::UNIT_Y) < 0)
                {
                    move_radian = -move_radian;
                }
                axis_of_rotation.y = 1;
            }
            else if (m_cursor_on_axis == 2)
            {
                move_radian = (delta_mouse_move_uv * angularVelocity).length();
                if (m_tmp_uistate->m_editor_camera->forward().dotProduct(Vector3::UNIT_Z) < 0)
                {
                    move_radian = -move_radian;
                }
                axis_of_rotation.z = 1;
            }
            else
            {
                return;
            }
            float move_direction = last_move_vector.x * new_move_vector.y - new_move_vector.x * last_move_vector.y;
            if (move_direction < 0)
            {
                move_radian = -move_radian;
            }

            Quaternion move_rot;
            move_rot.fromAngleAxis(Radian(move_radian), axis_of_rotation);
            new_model_matrix = axis_model_matrix * move_rot;
            new_model_matrix = new_model_matrix * Matrix4x4(model_rotation);
            new_model_matrix =
                new_model_matrix * Matrix4x4::buildScaleMatrix(model_scale.x, model_scale.y, model_scale.z);
            Vector3    new_scale;
            Quaternion new_rotation;
            Vector3    new_translation;

            new_model_matrix.decomposition(new_translation, new_scale, new_rotation);

            transform_component->setPosition(new_translation);
            transform_component->setRotation(new_rotation);
            transform_component->setScale(new_scale);
            m_scale_aixs.m_model_matrix = new_model_matrix;
        }
        else if (m_axis_mode == EditorAxisMode::ScaleMode) // scale
        {
            Vector3 delta_scale_vector = {0, 0, 0};
            Vector3 new_model_scale    = {0, 0, 0};
            if (m_cursor_on_axis == 0)
            {
                delta_scale_vector.x = 0.01f;
                if (delta_mouse_move_uv.dotProduct(axis_x_direction_uv) < 0)
                {
                    delta_scale_vector = -delta_scale_vector;
                }
            }
            else if (m_cursor_on_axis == 1)
            {
                delta_scale_vector.y = 0.01f;
                if (delta_mouse_move_uv.dotProduct(axis_y_direction_uv) < 0)
                {
                    delta_scale_vector = -delta_scale_vector;
                }
            }
            else if (m_cursor_on_axis == 2)
            {
                delta_scale_vector.z = 0.01f;
                if (delta_mouse_move_uv.dotProduct(axis_z_direction_uv) < 0)
                {
                    delta_scale_vector = -delta_scale_vector;
                }
            }
            else
            {
                return;
            }
            new_model_scale   = model_scale + delta_scale_vector;
            axis_model_matrix = axis_model_matrix * Matrix4x4(model_rotation);
            Matrix4x4 scale_mat;
            scale_mat.makeTransform(Vector3::ZERO, new_model_scale, Quaternion::IDENTITY);
            new_model_matrix = axis_model_matrix * scale_mat;
            Vector3    new_scale;
            Quaternion new_rotation;
            Vector3    new_translation;
            new_model_matrix.decomposition(new_translation, new_scale, new_rotation);

            transform_component->setPosition(new_translation);
            transform_component->setRotation(new_rotation);
            transform_component->setScale(new_scale);
        }
        m_selected_object_matrix = new_model_matrix;
    }
    void DrawVecControl(const std::string& label, Pilot::Vector3& values, float resetValue, float columnWidth)
    {
        ImGui::PushID(label.c_str());

        ImGui::Columns(2);
        ImGui::SetColumnWidth(0, columnWidth);
        ImGui::Text("%s", label.c_str());
        ImGui::NextColumn();

        ImGui::PushMultiItemsWidths(3, ImGui::CalcItemWidth());
        ImGui::PushStyleVar(ImGuiStyleVar_ItemSpacing, ImVec2 {0, 0});

        float  lineHeight = GImGui->Font->FontSize + GImGui->Style.FramePadding.y * 2.0f;
        ImVec2 buttonSize = {lineHeight + 3.0f, lineHeight};

        ImGui::PushStyleColor(ImGuiCol_Button, ImVec4 {0.8f, 0.1f, 0.15f, 1.0f});
        ImGui::PushStyleColor(ImGuiCol_ButtonHovered, ImVec4 {0.9f, 0.2f, 0.2f, 1.0f});
        ImGui::PushStyleColor(ImGuiCol_ButtonActive, ImVec4 {0.8f, 0.1f, 0.15f, 1.0f});
        if (ImGui::Button("X", buttonSize))
            values.x = resetValue;
        ImGui::PopStyleColor(3);

        ImGui::SameLine();
        ImGui::DragFloat("##X", &values.x, 0.1f, 0.0f, 0.0f, "%.2f");
        ImGui::PopItemWidth();
        ImGui::SameLine();

        ImGui::PushStyleColor(ImGuiCol_Button, ImVec4 {0.2f, 0.45f, 0.2f, 1.0f});
        ImGui::PushStyleColor(ImGuiCol_ButtonHovered, ImVec4 {0.3f, 0.55f, 0.3f, 1.0f});
        ImGui::PushStyleColor(ImGuiCol_ButtonActive, ImVec4 {0.2f, 0.45f, 0.2f, 1.0f});
        if (ImGui::Button("Y", buttonSize))
            values.y = resetValue;
        ImGui::PopStyleColor(3);

        ImGui::SameLine();
        ImGui::DragFloat("##Y", &values.y, 0.1f, 0.0f, 0.0f, "%.2f");
        ImGui::PopItemWidth();
        ImGui::SameLine();

        ImGui::PushStyleColor(ImGuiCol_Button, ImVec4 {0.1f, 0.25f, 0.8f, 1.0f});
        ImGui::PushStyleColor(ImGuiCol_ButtonHovered, ImVec4 {0.2f, 0.35f, 0.9f, 1.0f});
        ImGui::PushStyleColor(ImGuiCol_ButtonActive, ImVec4 {0.1f, 0.25f, 0.8f, 1.0f});
        if (ImGui::Button("Z", buttonSize))
            values.z = resetValue;
        ImGui::PopStyleColor(3);

        ImGui::SameLine();
        ImGui::DragFloat("##Z", &values.z, 0.1f, 0.0f, 0.0f, "%.2f");
        ImGui::PopItemWidth();

        ImGui::PopStyleVar();

        ImGui::Columns(1);
        ImGui::PopID();
    }

    void DrawVecControl(const std::string& label, Pilot::Quaternion& values, float resetValue, float columnWidth)
    {
        ImGui::PushID(label.c_str());

        ImGui::Columns(2);
        ImGui::SetColumnWidth(0, columnWidth);
        ImGui::Text("%s", label.c_str());
        ImGui::NextColumn();

        ImGui::PushMultiItemsWidths(4, ImGui::CalcItemWidth());
        ImGui::PushStyleVar(ImGuiStyleVar_ItemSpacing, ImVec2 {0, 0});

        float  lineHeight = GImGui->Font->FontSize + GImGui->Style.FramePadding.y * 2.0f;
        ImVec2 buttonSize = {lineHeight + 3.0f, lineHeight};

        ImGui::PushStyleColor(ImGuiCol_Button, ImVec4 {0.8f, 0.1f, 0.15f, 1.0f});
        ImGui::PushStyleColor(ImGuiCol_ButtonHovered, ImVec4 {0.9f, 0.2f, 0.2f, 1.0f});
        ImGui::PushStyleColor(ImGuiCol_ButtonActive, ImVec4 {0.8f, 0.1f, 0.15f, 1.0f});
        if (ImGui::Button("X", buttonSize))
            values.x = resetValue;
        ImGui::PopStyleColor(3);

        ImGui::SameLine();
        ImGui::DragFloat("##X", &values.x, 0.1f, 0.0f, 0.0f, "%.2f");
        ImGui::PopItemWidth();
        ImGui::SameLine();

        ImGui::PushStyleColor(ImGuiCol_Button, ImVec4 {0.2f, 0.45f, 0.2f, 1.0f});
        ImGui::PushStyleColor(ImGuiCol_ButtonHovered, ImVec4 {0.3f, 0.55f, 0.3f, 1.0f});
        ImGui::PushStyleColor(ImGuiCol_ButtonActive, ImVec4 {0.2f, 0.45f, 0.2f, 1.0f});
        if (ImGui::Button("Y", buttonSize))
            values.y = resetValue;
        ImGui::PopStyleColor(3);

        ImGui::SameLine();
        ImGui::DragFloat("##Y", &values.y, 0.1f, 0.0f, 0.0f, "%.2f");
        ImGui::PopItemWidth();
        ImGui::SameLine();

        ImGui::PushStyleColor(ImGuiCol_Button, ImVec4 {0.1f, 0.25f, 0.8f, 1.0f});
        ImGui::PushStyleColor(ImGuiCol_ButtonHovered, ImVec4 {0.2f, 0.35f, 0.9f, 1.0f});
        ImGui::PushStyleColor(ImGuiCol_ButtonActive, ImVec4 {0.1f, 0.25f, 0.8f, 1.0f});
        if (ImGui::Button("Z", buttonSize))
            values.z = resetValue;
        ImGui::PopStyleColor(3);

        ImGui::SameLine();
        ImGui::DragFloat("##Z", &values.z, 0.1f, 0.0f, 0.0f, "%.2f");
        ImGui::PopItemWidth();
        ImGui::SameLine();

        ImGui::PushStyleColor(ImGuiCol_Button, ImVec4 {0.5f, 0.25f, 0.5f, 1.0f});
        ImGui::PushStyleColor(ImGuiCol_ButtonHovered, ImVec4 {0.6f, 0.35f, 0.6f, 1.0f});
        ImGui::PushStyleColor(ImGuiCol_ButtonActive, ImVec4 {0.5f, 0.25f, 0.5f, 1.0f});
        if (ImGui::Button("W", buttonSize))
            values.w = resetValue;
        ImGui::PopStyleColor(3);

        ImGui::SameLine();
        ImGui::DragFloat("##W", &values.w, 0.1f, 0.0f, 0.0f, "%.2f");
        ImGui::PopItemWidth();

        ImGui::PopStyleVar();

        ImGui::Columns(1);
        ImGui::PopID();
    }
} // namespace Pilot
