#include "backend/vkn_window.hpp"
#include "backend/vkn_context.hpp"
#include "backend/vkn_pipeline.hpp"
#include "backend/vkn_spriterenderer.hpp"

#include "game/vkn_sprite.hpp"
#include "game/vkn_texture.hpp"

#include "systems/vkn_spriterenderingsystem.hpp"

#include <iostream>
#include <stdexcept>
#include <chrono>
#include <random>
#include <cmath>

#include "glm/glm.hpp"

void sp_update(std::vector<vkn::Sprite>& sprites) {
	for (auto& sp : sprites) {
		sp.rotation.z += glm::radians(2.0f);
	}
}

void run() {
	vkn::Window win(1600, 900, "VkNovel", true);

	vkn::ContextInfo contextInfo;
	contextInfo.appVersion = VK_MAKE_VERSION(1, 0, 0);
	contextInfo.app_name = std::string("sandbox");

	vkn::Context context(contextInfo, win);

	std::shared_ptr<vkn::Texture> statue= std::make_shared<vkn::Texture>(context, "assets/statue.jpg");
	std::shared_ptr<vkn::Texture> zoro = std::make_shared<vkn::Texture>(context, "assets/zoro.png");

	vkn::SpriteRenderer srenderer{ context };

	std::string title = "";
	int sample_rate = 30;
	int sample_index = 0;
	float sample_time = 0.0f;

	vkn::SpriteRenderingSystem sp_sys(context, sp_update);
	std::random_device rd;

	vkn::Sprite sp(zoro);

	sp.scale = glm::vec3(0.5f);
	sp.pos = glm::vec3(0.0f, 0.0f, 1.0f);
	sp.rotation = glm::vec3(0.0f, 0.0f, 0.0f);
	sp.col = glm::vec3(1.0f);

	sp_sys.addSprite(sp);

	sp.tex = statue;
	sp.pos = glm::vec3(0.6f, 0.0f, 1.5f);
	sp.col = glm::vec3(0.0f);
	sp.scale = glm::vec3(1.0f);

	sp_sys.addSprite(sp);

	float timer = 0.0f;
	while (!win.isClosed()) {
		auto start = std::chrono::high_resolution_clock::now();

		sp_sys.update();

		srenderer.begin();
		srenderer.draw(sp_sys.getSpriteData(), sp_sys.get_textures());
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