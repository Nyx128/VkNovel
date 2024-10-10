#include "vkn_spriterenderingsystem.hpp"

namespace vkn {
	SpriteRenderingSystem::SpriteRenderingSystem(vkn::Context& _context, sprite_update func):context(_context), update_func(func){
		float halfWidth = 8.0f;
		float aspect = 9.f / 16.f;
		proj = ortho_proj(-halfWidth, halfWidth, aspect * halfWidth, -(aspect * halfWidth), 0.01f, 100.0f);
	}

	SpriteRenderingSystem::~SpriteRenderingSystem(){
		sprite_data.clear();
		sprites.clear();
	}

	void SpriteRenderingSystem::addSprite(vkn::Sprite _sprite){
		if (tex_map.find(_sprite.tex) != tex_map.end()) {
			tex_indices.push_back(tex_map[_sprite.tex]);
		}
		else {
			tex_indices.push_back(tid);
			tex_map.insert({ _sprite.tex, tid++ });
			unique_textures.push_back(_sprite.tex);
		}
		sprites.push_back(_sprite);
	}

	const std::vector<vkn::SpriteRenderer::SpriteData>& SpriteRenderingSystem::getSpriteData(){
		//TODO: currently we are recalculating the whole data, thats not ideal. Create a system, where only the things changed are updated to save frame time
		if (sprites.size() > sprite_data.size()){
			sprite_data.resize(sprites.size());
		}
		for (int i = 0; i < sprite_data.size(); i++) {
			sprite_data[i].transform = proj * getTransform(sprites[i]);
			sprite_data[i].col = sprites[i].col;
			sprite_data[i].tex_id = tex_indices[i];
		}
		return sprite_data;
	}

	void SpriteRenderingSystem::update(){
		update_func(sprites);
	}

	const std::vector<std::shared_ptr<vkn::Texture>>& SpriteRenderingSystem::get_textures(){
		return unique_textures;
	}

	glm::mat4 SpriteRenderingSystem::getTransform(vkn::Sprite& sp) {
		//thanks to https://www.youtube.com/@BrendanGalea for the simplified matrix calculations
		const float c3 = glm::cos(sp.rotation.z);
		const float s3 = glm::sin(sp.rotation.z);
		const float c2 = glm::cos(sp.rotation.x);
		const float s2 = glm::sin(sp.rotation.x);
		const float c1 = glm::cos(sp.rotation.y);
		const float s1 = glm::sin(sp.rotation.y);
		glm::mat4 matrix = {
			{
				sp.scale.x * (c1 * c3 + s1 * s2 * s3),
				sp.scale.x * (c2 * s3),
				sp.scale.x * (c1 * s2 * s3 - c3 * s1),
				0.0f,
			},
			{
				sp.scale.y * (c3 * s1 * s2 - c1 * s3),
				sp.scale.y * (c2 * c3),
				sp.scale.y * (c1 * c3 * s2 + s1 * s3),
				0.0f,
			},
			{
				sp.scale.z * (c2 * s1),
				sp.scale.z * (-s2),
				sp.scale.z * (c1 * c2),
				0.0f,
			},
			{sp.pos.x, sp.pos.y, sp.pos.z, 1.0f}
		};

		return matrix;
	}

	glm::mat4 SpriteRenderingSystem::ortho_proj(float left, float right, float top, float bottom, float near, float far){
		glm::mat4 projectionMatrix = glm::mat4{ 1.0f };
		projectionMatrix[0][0] = 2.f / (right - left);
		projectionMatrix[1][1] = 2.f / (bottom - top);
		projectionMatrix[2][2] = 1.f / (far - near);
		projectionMatrix[3][0] = -(right + left) / (right - left);
		projectionMatrix[3][1] = -(bottom + top) / (bottom - top);
		projectionMatrix[3][2] = -near / (far - near);

		return projectionMatrix;
	}
}

