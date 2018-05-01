#pragma once
#include <string>
#include <GLM/glm.hpp>
#include "../Vulkan/Image.hpp"
#include "../Vulkan/Sampler.hpp"

namespace Oreginum
{
	class Texture
	{
	public:
		enum Format{RGB, LINEAR, HDR};

		Texture(){}
		Texture(const std::string& path, const Vulkan::Sampler& sampler, Format type = RGB)
			: Texture(std::vector<std::string>{path}, sampler, type){}
		Texture(const std::vector<std::string>& paths, const Vulkan::Sampler& sampler,
			Format type = RGB, bool cubemap = false);

		const Vulkan::Image& get_image() const { return image; }

	private:
		Vulkan::Image image;
		Format type;
		glm::ivec2 resolution;

		vk::Format get_format();
	};
}