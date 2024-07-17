#pragma once

#include "game/vkn_sprite.hpp"
#include "glm/glm.hpp"
#include "glm/gtc/matrix_transform.hpp"
#include "backend/vkn_context.hpp"
#include "backend/vkn_spriterenderer.hpp"

namespace vkn{
	class SpriteRenderingSystem {
	public:
		SpriteRenderingSystem(const SpriteRenderingSystem&) = delete;
		SpriteRenderingSystem& operator=(const SpriteRenderingSystem&) = delete;

		SpriteRenderingSystem(vkn::Context& _context);
		~SpriteRenderingSystem();

		void addSprite(vkn::Sprite _sprite);
		std::vector<vkn::SpriteRenderer::SpriteData>& getSpriteData();
	private:
		vkn::Context& context;
		std::vector<vkn::Sprite> sprites;

		std::vector<vkn::SpriteRenderer::SpriteData> sprite_data;
		bool updated = false;

		glm::mat4 proj = glm::mat4(1.0f);

		glm::mat4 SpriteRenderingSystem::getTransform(vkn::Sprite& sp);
		glm::mat4 ortho_proj(float left, float right, float top, float bottom, float near, float far);
	};
}