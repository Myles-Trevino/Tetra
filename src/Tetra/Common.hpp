#pragma once
#include <array>
#include <vector>
#include <GLM/glm.hpp>

namespace Tetra
{
	enum Materials{STONE = 1, DIRT, GRASS, SAND, WOOD, LEAVES, WATER};
	enum Axis{X, Y, Z};
	enum Faces{RIGHT, LEFT, TOP, BOTTOM, FRONT, BACK};

	static constexpr uint32_t SEED{666};
	const glm::uvec3 WORLD_SIZE{8, 2, 8};
	constexpr uint8_t CHUNK_SIZE{128}, CUBE_FACES{6}, THREADS{4}, CHUNKS_ADDED_PER_FRAME{1};
	constexpr uint32_t CHUNK_SIZE_CUBED{CHUNK_SIZE*CHUNK_SIZE*CHUNK_SIZE};
	static constexpr float VOXEL_SIZE{1.f};

	struct Voxel{ uint8_t cull_mask, material; };

	static const glm::u8vec3 TREE_SIZE{5, 7, 5};
	static constexpr uint8_t TREE[7][5][5]
	{
		{{0, 0, 0, 0, 0},
		{0, 0, 0, 0, 0},
		{0, 0, 5, 0, 0},
		{0, 0, 0, 0, 0},
		{0, 0, 0, 0, 0}},

		{{0, 0, 0, 0, 0},
		{0, 0, 0, 0, 0},
		{0, 0, 5, 0, 0},
		{0, 0, 0, 0, 0},
		{0, 0, 0, 0, 0}},

		{{0, 6, 6, 6, 0},
		{6, 6, 6, 6, 6},
		{6, 6, 5, 6, 6},
		{6, 6, 6, 6, 6},
		{0, 6, 6, 6, 0}},

		{{0, 0, 6, 0, 0},
		{0, 6, 6, 6, 0},
		{6, 6, 5, 6, 6},
		{0, 6, 6, 6, 0},
		{0, 0, 6, 0, 0}},

		{{0, 0, 0, 0, 0},
		{0, 6, 6, 6, 0},
		{0, 6, 5, 6, 0},
		{0, 6, 6, 6, 0},
		{0, 0, 0, 0, 0}},

		{{0, 0, 0, 0, 0},
		{0, 0, 6, 0, 0},
		{0, 6, 5, 6, 0},
		{0, 0, 6, 0, 0},
		{0, 0, 0, 0, 0}},

		{{0, 0, 0, 0, 0},
		{0, 0, 0, 0, 0},
		{0, 0, 6, 0, 0},
		{0, 0, 0, 0, 0},
		{0, 0, 0, 0, 0}},
	};
}