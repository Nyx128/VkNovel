#include "vkn_sprite.hpp"

namespace vkn {
	Sprite::Sprite(std::shared_ptr<vkn::Texture>& _tex, glm::vec3 _pos, glm::vec3 _scale, glm::vec3 _rotation, glm::vec3 _col)
		:tex(_tex), pos(_pos), scale(_scale), rotation(_rotation), col(_col){
	}
	Sprite::~Sprite(){
	}
}