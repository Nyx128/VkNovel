#pragma once
#include "glm/glm.hpp"

namespace vkn {
	struct Sprite {
		glm::vec3 pos;
		glm::vec3 scale;
		glm::vec3 rotation;

		glm::mat4 getTransform() {
			//thanks to https://www.youtube.com/@BrendanGalea for the simplified matrix calculations
			const float c3 = glm::cos(rotation.z);
			const float s3 = glm::sin(rotation.z);
			const float c2 = glm::cos(rotation.x);
			const float s2 = glm::sin(rotation.x);
			const float c1 = glm::cos(rotation.y);
			const float s1 = glm::sin(rotation.y);
			glm::mat4 matrix = {
				{
					scale.x * (c1 * c3 + s1 * s2 * s3),
					scale.x * (c2 * s3),
					scale.x * (c1 * s2 * s3 - c3 * s1),
					0.0f,
				},
				{
					scale.y * (c3 * s1 * s2 - c1 * s3),
					scale.y * (c2 * c3),
					scale.y * (c1 * c3 * s2 + s1 * s3),
					0.0f,
				},
				{
					scale.z * (c2 * s1),
					scale.z * (-s2),
					scale.z * (c1 * c2),
					0.0f,
				},
				{pos.x, pos.y, pos.z, 1.0f}
			};

			return matrix;
		}
	};
}
