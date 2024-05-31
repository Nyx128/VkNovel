#pragma once
#define GLFW_INCLUDE_NONE
#include "GLFW/glfw3.h"

#include <array>

namespace vkn {
	class Window {
	public:
		Window(const Window&) = delete;
		Window& operator=(const Window&) = delete;

		Window(int _width, int _height, const char* _title, bool _vsync = true);
		~Window();

		void pollEvents();

		void close();

		GLFWwindow* getHandle() { return handle; }

		inline void setVsync(bool flag) { vsync = flag; }
		inline bool isClosed() { return glfwWindowShouldClose(handle); }

		inline int isKeyPressed(int key) { return keys[key]; }

		//do not call this by any chance
		void key_callback(int key, int scancode, int action, int mods);
	private:
		GLFWwindow* handle;
		int width, height;
		const char* title;
		bool vsync = true;

		std::array<int, 512> keys;
		std::array<int, 128> buttons;

	};
}