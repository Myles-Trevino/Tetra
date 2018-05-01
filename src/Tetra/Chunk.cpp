#include "Chunk.hpp"

Tetra::Chunk::Chunk(const glm::fvec3& translation, const glm::fvec3& world_translation,
	const glm::u8vec3& index) : translation(translation), 
	world_translation(world_translation), index(index){}

void Tetra::Chunk::greedy_face(std::unordered_map<uint8_t, Mesh_Data> *mesh_datas,
	uint8_t material, uint32_t *face, uint8_t axis, uint8_t sign,
	const glm::fvec3& position, const glm::fvec2& size)
{
	const uint8_t FACE_INDEX{axis*2U+sign};
	const float SIGN_OFFSET{sign ? 0.f : 1.f}, SIGN_DIRECTION{sign ? -1.f : 1.f};

	Mesh_Data *mesh_data{nullptr};
	const uint8_t MATERIAL_TYPE{get_material_type(material)};
	auto iterator{mesh_datas->find(MATERIAL_TYPE)};
	if(iterator == mesh_datas->end())
		mesh_data = &mesh_datas->emplace(MATERIAL_TYPE, Mesh_Data{}).first->second;
	else mesh_data = &iterator->second;

	//Vertices
	const glm::fvec3 V[4]{{position.x, position.y, position.z+SIGN_OFFSET},
		{position.x, position.y+size.y+VOXEL_SIZE, position.z+SIGN_OFFSET},
		{position.x+size.x+VOXEL_SIZE, position.y+size.y+VOXEL_SIZE, position.z+SIGN_OFFSET},
		{position.x+size.x+VOXEL_SIZE, position.y, position.z+SIGN_OFFSET}};
	const glm::fvec2 UV[4]{{0, 0}, {0, size.y+1}, {size.x+1, size.y+1}, {size.x+1, 0}};
	uint8_t j{};
	for(uint8_t i{}; i < 4; ++i)
	{
		for(j = 0; j < 3; ++j) mesh_data->vertices.push_back(V[i][VERTEX_ORDERS[axis][j]]);
		for(j = 0; j < 2; ++j) mesh_data->vertices.push_back(
			UV[axis == Axis::Z ? sign ? i : 3-i : sign ? 3-i : i][j]);
		for(j = 0; j < 3; ++j)
			mesh_data->vertices.push_back(NORMAL_AXIS[axis] == j ? SIGN_DIRECTION : 0);
		mesh_data->vertices.emplace_back(material);
	}

	//Indices
	for(uint32_t i{}; i < RECTANGLE_INDICES.size(); ++i)
	{
		bool backface{axis == Axis::Z ? sign == 0 : sign != 0};
		uint32_t index{RECTANGLE_INDICES[backface ? (RECTANGLE_INDICES.size()-1)-i : i]};
		mesh_data->indices.emplace_back(index+4*mesh_data->face);
	}

	++mesh_data->face;
}

Tetra::Voxel Tetra::Chunk::greedy_get(uint8_t axis, uint8_t layer, uint8_t row, uint8_t column)
{
	return axis == Axis::X ? voxels[column][row][layer] : axis == Axis::Y ?
		voxels[row][layer][column] : voxels[layer][row][column];
}

uint8_t Tetra::Chunk::get_material_type(uint8_t material)
{
	if(!material) return NULL;
	if(material == WATER) return Oreginum::Renderable::Type::VOXEL_TRANSLUCENT;
	else return Oreginum::Renderable::Type::VOXEL;
}

void Tetra::Chunk::greedy_main(std::unordered_map<uint8_t, Mesh_Data> *mesh_datas,
	uint32_t *face, uint8_t axis, uint8_t sign, uint8_t layer)
{
	//Determine face index
	const uint8_t FACE_INDEX{axis*2U+sign};

	//Create meshed mask
	uint8_t x, y{};
	bool meshed[CHUNK_SIZE][CHUNK_SIZE];
	for(; y < CHUNK_SIZE; ++y)
		for(x = 0; x < CHUNK_SIZE; ++x)
			meshed[y][x] = false;

	//While layer is not meshed
	glm::u8vec2 position{}, size;
	Voxel initial_voxel, voxel;
	bool found;
	while(true)
	{
		//Find the initial voxel and position
		found = false;
		for(y = position.y; y < CHUNK_SIZE && !found; ++y)
			for(x = y == position.y ? position.x : 0U;
				x < CHUNK_SIZE && !found; ++x)
				{
					if(meshed[y][x]) continue;
					voxel = greedy_get(axis, layer, y, x);
					if(!is_face_culled(voxel.cull_mask, FACE_INDEX))
						initial_voxel = voxel, position = {x, y}, found = true;
				}
		if(!found) break;

		//Find the width
		size.x = 0;
		for(x = position.x+1U; x < CHUNK_SIZE; ++x)
		{
			voxel = greedy_get(axis, layer, position.y, x);
			if(meshed[position.y][x] || is_face_culled(voxel.cull_mask, FACE_INDEX) ||
				voxel.material != initial_voxel.material || x == CHUNK_SIZE-1)
				{ size.x = (x-1)-position.x; break; }
		}

		//Find the height
		found = false;
		size.y = 0;
		for(y = position.y+1U; y < CHUNK_SIZE && !found; ++y)
			for(x = position.x; x <= position.x+size.x && !found; ++x)
			{
				voxel = greedy_get(axis, layer, y, x);
				if(meshed[y][x] || is_face_culled(voxel.cull_mask, FACE_INDEX) ||
					voxel.material != initial_voxel.material || y == CHUNK_SIZE-1)
					size.y = (y-1)-position.y, found = true;
			}

		//Update meshed mask
		for(y = position.y; y <= position.y+size.y; ++y)
			for(x = position.x; x <= position.x+size.x; ++x)
				meshed[y][x] = true;

		//Create face
		greedy_face(mesh_datas, initial_voxel.material, face,
			axis, sign, glm::fvec3{position, layer}, size);
	}
}

void Tetra::Chunk::greedy_mesh_simplification(std::unordered_map<uint8_t, Mesh_Data> *mesh_datas)
{
	uint32_t face{};
	for(uint8_t axis{}; axis < 3; ++axis)
		for(uint8_t sign{}; sign < 2; ++sign)
			for(uint8_t layer{}; layer < CHUNK_SIZE; ++layer)
				greedy_main(mesh_datas, &face, axis, sign, layer);
}

void Tetra::Chunk::create_mesh()
{
	if(culled) return;
	mesh_datas.clear();
	greedy_mesh_simplification(&mesh_datas);
}

void Tetra::Chunk::create_render_groups()
{
	render_groups.clear();
	for(const auto& m : mesh_datas) render_groups.emplace_back(m.second.vertices,
		m.second.indices, m.first, translation+world_translation);
	mesh_datas.clear();
}

void Tetra::Chunk::translate(const glm::fvec3& translation, const glm::u8vec3& index_translation)
{
	this->translation += translation;
	this->index += index_translation;
	for(Render_Group& r : render_groups) r.translate(translation);
}