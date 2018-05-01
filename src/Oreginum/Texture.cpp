#include "Core.hpp"
#include "Renderer Core.hpp"
#include "Texture.hpp"
#define STB_IMAGE_IMPLEMENTATION
#include <STB IMAGE/stb_image.h>

Oreginum::Texture::Texture(const std::vector<std::string>& paths, const Vulkan::Sampler& sampler,
	Format type, bool cubemap) : type(type)
{
	
	std::vector<void *> datas;
	for(unsigned i{}; i < paths.size(); ++i)
	{
		void *data;
		glm::ivec2 resolution;
		if(type == Format::HDR) data = stbi_loadf(paths[i].c_str(),
			&resolution.x, &resolution.y, nullptr, STBI_rgb_alpha);
		else data = stbi_load(paths[i].c_str(), &resolution.x,
			&resolution.y, nullptr, STBI_rgb_alpha);
		if(!data) Core::error("Could not load image \""+paths[i]+"\".");

		datas.push_back(data);
		if(i == 0) this->resolution = resolution;
		else if(this->resolution != resolution) Core::error("Could not load"
			"image array because \""+paths[i]+"\" is a different resolution.");
	}

	image = Vulkan::Image{Renderer_Core::get_device(), sampler,
		Renderer_Core::get_temporary_command_buffer(),
		this->resolution, datas, get_format(), cubemap};

	for(void *d : datas) stbi_image_free(d);
}

vk::Format Oreginum::Texture::get_format()
{
	return (type == RGB) ? Vulkan::Image::RGB_FORMAT : (type == LINEAR) ?
		Vulkan::Image::LINEAR_FORMAT : Vulkan::Image::HDR_FORMAT_32;
}