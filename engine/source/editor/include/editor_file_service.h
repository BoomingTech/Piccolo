#pragma once

#include <memory>
#include <string>
#include <vector>

namespace Piccolo
{
    class EditorFileNode;
    using EditorFileNodeArray = std::vector<std::shared_ptr<EditorFileNode>>;

    struct EditorFileNode
    {
        std::string         m_file_name;
        std::string         m_file_type;
        std::string         m_file_path;
        int                 m_node_depth;
        EditorFileNodeArray m_child_nodes;
        EditorFileNode() = default;
        EditorFileNode(const std::string& name, const std::string& type, const std::string& path, int depth) :
            m_file_name(name), m_file_type(type), m_file_path(path), m_node_depth(depth)
        {}
    };

    class EditorFileService
    {
        EditorFileNodeArray m_file_node_array;
        EditorFileNode      m_root_node{ "asset", "Folder", "asset", -1 };

    private:
        EditorFileNode* getParentNodePtr(EditorFileNode* file_node);
        bool            checkFileArray(EditorFileNode* file_node);

    public:
        EditorFileNode* getEditorRootNode() { return m_file_node_array.empty() ? nullptr : m_file_node_array[0].get(); }

        void buildEngineFileTree();
    };
} // namespace Piccolo
