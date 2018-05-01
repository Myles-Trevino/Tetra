#pragma once
#include "Common.hpp"
#include "Render Group.hpp"

namespace Tetra
{
	class Chunk
	{
	public:
		Chunk(const glm::fvec3& translation, const glm::fvec3& world_translation,
			const glm::u8vec3& index);
		~Chunk(){ remove_render_groups(); }

		void create_mesh();
		void create_render_groups();
		void add_render_groups(){ for(Render_Group& r : render_groups) r.add(); }
		void remove_render_groups(){ for(Render_Group& r : render_groups) r.remove(); }

		void translate(const glm::fvec3& translation, const glm::u8vec3& index_translation);

		glm::fvec3 get_translation() const { return translation; }
		glm::u8vec3 get_index() const { return index; }
		uint8_t get_voxel_material(const glm::u8vec3& voxel)
		{ return voxels[voxel.z][voxel.y][voxel.x].material; }
		bool is_culled(){ return culled; }
		bool is_populated(uint8_t pass) const { return populated[pass]; }
		bool is_meshed() const { return meshed; }
		bool is_being_created() const { return being_created; }
		bool is_being_deleted() const { return being_deleted; }
		const Voxel& get_voxel(const glm::u8vec3& voxel) const
		{ return voxels[voxel.z][voxel.y][voxel.x]; }
		bool is_voxel_transparent(const glm::u8vec3& voxel) const
		{
			const uint8_t MATERIAL{voxels[voxel.z][voxel.y][voxel.x].material};
			return MATERIAL == NULL || MATERIAL == WATER;
		}
		bool is_voxel_culled(const glm::u8vec3& voxel)
		{ return voxels[voxel.z][voxel.y][voxel.x].cull_mask < 0b00111111; }

		void set_culled(bool culled){ this->culled = culled; }
		void set_populated(uint8_t pass, bool populated){ this->populated[pass] = populated; }
		void set_meshed(bool meshed){ this->meshed = meshed; }
		void set_being_created(bool being_created){ this->being_created = being_created; }
		void set_being_deleted(bool being_deleted){ this->being_deleted = being_deleted; }
		void set_voxel_material(const glm::u8vec3& voxel, uint8_t material)
		{ voxels[voxel.z][voxel.y][voxel.x].material = material; }
		void set_voxel_culled(const glm::u8vec3& voxel, bool culled, uint8_t face = 6)
		{
			if(face > 5) voxels[voxel.z][voxel.y][voxel.x].cull_mask |= 0b00111111;
			else if(culled) voxels[voxel.z][voxel.y][voxel.x].cull_mask |= 0b1<<face;
			else voxels[voxel.z][voxel.y][voxel.x].cull_mask &= ~(0b1<<face);
		}

	private:
		static constexpr uint8_t VERTEX_ORDERS[3][3]{{2, 1, 0}, {0, 2, 1}, {0, 1, 2}},
			NORMAL_AXIS[3]{0, 1, 2};
		static constexpr std::array<uint16_t, 6> RECTANGLE_INDICES{0, 1, 2, 2, 3, 0};

		struct Mesh_Data
		{
			uint32_t face;
			std::vector<float> vertices;
			std::vector<uint32_t> indices;
		};

		bool culled, populated[2], meshed, being_created, being_deleted;
		Voxel voxels[CHUNK_SIZE][CHUNK_SIZE][CHUNK_SIZE];
		glm::fvec3 translation, world_translation;
		std::vector<Render_Group> render_groups;
		std::unordered_map<uint8_t, Mesh_Data> mesh_datas;
		glm::u8vec3 index;

		void greedy_face(std::unordered_map<uint8_t, Mesh_Data> *mesh_datas,
			uint8_t material, uint32_t *face, uint8_t axis, uint8_t sign,
			const glm::fvec3& position, const glm::fvec2& size);
		Voxel greedy_get(uint8_t axis, uint8_t layer, uint8_t row, uint8_t column);
		uint8_t get_material_type(uint8_t material);
		bool is_face_culled(uint8_t cull_mask, uint8_t face_index)
		{ return cull_mask&(0b1<<face_index); }
		void greedy_main(std::unordered_map<uint8_t, Mesh_Data> *mesh_datas,
			uint32_t *face, uint8_t axis, uint8_t sign, uint8_t layer);
		void greedy_mesh_simplification(std::unordered_map<uint8_t, Mesh_Data> *mesh_datas);
	};
}