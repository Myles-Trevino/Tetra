#pragma once
#include "Common.hpp"
#include "Chunk.hpp"

namespace Tetra
{
	class World
	{
	public:
		World(glm::u8vec3 size = WORLD_SIZE);
		~World();
		void move(uint8_t axis, bool sign);
		void update();

		const glm::u8vec3& get_size() const { return SIZE; }

	private:
		

		const glm::u8vec3 SIZE;
		const uint32_t SIZE_CUBED;
		glm::ivec3 offset;
		glm::fvec3 translation;
		Chunk **chunks;
		std::vector<std::pair<Chunk *, uint8_t>> deletion_queue;
		std::vector<glm::u8vec3> creation_queue;
		std::vector<Chunk *> add_queue;
		std::thread threads[THREADS];
		bool is_thread_busy[THREADS];
		bool was_thread_launched[THREADS];
		bool populated, meshed;
		std::mutex add_queue_mutex, deletion_queue_mutex;

		void population_pass_1(Chunk *chunk, uint8_t thread_index);
		void population_pass_2(Chunk *chunk, uint8_t thread_index);
		void mesh_chunk(Chunk *chunk, uint8_t thread_index);
		Chunk *get_population_pass_1_chunk();
		Chunk *get_population_pass_2_chunk();
		Chunk *get_unmeshed_chunk();
		int8_t get_thread();
		float *simplex(const glm::ivec3& offset, const glm::ivec3& size,
			float frequency, uint32_t octaves, uint32_t seed);
		bool float_equals(float a, float b, float range){ return abs(a-b) < range; }
		void populate_chunk_pass_1(Chunk *chunk);
		bool axis_bounds_check(glm::u8vec3 values, glm::u8vec3 minimum, glm::u8vec3 maximum);
		void inter_chunk_set(Chunk *chunk, glm::i16vec3 voxel_index, uint8_t material);
		void create_tree(Chunk *chunk, glm::i16vec3 base_voxel_index);
		void populate_chunk_pass_2(Chunk *chunk);
		void transparent_neighbor_cull(Chunk *chunk, const glm::u8vec3& voxel_position);
		void cull_chunk(Chunk *chunk);

		const Chunk& get_chunk(const glm::u8vec3& position)
		{ return *chunks[get_chunk_index(position)]; }
		uint32_t get_chunk_index(const glm::u8vec3& position)
		{ return position.x+position.y*SIZE.x+position.z*SIZE.x*SIZE.y; }
	};
}