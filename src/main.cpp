#include "backend/vkn_window.hpp"
#include "backend/vkn_context.hpp"
#include "backend/vkn_pipeline.hpp"
#include "backend/vkn_spriterenderer.hpp"

#include <iostream>
#include <stdexcept>
#include <chrono>

void run() {
	vkn::Window win(1280, 720, "VkNovel", true);

	vkn::ContextInfo contextInfo;
	contextInfo.appVersion = VK_MAKE_VERSION(1, 0, 0);
	contextInfo.app_name = std::string("sandbox");

	vkn::Context context(contextInfo, win);

	std::vector<float> vertices = {
			0.5f,  0.5f, 0.0f,  // top right
			0.5f, -0.5f, 0.0f,  // bottom right
			-0.5f, -0.5f, 0.0f,  // bottom left
			-0.5f,  0.5f, 0.0f   // top left 
	};

	std::vector<uint32_t> indices = {
		0, 1, 3,   // first triangle
		1, 2, 3    // second triangle
	};

	vkn::SpriteRenderer srenderer(context);

	std::string title = "";
	int sample_rate = 30;
	int sample_index = 0;
	float sample_time = 0.0f;
	while (!win.isClosed()) {
		auto start = std::chrono::high_resolution_clock::now();

		srenderer.render(vertices, indices);
		win.pollEvents();

		auto end = std::chrono::high_resolution_clock::now();
		float dt = std::chrono::duration_cast<std::chrono::milliseconds>(end - start).count();
		if (sample_index > sample_rate) {
			float avg_dt = sample_time / sample_rate;
			title = std::string("VkNovel") + std::string(" FPS: ") + std::to_string((int)round(1000.0f / avg_dt));
			win.setTitle(title.c_str());
			sample_index = 0;
			sample_time = 0.0f;
		}
		sample_index++;
		sample_time += dt;
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