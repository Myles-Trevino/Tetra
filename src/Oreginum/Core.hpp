#pragma once
#include <string>
#include <GLM/glm.hpp>

namespace Oreginum
{
	namespace Core
	{
		void initialize(const std::string& title, const glm::ivec2& resolution,
			bool vsync = true, bool terminal = false, bool debug = false);
		void destroy();

		void error(const std::string& error);
		bool update();

		uint32_t get_refresh_rate();
		const glm::ivec2& get_screen_resolution();
		float get_time();
		float get_delta();
		bool get_debug();
	}
}
