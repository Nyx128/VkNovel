#include "vulkan/vulkan.hpp"
#include "GLFW/glfw3.h"

#include "backend/vkn_window.hpp"
#include "backend/vkn_context.hpp"

#include <iostream>
#include <stdexcept>

void run() {
	vkn::Window win(1280, 720, "VkNovel", true);

	vkn::ContextInfo contextInfo;
	contextInfo.appVersion = VK_MAKE_VERSION(1, 0, 0);
	contextInfo.app_name = std::string("sandbox");

	vkn::Context context(contextInfo, win);

	while (!win.isClosed()) {
		win.pollEvents();
	}
}

int main() {
	try {
		run();
	}
	catch (vk::SystemError& sysErr) {
		std::cout << sysErr.what() << std::endl;
		return EXIT_FAILURE;
	}
	catch (std::exception& ex) {
		std::cout << ex.what() << std::endl;
		return EXIT_FAILURE;
	}
	catch (...) {
		std::cout << "UNKNOWN ERROR" << std::endl;
		return EXIT_FAILURE;
	}
}