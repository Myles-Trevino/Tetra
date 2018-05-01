#pragma once

namespace Oreginum
{
	class Uniform
	{
	public:
		Uniform(){}

		struct Descriptor_Information
		{
			vk::DescriptorType type;
			const vk::DescriptorImageInfo *image;
			const vk::DescriptorBufferInfo *buffer;
		};

		virtual Descriptor_Information get_descriptor_information() const = 0;

	private:
	};
}