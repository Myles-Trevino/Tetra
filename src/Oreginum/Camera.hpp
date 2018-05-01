#pragma once
#include <GLM/glm.hpp>

namespace Oreginum
{
	namespace Camera
	{
		void update();

		void set_frozen(bool frozen);
		void set_position(const glm::fvec3& position);

		bool get_frozen();
		float get_fov();
		const glm::fvec3& get_position();
		const glm::fvec3& get_direction();
		const glm::fmat4& get_view();
		const glm::fmat4& get_projection();
        glm::fmat4 get_normalized_orthographic();
        glm::fmat4 get_window_orthographic();
	}
}