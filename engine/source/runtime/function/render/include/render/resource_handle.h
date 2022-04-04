#pragma once

namespace Pilot
{
    struct SceneResourceHandle
    {
        size_t handle;
        bool   operator==(const SceneResourceHandle& rhs) const { return handle == rhs.handle; }
        size_t getHashValue() const { return handle; }
    };
    struct VertexBufferHandle : SceneResourceHandle
    {};
    struct IndexBufferHandle : SceneResourceHandle
    {};
    struct TextureHandle : SceneResourceHandle
    {};
    struct DynamicVertexBufferHandle : SceneResourceHandle
    {};
    struct SkeletonBindingBufferHandle : SceneResourceHandle
    {};

#define PILOT_INVALID_HANDLE {0};
#define PILOT_INVALID_GUID 0
} // namespace Pilot
