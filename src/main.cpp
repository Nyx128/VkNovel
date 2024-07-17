#include "backend/vkn_window.hpp"
#include "backend/vkn_context.hpp"
#include "backend/vkn_pipeline.hpp"
#include "backend/vkn_spriterenderer.hpp"

#include "game/vkn_sprite.hpp"

#include "systems/vkn_spriterenderingsystem.hpp"

#include <iostream>
#include <stdexcept>
#include <chrono>
#include <random>
#include <cmath>

#define GLM_FORCE_RADIANS
#define GLM_FORCE_DEPTH_ZERO_TO_ONE
#include "glm/glm.hpp"
#include "glm/gtc/matrix_transform.hpp"

glm::mat4 ortho_proj(float left, float right, float top, float bottom, float near, float far) {
	glm::mat4 projectionMatrix = glm::mat4{ 1.0f };
	projectionMatrix[0][0] = 2.f / (right - left);
	projectionMatrix[1][1] = 2.f / (bottom - top);
	projectionMatrix[2][2] = 1.f / (far - near);
	projectionMatrix[3][0] = -(right + left) / (right - left);
	projectionMatrix[3][1] = -(bottom + top) / (bottom - top);
	projectionMatrix[3][2] = -near / (far - near);

	return projectionMatrix;
}

void run() {
	vkn::Window win(1600, 900, "VkNovel", true);

	vkn::ContextInfo contextInfo;
	contextInfo.appVersion = VK_MAKE_VERSION(1, 0, 0);
	contextInfo.app_name = std::string("sandbox");

	vkn::Context context(contextInfo, win);

	vkn::SpriteRenderer srenderer{ context };

	std::string title = "";
	int sample_rate = 30;
	int sample_index = 0;
	float sample_time = 0.0f;

	vkn::SpriteRenderingSystem sp_sys(context);
	std::random_device rd;

	// Choose a random number generator and seed it
	std::mt19937 gen(rd());

	std::uniform_real_distribution<> pos_dis_x(-7.5f, 7.5f);
	std::uniform_real_distribution<> pos_dis_y(-4.0f, 4.0f);
	std::uniform_real_distribution<> rot_dis(-180.0f, 180.0f);
	for (int i = 0; i < 1024; i++) {
		vkn::Sprite sp;

		sp.scale = glm::vec3(0.03f);
		sp.pos = glm::vec3(pos_dis_x(gen), pos_dis_y(gen), 1.0f);
		sp.rotation = glm::vec3(0.0f, 0.0f, glm::radians(rot_dis(gen)));

		sp_sys.addSprite(sp);
	}

	float timer = 0.0f;
	while (!win.isClosed()) {
		auto start = std::chrono::high_resolution_clock::now();

		srenderer.begin();
		srenderer.draw(sp_sys.getSpriteData());
		srenderer.end();
		win.pollEvents();

		auto end = std::chrono::high_resolution_clock::now();
		float dt = std::chrono::duration_cast<std::chrono::milliseconds>(end - start).count();
		timer += dt;
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