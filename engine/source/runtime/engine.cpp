#include "runtime/engine.h"

#include "runtime/core/base/macro.h"
#include "runtime/core/meta/reflection/reflection_register.h"

#include "runtime/resource/asset_manager/asset_manager.h"
#include "runtime/resource/config_manager/config_manager.h"

#include "runtime/function/framework/world/world_manager.h"
#include "runtime/function/input/input_system.h"
#include "runtime/function/render/include/render/framebuffer.h"
#include "runtime/function/render/include/render/render.h"
#include "runtime/function/render/include/render/surface.h"
#include "runtime/function/scene/scene_manager.h"
#include "runtime/function/ui/ui_system.h"

namespace Pilot
{
    bool g_is_editor_mode {true};

    const FrameBuffer* getFrameBuffer(ThreeFrameBuffers* t, const PilotRenderer*)
    {
        return (t->consumingBufferShift());
    }
    const SceneMemory* getMemoryFromHandle(SceneResourceHandle handle)
    {
        return SceneManager::getInstance().memoryFromHandle(handle);
    }
    const SceneImage* getImageFromHandle(TextureHandle handle)
    {
        return SceneManager::getInstance().imageFromHandle(handle);
    }
    void addReleaseMeshHandle(MeshHandle handle) { SceneManager::getInstance().addReleaseMeshHandle(handle); }
    void addReleaseMaterialHandle(PMaterialHandle handle)
    {
        SceneManager::getInstance().addReleaseMaterialHandle(handle);
    }
    void addReleaseSkeletonBindingHandle(SkeletonBindingBufferHandle handle)
    {
        SceneManager::getInstance().addReleaseSkeletonBindingHandle(handle);
    }

    PilotEngine::PilotEngine() { m_renderer = std::make_shared<PilotRenderer>(); }

    void PilotEngine::startEngine(const EngineInitParams& param)
    {
        Reflection::TypeMetaRegister::Register();

        ConfigManager::getInstance().initialize(param);
        AssetManager::getInstance().initialize();
        PUIManager::getInstance().initialize();

        WorldManager::getInstance().initialize();
        SceneManager::getInstance().initialize();

        m_tri_frame_buffer.initialize();
        m_renderer->RegisterGetPtr(std::bind(&getFrameBuffer, &m_tri_frame_buffer, std::placeholders::_1));
        m_renderer->RegisterGetPtr(std::bind(&getMemoryFromHandle, std::placeholders::_1));
        m_renderer->RegisterGetPtr(std::bind(&getImageFromHandle, std::placeholders::_1));
        m_renderer->RegisterFuncPtr(std::bind(&addReleaseMeshHandle, std::placeholders::_1));
        m_renderer->RegisterFuncPtr(std::bind(&addReleaseMaterialHandle, std::placeholders::_1));
        m_renderer->RegisterFuncPtr(std::bind(&addReleaseSkeletonBindingHandle, std::placeholders::_1));
        m_renderer->initialize();

        LOG_INFO("engine start");
    }

    void PilotEngine::shutdownEngine()
    {

        LOG_INFO("engine shutdown");

        SceneManager::getInstance().clear();
        WorldManager::getInstance().clear();
        PUIManager::getInstance().clear();
        AssetManager::getInstance().clear();
        ConfigManager::getInstance().clear();

        Reflection::TypeMetaRegister::Unregister();

        // m_renderer.clear();
        m_tri_frame_buffer.clear();
    }

    void PilotEngine::initialize() {}
    void PilotEngine::clear() {}
    void PilotEngine::run()
    {
        while (true)
        {
            float delta_time;
            {
                using namespace std::chrono;

                steady_clock::time_point tick_time_point = steady_clock::now();
                duration<float> time_span = duration_cast<duration<float>>(tick_time_point - m_last_tick_time_point);
                delta_time                = time_span.count();

                m_last_tick_time_point = tick_time_point;
            }

            logicalTick(delta_time);
            fps(delta_time);

            if (!rendererTick())
                return;
        }
    }

    void PilotEngine::logicalTick(float delta_time)
    {
        m_tri_frame_buffer.producingBufferShift();
        WorldManager::getInstance().tick(delta_time);
        SceneManager::getInstance().tick(m_tri_frame_buffer.getProducingBuffer());
        InputSystem::getInstance().tick();
        // PhysicsSystem::getInstance().tick(delta_time);
    }

    bool PilotEngine::rendererTick() { return m_renderer->tick(); }

    const float PilotEngine::k_fps_alpha = 1.f / 100;
    void        PilotEngine::fps(float delta_time)
    {
        m_frame_count++;

        if (m_frame_count == 1)
        {
            m_average_duration = delta_time;
        }
        else
        {
            m_average_duration = m_average_duration * (1 - k_fps_alpha) + delta_time * k_fps_alpha;
        }

        m_fps = static_cast<int>(1.f / m_average_duration);
    }

    std::shared_ptr<SurfaceIO> PilotEngine::getSurfaceIO() { return m_renderer->getPSurface()->getSurfaceIO(); }

    void ThreeFrameBuffers::initialize()
    {
        three_buffers._struct._A = new FrameBuffer();
        three_buffers._struct._B = new FrameBuffer();
        three_buffers._struct._C = new FrameBuffer();

        // tri frame buffers are designed to use same scene now
        auto current_scene                = SceneManager::getInstance().getCurrentScene();
        three_buffers._struct._A->m_scene = current_scene;
        three_buffers._struct._B->m_scene = current_scene;
        three_buffers._struct._C->m_scene = current_scene;

        three_buffers._struct._A->m_uistate->m_editor_camera = current_scene->m_camera;
        three_buffers._struct._B->m_uistate->m_editor_camera = current_scene->m_camera;
        three_buffers._struct._C->m_uistate->m_editor_camera = current_scene->m_camera;
    }
    void ThreeFrameBuffers::clear()
    {
        delete (three_buffers._struct._A);
        delete (three_buffers._struct._B);
        delete (three_buffers._struct._C);
    }
    FrameBuffer* ThreeFrameBuffers::producingBufferShift()
    {
        m_last_producing_index = m_producing_index;
        do
        {
            m_producing_index = (m_producing_index + 1) % 3;
        } while (m_consuming_index == m_producing_index || m_last_producing_index == m_producing_index);
        three_buffers._array[m_producing_index]->logicalFrameIndex = ++m_logical_frame_index;
        return three_buffers._array[m_producing_index];
    }
    FrameBuffer*       ThreeFrameBuffers::getProducingBuffer() { return three_buffers._array[m_producing_index]; }
    const FrameBuffer* ThreeFrameBuffers::consumingBufferShift()
    {
        m_consuming_index = m_last_producing_index;
        return three_buffers._array[m_consuming_index];
    }
    const FrameBuffer* ThreeFrameBuffers::getConsumingBuffer() { return three_buffers._array[m_consuming_index]; }
} // namespace Pilot
