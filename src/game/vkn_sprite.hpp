#pragma once

#define GLM_FORCE_RADIANS
#define GLM_FORCE_DEPTH_ZERO_TO_ONE
#include "glm/glm.hpp"

#include "game/vkn_texture.hpp"

namespace vkn {
	class Sprite {
	public:
		Sprite(std::shared_ptr<vkn::Texture>& _tex, glm::vec3 _pos = glm::vec3(0.0f),
			glm::vec3 _scale=glm::vec3(1.0f), glm::vec3 _rotation = glm::vec3(0.0f), glm::vec3 _col = glm::vec3(1.0f));
		~Sprite();

		glm::vec3 pos;
		glm::vec3 scale;
		glm::vec3 rotation;
		glm::vec3 col;
		std::shared_ptr<vkn::Texture>& tex;
	};
}
