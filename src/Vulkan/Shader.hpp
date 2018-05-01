#pragma once
#include <string>
#define NOMINMAX
#define VK_USE_PLATFORM_WIN32_KHR
#define VULKAN_HPP_NO_EXCEPTIONS
#include <Vulkan/vulkan.h>
#include "Device.hpp"

namespace Oreginum::Vulkan
{
	class Shader
	{
	public:
		Shader(){}
		Shader(std::shared_ptr<Device> device, const std::vector<std::pair<
			std::string, vk::ShaderStageFlagBits>>& shaders);
		Shader *operator=(Shader other){ swap(&other); return this; }
		~Shader();

		const std::vector<vk::PipelineShaderStageCreateInfo>& get() const { return information; }

	private:
		std::shared_ptr<Device> device;
		std::shared_ptr<std::vector<vk::ShaderModule>> modules =
			std::make_unique<std::vector<vk::ShaderModule>>();
		std::vector<vk::PipelineShaderStageCreateInfo> information;

		void swap(Shader *other);
		vk::ShaderModule create_shader_module(const std::string& shader);
	};
}