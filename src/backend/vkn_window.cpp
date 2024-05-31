#include "vkn_window.hpp"

#include <stdexcept>


namespace vkn {
    //this is a callback function, DO NOT CALL THIS ANYWHERE
    void key_callback(GLFWwindow* window, int key, int scancode, int action, int mods) {
        vkn::Window* win = reinterpret_cast<vkn::Window*>(glfwGetWindowUserPointer(window));
        win->key_callback(key, scancode, action, mods);
    }

    Window::Window(int _width, int _height, const char* _title, bool _vsync) :width(_width), height(_height), title(_title), vsync(_vsync) {
        if (!glfwInit()) {
            throw std::runtime_error("Failed to initialize GLFW");
        }

        glfwWindowHint(GLFW_RESIZABLE, GLFW_FALSE);
        glfwWindowHint(GLFW_CLIENT_API, GLFW_NO_API);

        handle = glfwCreateWindow(width, height, title, NULL, NULL);
        if (!handle) {
            glfwTerminate();
            throw std::runtime_error("Failed to create window");
        }

        glfwMakeContextCurrent(handle);
        glfwSetWindowUserPointer(handle, this);
        glfwSetKeyCallback(handle, vkn::key_callback);

        for (int i = 0; i < keys.size(); i++) {
            keys[i] = false;
        }

        for (int i = 0; i < buttons.size(); i++) {
            buttons[i] = false;
        }

        if (vsync) {
            glfwSwapInterval(1);
        }
        else {
            glfwSwapInterval(0);
        }
    }

    Window::~Window() {
        glfwDestroyWindow(handle);
        glfwTerminate();
    }

    void Window::pollEvents() {
        glfwPollEvents();
    }

    void Window::close() {
        glfwSetWindowShouldClose(handle, GLFW_TRUE);
    }

    void Window::key_callback(int key, int scancode, int action, int mods) {
        keys[key] = action;
    }

}