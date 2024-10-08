#pragma once
#include <unordered_map>

#include "game/vkn_sprite.hpp"
#include "glm/glm.hpp"

#include "backend/vkn_context.hpp"
#include "backend/vkn_spriterenderer.hpp"

namespace vkn{
	class SpriteRenderingSystem {
	public:
		SpriteRenderingSystem(const SpriteRenderingSystem&) = delete;
		SpriteRenderingSystem& operator=(const SpriteRenderingSystem&) = delete;

		typedef void(*sprite_update)(std::vector<vkn::Sprite>&);

		SpriteRenderingSystem(vkn::Context& _context, sprite_update func);
		~SpriteRenderingSystem();

		void addSprite(vkn::Sprite _sprite);
		std::vector<vkn::SpriteRenderer::SpriteData>& getSpriteData();

		void update();

		const std::vector<std::shared_ptr<vkn::Texture>>& get_textures();
	private:
		vkn::Context& context;
		std::vector<vkn::Sprite> sprites;

		std::vector<vkn::SpriteRenderer::SpriteData> sprite_data;

		glm::mat4 proj = glm::mat4(1.0f);

		sprite_update update_func;

		glm::mat4 SpriteRenderingSystem::getTransform(vkn::Sprite& sp);
		glm::mat4 ortho_proj(float left, float right, float top, float bottom, float near, float far);

		uint32_t tid = 0;
		std::unordered_map<std::shared_ptr<vkn::Texture>, uint32_t> tex_map;
		std::vector<uint32_t> tex_indices;
		std::vector<std::shared_ptr<vkn::Texture>> unique_textures;
	};
}