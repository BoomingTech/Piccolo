#pragma once

#include <GLFW/glfw3.h>

#include <functional>
#include <iostream>

namespace Pilot
{
    class SurfaceIO
    {
        static void keyCallback(GLFWwindow* window, int key, int scancode, int action, int mods)
        {
            Pilot::SurfaceIO* app = (Pilot::SurfaceIO*)glfwGetWindowUserPointer(window);
            app->onKey(key, scancode, action, mods);
        }
        static void charCallback(GLFWwindow* window, unsigned int codepoint)
        {
            Pilot::SurfaceIO* app = (Pilot::SurfaceIO*)glfwGetWindowUserPointer(window);
            app->onChar(codepoint);
        }
        static void charModsCallback(GLFWwindow* window, unsigned int codepoint, int mods)
        {
            Pilot::SurfaceIO* app = (Pilot::SurfaceIO*)glfwGetWindowUserPointer(window);
            app->onCharMods(codepoint, mods);
        }
        static void mouseButtonCallback(GLFWwindow* window, int button, int action, int mods)
        {
            Pilot::SurfaceIO* app = (Pilot::SurfaceIO*)glfwGetWindowUserPointer(window);
            app->onMouseButton(button, action, mods);
        }
        static void cursorPosCallback(GLFWwindow* window, double xpos, double ypos)
        {
            Pilot::SurfaceIO* app = (Pilot::SurfaceIO*)glfwGetWindowUserPointer(window);
            app->onCursorPos(xpos, ypos);
        }
        static void cursorEnterCallback(GLFWwindow* window, int entered)
        {
            Pilot::SurfaceIO* app = (Pilot::SurfaceIO*)glfwGetWindowUserPointer(window);
            app->onCursorEnter(entered);
        }
        static void scrollCallback(GLFWwindow* window, double xoffset, double yoffset)
        {
            Pilot::SurfaceIO* app = (Pilot::SurfaceIO*)glfwGetWindowUserPointer(window);
            app->onScroll(xoffset, yoffset);
        }
        static void dropCallback(GLFWwindow* window, int count, const char** paths)
        {
            Pilot::SurfaceIO* app = (Pilot::SurfaceIO*)glfwGetWindowUserPointer(window);
            app->onDrop(count, paths);
        }
        static void windowSizeCallback(GLFWwindow* window, int width, int height)
        {
            Pilot::SurfaceIO* app = (Pilot::SurfaceIO*)glfwGetWindowUserPointer(window);
            app->m_width          = width;
            app->m_height         = height;
            app->reset(app->m_reset);
            app->onWindowSize(width, height);
        }
        static void windowCloseCallback(GLFWwindow* window) { glfwSetWindowShouldClose(window, true); }

        typedef std::function<void()>                   onResetFunc;
        typedef std::function<void(int, int, int, int)> onKeyFunc;
        typedef std::function<void(unsigned int)>       onCharFunc;
        typedef std::function<void(int, unsigned int)>  onCharModsFunc;
        typedef std::function<void(int, int, int)>      onMouseButtonFunc;
        typedef std::function<void(double, double)>     onCursorPosFunc;
        typedef std::function<void(int)>                onCursorEnterFunc;
        typedef std::function<void(double, double)>     onScrollFunc;
        typedef std::function<void(int, const char**)>  onDropFunc;
        typedef std::function<void(int, int)>           onWindowSizeFunc;
        typedef std::function<void()>                   onWindowCloseFunc;

        std::vector<onResetFunc>       m_onResetFunc;
        std::vector<onKeyFunc>         m_onKeyFunc;
        std::vector<onCharFunc>        m_onCharFunc;
        std::vector<onCharModsFunc>    m_onCharModsFunc;
        std::vector<onMouseButtonFunc> m_onMouseButtonFunc;
        std::vector<onCursorPosFunc>   m_onCursorPosFunc;
        std::vector<onCursorEnterFunc> m_onCursorEnterFunc;
        std::vector<onScrollFunc>      m_onScrollFunc;
        std::vector<onDropFunc>        m_onDropFunc;
        std::vector<onWindowSizeFunc>  m_onWindowSizeFunc;
        std::vector<onWindowCloseFunc> m_onWindowCloseFunc;

        void onReset()
        {
            for (auto& func : m_onResetFunc)
                func();
        }
        void onKey(int key, int scancode, int action, int mods)
        {
            for (auto& func : m_onKeyFunc)
                func(key, scancode, action, mods);
        }
        void onChar(unsigned int codepoint)
        {
            for (auto& func : m_onCharFunc)
                func(codepoint);
        }
        void onCharMods(int codepoint, unsigned int mods)
        {
            for (auto& func : m_onCharModsFunc)
                func(codepoint, mods);
        }
        void onMouseButton(int button, int action, int mods)
        {
            for (auto& func : m_onMouseButtonFunc)
                func(button, action, mods);
        }
        void onCursorPos(double xpos, double ypos)
        {
            for (auto& func : m_onCursorPosFunc)
                func(xpos, ypos);
        }
        void onCursorEnter(int entered)
        {
            for (auto& func : m_onCursorEnterFunc)
                func(entered);
        }
        void onScroll(double xoffset, double yoffset)
        {
            for (auto& func : m_onScrollFunc)
                func(xoffset, yoffset);
        }
        void onDrop(int count, const char** paths)
        {
            for (auto& func : m_onDropFunc)
                func(count, paths);
        }
        void onWindowSize(int width, int height)
        {
            for (auto& func : m_onWindowSizeFunc)
                func(width, height);
        }

    public:
        void onWindowClose()
        {

            for (auto& func : m_onWindowCloseFunc)
                func();
        }

    public:
        void        registerOnResetFunc(onResetFunc func) { m_onResetFunc.push_back(func); }
        void        registerOnKeyFunc(onKeyFunc func) { m_onKeyFunc.push_back(func); }
        void        registerOnCharFunc(onCharFunc func) { m_onCharFunc.push_back(func); }
        void        registerOnCharModsFunc(onCharModsFunc func) { m_onCharModsFunc.push_back(func); }
        void        registerOnMouseButtonFunc(onMouseButtonFunc func) { m_onMouseButtonFunc.push_back(func); }
        void        registerOnCursorPosFunc(onCursorPosFunc func) { m_onCursorPosFunc.push_back(func); }
        void        registerOnCursorEnterFunc(onCursorEnterFunc func) { m_onCursorEnterFunc.push_back(func); }
        void        registerOnScrollFunc(onScrollFunc func) { m_onScrollFunc.push_back(func); }
        void        registerOnDropFunc(onDropFunc func) { m_onDropFunc.push_back(func); }
        void        registerOnWindowSizeFunc(onWindowSizeFunc func) { m_onWindowSizeFunc.push_back(func); }
        void        registerOnWindowCloseFunc(onWindowCloseFunc func) { m_onWindowCloseFunc.push_back(func); }
        uint32_t    getWidth() const { return m_width; }
        uint32_t    getHeight() const { return m_height; }
        void        setSize(int width, int height) { glfwSetWindowSize(m_window, width, height); }
        const char* getTitle() const { return m_title; }
        void        setTitle(const char* title)
        {
            m_title = title;
            glfwSetWindowTitle(m_window, title);
        }
        bool m_is_editor_mode {true};
        bool m_is_focus_mode {false};
        void setEditorMode(bool mode) { m_is_editor_mode = mode; }
        void setFocusMode(bool mode) { m_is_focus_mode = mode; }
        void toggleFullscreen()
        {
            static int oldX = 0, oldY = 0;
            static int oldWidth = 0, oldHeight = 0;

            // GLFW didn't create the context, bgfx did
            // glfwGetCurrentContext returns null
            GLFWwindow* window = m_window; // glfwGetCurrentContext();
            if (glfwGetWindowMonitor(window))
            {
                glfwSetWindowMonitor(window, NULL, oldX, oldY, oldWidth, oldHeight, 0);
                // config->fullscreen = false;
            }
            else
            {
                GLFWmonitor* monitor = glfwGetPrimaryMonitor();
                if (NULL != monitor)
                {
                    glfwGetWindowPos(window, &oldX, &oldY);
                    glfwGetWindowSize(window, &oldWidth, &oldHeight);

                    const GLFWvidmode* mode = glfwGetVideoMode(monitor);
                    glfwSetWindowMonitor(window, monitor, 0, 0, mode->width, mode->height, mode->refreshRate);
                    // config->fullscreen = true;
                }
            }
        }
        bool isKeyDown(int key) const
        {
            if (key < GLFW_KEY_SPACE || key > GLFW_KEY_LAST)
            {
                return false;
            }
            return glfwGetKey(m_window, key) == GLFW_PRESS;
        }
        bool isMouseButtonDown(int button) const
        {
            if (button < GLFW_MOUSE_BUTTON_1 || button > GLFW_MOUSE_BUTTON_LAST)
            {
                return false;
            }

            return glfwGetMouseButton(m_window, button) == GLFW_PRESS;
        }
        float getMouseWheelH() const { return m_mouse_wheel_h; }
        float getMouseWheel() const { return m_mouse_wheel; }
        void  reset(uint32_t flags = 0)
        {
            m_reset = flags;
            onReset();
        }
        GLFWwindow* m_window {nullptr};

        uint32_t    m_width;
        uint32_t    m_height;
        const char* m_title;
        float       m_mouse_wheel_h = 0.0f;
        float       m_mouse_wheel   = 0.0f;

        uint32_t m_reset {0};
        SurfaceIO(const char* title, uint32_t width, uint32_t height)
        {
            m_width  = width;
            m_height = height;
            m_title  = title;
        }
        int initialize()
        {
            if (!glfwInit())
            {
                return -1;
            }

            glfwWindowHint(GLFW_CLIENT_API, GLFW_NO_API);
            m_window = glfwCreateWindow(getWidth(), getHeight(), getTitle(), NULL, NULL);
            if (!m_window)
            {
                glfwTerminate();
                return -1;
            }

            // Setup input callbacks
            glfwSetWindowUserPointer(m_window, this);
            glfwSetKeyCallback(m_window, keyCallback);
            glfwSetCharCallback(m_window, charCallback);
            glfwSetCharModsCallback(m_window, charModsCallback);
            glfwSetMouseButtonCallback(m_window, mouseButtonCallback);
            glfwSetCursorPosCallback(m_window, cursorPosCallback);
            glfwSetCursorEnterCallback(m_window, cursorEnterCallback);
            glfwSetScrollCallback(m_window, scrollCallback);
            glfwSetDropCallback(m_window, dropCallback);
            glfwSetWindowSizeCallback(m_window, windowSizeCallback);
            glfwSetWindowCloseCallback(m_window, windowCloseCallback);

            glfwSetInputMode(m_window, GLFW_RAW_MOUSE_MOTION, GLFW_FALSE);
            return 0;
        }
        bool tick_NotQuit()
        {
            if (!glfwWindowShouldClose(m_window))
            {
                glfwPollEvents();
                return true;
            }
            return false;
        }
        int clear()
        {
            glfwDestroyWindow(m_window);
            glfwTerminate();
            return 0;
        }
    };
} // namespace Pilot
