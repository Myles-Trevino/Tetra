#include <GLM/gtx/transform.hpp>
#include "../Oreginum/Camera.hpp"
#include "Render Group.hpp"

Oreginum::Vulkan::Sampler Tetra::Render_Group::sampler;
Oreginum::Vulkan::Descriptor_Set Tetra::Render_Group::descriptor_set;
Oreginum::Texture Tetra::Render_Group::texture_map;

Tetra::Render_Group::Render_Group(const std::vector<float>& vertices, const std::vector<uint32_t>& indices,
	uint8_t material_type, const glm::fvec3& translation) :
	material_type(material_type), translation(translation)
{
	//Create vertex and index buffers
	vertex_buffer = {Oreginum::Renderer_Core::get_device(),
		Oreginum::Renderer_Core::get_temporary_command_buffer(),
		vk::BufferUsageFlagBits::eVertexBuffer,
		sizeof(float)*vertices.size(), vertices.data()};
	index_buffer = {Oreginum::Renderer_Core::get_device(),
		Oreginum::Renderer_Core::get_temporary_command_buffer(),
		vk::BufferUsageFlagBits::eIndexBuffer,
		sizeof(uint32_t)*indices.size(), indices.data()};
	index_count = static_cast<int32_t>(indices.size());

	//Load texture array and create sampler if not already created
	if(texture_map.get_image().get()) return;
	sampler = {Oreginum::Renderer_Core::get_device(), 9, true};
	texture_map = {std::vector<std::string>{
		"Resources/Textures/Stone.png",
		"Resources/Textures/Dirt Specular.png",
		"Resources/Textures/Dirt.png",
		"Resources/Textures/Dirt Specular.png",
		"Resources/Textures/Grass.png",
		"Resources/Textures/Dirt Specular.png",
		"Resources/Textures/Sand.png",
		"Resources/Textures/Dirt Specular.png",
		"Resources/Textures/Wood.png",
		"Resources/Textures/Dirt Specular.png",
		"Resources/Textures/Leaves.png",
		"Resources/Textures/Dirt Specular.png",
	}, sampler, Oreginum::Texture::LINEAR};
}
	
void Tetra::Render_Group::initialize_descriptor()
{
	descriptor_set =
		{Oreginum::Renderer_Core::get_device(), Oreginum::Renderer_Core::get_descriptor_pool(),
		{{vk::DescriptorType::eCombinedImageSampler, vk::ShaderStageFlagBits::eFragment}}};

	descriptor_set.write({&texture_map.get_image()});
}

void Tetra::Render_Group::update()
{
	uniforms.projection = Oreginum::Camera::get_projection();
	uniforms.view = Oreginum::Camera::get_view();
	uniforms.model = glm::translate(translation);
}

void Tetra::Render_Group::draw(const Oreginum::Vulkan::Command_Buffer& command_buffer)
{
	command_buffer.get().bindVertexBuffers(0, vertex_buffer.get(), {0});
	command_buffer.get().bindIndexBuffer(index_buffer.get(), 0, vk::IndexType::eUint32);
	command_buffer.get().drawIndexed(index_count, 1, 0, 0, 0);
}