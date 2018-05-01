#include "FastNoiseSIMD/FastNoiseSIMD.h"
#include "World.hpp"

Tetra::World::World(glm::u8vec3 size) : SIZE(size), SIZE_CUBED(size.x*size.y*size.z),
	translation(-glm::fvec3{size.x/2.f, size.y-1 ? size.y-1 : .5f, size.z/2.f}*
		static_cast<float>(CHUNK_SIZE))
{
	//Allocate chunks
	chunks = new Chunk *[SIZE_CUBED];
	for(uint8_t z{}; z < size.z; ++z)
		for(uint8_t y{}; y < size.y; ++y)
			for(uint8_t x{}; x < size.x; ++x)
				chunks[get_chunk_index({x, y, z})] = new Chunk{glm::fvec3{x, y, z}*
				static_cast<float>(CHUNK_SIZE), translation, {x, y, z}};

	//Initial world creation
	while(!populated) update();
}

Tetra::World::~World()
{
	for(uint8_t i{}; i < THREADS; ++i) if(was_thread_launched[i]) threads[i].join();
	for(auto& d : deletion_queue) delete d.first;
	for(uint32_t i{}; i < SIZE_CUBED; ++i) if(chunks[i]) delete chunks[i];
	if(chunks) delete[] chunks;
}

void Tetra::World::move(uint8_t axis, bool sign)
{
	offset[axis] += sign ? CHUNK_SIZE : -CHUNK_SIZE;
	std::vector<Chunk *> new_deletion_queue;

	//Move and create pointers and assign to appropriate queue
	for(int16_t z{}; z < SIZE.z; ++z)
		for(uint8_t y{}; y < SIZE.y; ++y)
			for(uint8_t x{}; x < SIZE.x; ++x)
			{
				glm::u8vec3 index{x, y, z};
				uint32_t chunk_index{get_chunk_index(index)};

				//If the chunk is at the created edge of the world
				if(sign ? index[axis] == SIZE[axis]-1 : !index[axis])
				{
					//Make a new chunk and emplace it for culling
					chunks[chunk_index] = new Chunk{glm::fvec3{x, y, z}*
						static_cast<float>(CHUNK_SIZE), translation, {x, y, z}};
					continue;
				}

				glm::i16vec3 adjacent_index{index};
				adjacent_index[axis] += sign ? 1 : -1;
				uint32_t adjacent_chunk_index{get_chunk_index(adjacent_index)};

				//If the chunk is at the deleted edge of the world,
				//emplace it in the deletion queue, and change pointer
				if(sign ? !index[axis] : (index[axis] == SIZE[axis]-1))
				{
					new_deletion_queue.emplace_back(chunks[chunk_index]);
					chunks[chunk_index] = chunks[adjacent_chunk_index];
					continue;
				}
					
				//If inside the world, change pointer
				chunks[chunk_index] = chunks[adjacent_chunk_index];
			}

	{
		//Add newly deleted chunks to main deletion queue
		std::lock_guard<std::mutex> deletion_queue_guard{deletion_queue_mutex};
		for(Chunk *c : new_deletion_queue)
		{
			c->set_being_deleted(true);
			deletion_queue.emplace_back(c, 0);
		}
	}

	//Remove newly deleted chunks
	for(Chunk *c : new_deletion_queue) c->remove_render_groups();

	//Translate
	glm::fvec3 translation{};
	translation[axis] = sign ? -1.f : 1.f;
	for(int16_t z{}; z < SIZE.z; ++z)
		for(uint8_t y{}; y < SIZE.y; ++y)
			for(uint8_t x{}; x < SIZE.x; ++x)
			{
				glm::u8vec3 index{x, y, z};
				if(sign ? index[axis] == SIZE[axis]-1 : !index[axis]) continue;
				uint32_t chunk_index{get_chunk_index(index)};
				chunks[chunk_index]->translate(translation*
					static_cast<float>(CHUNK_SIZE), translation);
			}
}

void Tetra::World::population_pass_1(Chunk *chunk, uint8_t thread_index)
{
	populate_chunk_pass_1(chunk);

	chunk->set_populated(0, true);
	chunk->set_being_created(false);

	is_thread_busy[thread_index] = false;
}

void Tetra::World::population_pass_2(Chunk *chunk, uint8_t thread_index)
{
	populate_chunk_pass_2(chunk);

	//Set all chunks in column to populated and not being created
	glm::u8vec3 index{chunk->get_index()};
	for(uint8_t y{}; y < SIZE.y; ++y)
	{
		Chunk *column_chunk{chunks[get_chunk_index({index.x, y, index.z})]};
		column_chunk->set_populated(1, true);
		column_chunk->set_being_created(false);
	}

	is_thread_busy[thread_index] = false;
}

void Tetra::World::mesh_chunk(Chunk *chunk, uint8_t thread_index)
{
	//Cull, and mesh
	cull_chunk(chunk);
	chunk->create_mesh();

	//Emplace in add queue
	{
		std::lock_guard<std::mutex> guard{add_queue_mutex};
		add_queue.emplace_back(chunk);
	}

	chunk->set_meshed(true);
	chunk->set_being_created(false);
	is_thread_busy[thread_index] = false;
}

Tetra::Chunk *Tetra::World::get_population_pass_1_chunk()
{
	for(uint8_t y{}; y < SIZE.y; ++y)
		for(uint8_t z{}; z < SIZE.z; ++z)
			for(uint8_t x{}; x < SIZE.x; ++x)
			{
				Chunk *chunk{chunks[get_chunk_index({x, y, z})]};
				if(!chunk->is_populated(0) && !chunk->is_being_created())
					return chunk;
			}
	return nullptr;
}

Tetra::Chunk *Tetra::World::get_population_pass_2_chunk()
{
	//Search world by column
	for(uint8_t x{}; x < SIZE.x; ++x)
		for(uint8_t z{}; z < SIZE.z; ++z)
			for(uint8_t y{}; y < SIZE.y; ++y)
			{
				//If chunk has not had the second population pass
				Chunk *chunk{chunks[get_chunk_index({x, y, z})]};
				if(!chunk->is_populated(1) && !chunk->is_being_created())
				{
					//Make sure no chunks in the chunk's row are being created
					bool free{true};
					for(uint8_t y_2{}; y_2 < SIZE.y; ++y_2)
					{
						Chunk *chunk_2{chunks[get_chunk_index({x, y_2, z})]};
						if(chunk->is_being_created()){ free = false; break; }
					}

					//If no chunks in the row are being created, return the chunk
					if(free) return chunk;
				}
			}
	return nullptr;
}

Tetra::Chunk *Tetra::World::get_unmeshed_chunk()
{
	//Get closest chunk that is unmeshed
	Chunk *result{nullptr};
	glm::u8vec3 half_size{SIZE/glm::u8vec3{2}};
	glm::u8vec3 result_difference{UINT8_MAX};
	uint32_t result_sum{UINT32_MAX};
	glm::u8vec3 potential_difference{};

	for(uint8_t x{}; x < SIZE.x; ++x)
	{
		potential_difference.x = abs(half_size.x-x);
		for(uint8_t y{}; y < SIZE.y; ++y)
		{
			potential_difference.y = abs(half_size.y-y);
			for(uint8_t z{}; z < SIZE.z; ++z)
			{
				potential_difference.z = abs(half_size.z-z);
				uint32_t sum{static_cast<uint32_t>(potential_difference.x+
					potential_difference.y+potential_difference.z)};

				if(sum < result_sum)
				{
					uint32_t index{get_chunk_index({x, y, z})};
					if(!chunks[index]->is_meshed() && !chunks[index]->is_being_created())
					{
						result = chunks[get_chunk_index({x, y, z})];
						result_sum = sum;
					}
				}
			}
		}
	}

	return result;
}

int8_t Tetra::World::get_thread()
{
	//Get thread
	int8_t thread_index{-1};
	for(uint8_t j{}; j < THREADS; ++j) if(!is_thread_busy[j]) thread_index = j;
	if(thread_index == -1) return -1;

	//Check and prepare chunk
	is_thread_busy[thread_index] = true;
	if(was_thread_launched[thread_index]) threads[thread_index].join();
	else was_thread_launched[thread_index] = true;
	return thread_index;

	return -1;
}

void Tetra::World::update()
{
	//Wait 3 frames to ensure buffers aren't in use, then delete enqueued chunks
	if(deletion_queue.size())
	{
		std::lock_guard<std::mutex> deletion_queue_guard{deletion_queue_mutex};
		std::lock_guard<std::mutex> render_guard{
			*Oreginum::Renderer_Core::get_render_mutex()};
		for(uint32_t i{}; i < deletion_queue.size(); ++i)
		{
			if(deletion_queue[i].second > 5)
			{
				if(!deletion_queue[i].first->is_being_created())
				{
					for(uint32_t j{}; j < add_queue.size(); ++j)
						if(add_queue[j] == deletion_queue[i].first)
							add_queue.erase(add_queue.begin()+j);
					delete deletion_queue[i].first;
					deletion_queue.erase(deletion_queue.begin()+i);
				}
			} else ++deletion_queue[i].second;
		}
	}

	//Create chunk render groups
	for(uint8_t i{}; i < CHUNKS_ADDED_PER_FRAME; ++i)
	{
		if(add_queue.size())
		{
			if(!add_queue.front()->is_being_deleted()) 
			{
				add_queue.front()->create_render_groups();
				add_queue.front()->add_render_groups();
			}
			add_queue.erase(add_queue.begin());
		} else break;
	}

	//First pass population
	Chunk *population_pass_1_chunk{nullptr};
	while(true)
	{
		population_pass_1_chunk = get_population_pass_1_chunk();
		if(!population_pass_1_chunk) break;

		int8_t thread_index{get_thread()};
		if(thread_index == -1) break;

		population_pass_1_chunk->set_being_created(true);
		threads[thread_index] = std::thread{&World::population_pass_1,
			this, population_pass_1_chunk, thread_index};
	}

	//If first pass population has completed, do second population pass
	Chunk *population_pass_2_chunk{nullptr};
	while(!population_pass_1_chunk)
	{
		population_pass_2_chunk = get_population_pass_2_chunk();
		if(!population_pass_2_chunk) break;

		int8_t thread_index{get_thread()};
		if(thread_index == -1) break;

		//Set all chunks in column as being created
		glm::u8vec3 index{population_pass_2_chunk->get_index()};
		for(uint8_t y{}; y < SIZE.y; ++y)
		{
			Chunk *column_chunk{chunks[get_chunk_index({index.x, y, index.z})]};
			column_chunk->set_being_created(true);
		}

		threads[thread_index] = std::thread{&World::population_pass_2,
			this, population_pass_2_chunk, thread_index};
	}

	populated = !population_pass_1_chunk && !population_pass_2_chunk;

	//If population has completed, cull and mesh unmeshed
	//chunks, then add them to the add queue
	Chunk *unmeshed_chunk{nullptr};
	while(populated)
	{
		unmeshed_chunk = get_unmeshed_chunk();
		if(!unmeshed_chunk) break;

		int8_t thread_index{get_thread()};
		if(thread_index == -1) break;

		unmeshed_chunk->set_being_created(true);
		threads[thread_index] = std::thread{&World::mesh_chunk,
			this, unmeshed_chunk, thread_index};
	}

	meshed = !unmeshed_chunk;
}

float *Tetra::World::simplex(const glm::ivec3& offset, const glm::ivec3& size,
	float frequency, uint32_t octaves, uint32_t seed)
{
	FastNoiseSIMD *generator{FastNoiseSIMD::NewFastNoiseSIMD(seed)};
	generator->SetFrequency(frequency);
	generator->SetFractalOctaves(octaves);
	return generator->GetSimplexFractalSet(offset.x,
		offset.y, offset.z, size.x, size.y, size.z);
}

void Tetra::World::populate_chunk_pass_1(Chunk *chunk)
{
	const glm::ivec3 CHUNK_OFFSET{offset.x+chunk->get_translation().x,
		offset.y+chunk->get_translation().y, offset.z+chunk->get_translation().z},
		OFFSET_2D{CHUNK_OFFSET.z, CHUNK_OFFSET.x, 0}, SIZE_2D{CHUNK_SIZE, CHUNK_SIZE, 1};
	constexpr uint8_t RISES_BASES_HEIGHT{20}, EARTH_RANGE{50}, MOUNTAINOUSNESS_RANGE{200};

	float *mountainousness_set{simplex(OFFSET_2D, SIZE_2D, .003f, 7, SEED)};

	float *earth_set{simplex(OFFSET_2D, SIZE_2D, .0005f, 1, SEED+1)};
	float *hills_set{simplex(OFFSET_2D, SIZE_2D, .01f, 2, SEED+2)};
	float *detail_set{simplex(OFFSET_2D, SIZE_2D, .01f, 1, SEED+3)};
	float *plateau_fill_set{simplex({CHUNK_OFFSET.z, CHUNK_OFFSET.x, CHUNK_OFFSET.y},
		{CHUNK_SIZE, CHUNK_SIZE, CHUNK_SIZE}, .002f, 7, SEED+4)};
	float *plateau_height_set{simplex(OFFSET_2D, SIZE_2D, .003f, 2, SEED+5)};
		
	//Create ground
	uint32_t noise_index_2d{}, noise_index_3d{};
	for(uint8_t z{}; z < CHUNK_SIZE; ++z)
		for(uint8_t x{}; x < CHUNK_SIZE; ++x)
		{
			for(uint8_t y{}; y < CHUNK_SIZE; ++y)
			{
				const float MOUNTAINOUSNESS{std::max(
					mountainousness_set[noise_index_2d]*
					MOUNTAINOUSNESS_RANGE, 0.f)};

				float VOXEL_Y{offset.y+translation.y+chunk->get_translation().y+y};

				const float EARTH{earth_set[noise_index_2d]*EARTH_RANGE};
				const float HILLS{hills_set[noise_index_2d]*5*MOUNTAINOUSNESS/30};
				const float DETAIL{detail_set[noise_index_2d]*2};
				const float GROUND{EARTH+HILLS+DETAIL};

				const float PLATEAU_HEIGHT{EARTH+plateau_height_set[noise_index_2d]*
					MOUNTAINOUSNESS-RISES_BASES_HEIGHT+DETAIL};
				const float PLATEAU{VOXEL_Y > PLATEAU_HEIGHT ?
					plateau_fill_set[noise_index_3d]*(VOXEL_Y-PLATEAU_HEIGHT) : 0};

				if(VOXEL_Y > GROUND || PLATEAU > .1)
					chunk->set_voxel_material({x, y, z}, Materials::STONE);

				++noise_index_3d;
			}
			++noise_index_2d;
		}

	FastNoiseSIMD::FreeNoiseSet(plateau_height_set);
	FastNoiseSIMD::FreeNoiseSet(plateau_fill_set);
	FastNoiseSIMD::FreeNoiseSet(detail_set);
	FastNoiseSIMD::FreeNoiseSet(hills_set);
	FastNoiseSIMD::FreeNoiseSet(earth_set);
	FastNoiseSIMD::FreeNoiseSet(mountainousness_set);
}

bool Tetra::World::axis_bounds_check(glm::u8vec3 values, glm::u8vec3 minimum, glm::u8vec3 maximum)
{
	for(uint8_t axis{}; axis < 3; ++axis)
		if(values[axis] < minimum[axis] || values[axis] > maximum[axis]) return true;
	return false;
}

void Tetra::World::inter_chunk_set(Chunk *chunk, glm::i16vec3 voxel_index, uint8_t material)
{
	//If outside chunk
	if(axis_bounds_check(voxel_index, glm::u8vec3{0}, glm::u8vec3{CHUNK_SIZE-1}))
	{
		//Get new chunk index
		glm::i16vec3 chunk_index{chunk->get_index()};
		for(uint8_t axis{}; axis < 3; ++axis)
			if(voxel_index[axis] < 0) --chunk_index[axis],
				voxel_index[axis] = static_cast<int16_t>(CHUNK_SIZE)+voxel_index[axis];
			else if(voxel_index[axis] > CHUNK_SIZE-1) ++chunk_index[axis],
				voxel_index[axis] = voxel_index[axis]- static_cast<int16_t>(CHUNK_SIZE);

		//If outside world
		if(axis_bounds_check(chunk_index, glm::u8vec3{0}, SIZE-glm::u8vec3{1, 1, 1})){}

		//If inside world
		else chunks[get_chunk_index(chunk_index)]->set_voxel_material(voxel_index, material);
	}

	//If inside chunk
	else chunk->set_voxel_material(voxel_index, material);
}

void Tetra::World::create_tree(Chunk *chunk, glm::i16vec3 base_voxel_index)
{
	base_voxel_index -= glm::i16vec3{TREE_SIZE.x/2, 1, TREE_SIZE.z/2};

	for(int8_t y{}; y < TREE_SIZE.y; ++y)
		for(int8_t x{}; x < TREE_SIZE.x; ++x)
			for(int8_t z{}; z < TREE_SIZE.z; ++z)
			{
				glm::i16vec3 voxel_index{base_voxel_index};
				voxel_index += glm::i16vec3{x, -y, z};
				uint8_t material{TREE[y][z][x]};
				if(!material) continue;
				inter_chunk_set(chunk, voxel_index, material);
			}
}

void Tetra::World::populate_chunk_pass_2(Chunk *chunk)
{
	const glm::ivec3 CHUNK_OFFSET{offset.x+chunk->get_translation().x,
		offset.y+chunk->get_translation().y, offset.z+chunk->get_translation().z},
		OFFSET_2D{CHUNK_OFFSET.z, CHUNK_OFFSET.x, 0}, SIZE_2D{CHUNK_SIZE, CHUNK_SIZE, 1};

	float *tree_area_set{simplex(OFFSET_2D, SIZE_2D, .003f, 5, SEED+6)};

	//Iterate though the world by voxel column to set ground layers
	glm::u8vec3 index{chunk->get_index()};
	uint16_t relative_depth{};
	uint32_t noise_index_2d{};
	for(uint8_t voxel_z{}; voxel_z < CHUNK_SIZE; ++voxel_z)
	for(uint8_t voxel_x{}; voxel_x < CHUNK_SIZE; ++voxel_x)
	{
		for(uint8_t chunk_y{}; chunk_y < SIZE.y; ++chunk_y)
		{
			Chunk *chunk{chunks[get_chunk_index({index.x, chunk_y, index.z})]};

			for(uint8_t voxel_y{}; voxel_y < CHUNK_SIZE; ++voxel_y)
			{
				glm::u8vec3 voxel_index{voxel_x, voxel_y, voxel_z};
				uint8_t voxel_material{chunk->get_voxel_material(voxel_index)};
				float depth{offset.y+translation.y+chunk->get_translation().y+voxel_y};

				//Water
				if(depth > 0 && voxel_material == NULL)
				{
					chunk->set_voxel_material(voxel_index, Materials::WATER);
					voxel_material = chunk->get_voxel_material(voxel_index);
					relative_depth = 1;
				}

				//Sand
				else if(depth > -3 && depth < 6 && voxel_material == Materials::STONE)
				{
					chunk->set_voxel_material(voxel_index, Materials::SAND);
					++relative_depth;
				}

				//Grass and dirt
				else if(voxel_material == Materials::STONE)
				{
					if(relative_depth == 0)
						chunk->set_voxel_material(voxel_index, Materials::GRASS);
					else if(relative_depth > 0 && relative_depth <= 3)
						chunk->set_voxel_material(voxel_index, Materials::DIRT);
					++relative_depth;
				}

				//Trees
				float random{(rand()%2000)/100.f};
				const bool TREE{random < std::max(tree_area_set[noise_index_2d], 0.f)};

				if(chunk->get_voxel_material(voxel_index) ==  Materials::GRASS && TREE)
				{
					create_tree(chunk, voxel_index);
				}

				if(voxel_material == Materials::WATER) relative_depth = 1;
			}
		}
		++noise_index_2d;
		relative_depth = 0;
	}

	FastNoiseSIMD::FreeNoiseSet(tree_area_set);
}

void Tetra::World::transparent_neighbor_cull(Chunk *chunk, const glm::u8vec3& voxel_position)
{
	chunk->set_voxel_culled(voxel_position, true);
	for(int axis{}; axis < 3; ++axis)
		for(int sign{}; sign < 2; ++sign)
		{
			//If at edge of chunk
			if(sign ? !voxel_position[axis] : voxel_position[axis] == CHUNK_SIZE-1)
				chunk->set_voxel_culled(voxel_position, false, axis*2+sign);

			//If inside chunk
			else
			{
				glm::u8vec3 adjacent_voxel_position{voxel_position};
				adjacent_voxel_position[axis] += sign ? -1 : 1;

				if(chunk->is_voxel_transparent(adjacent_voxel_position) &&
					chunk->get_voxel(adjacent_voxel_position).material !=
					chunk->get_voxel(voxel_position).material)
					chunk->set_voxel_culled(voxel_position, false, axis*2+sign);
			}
		}
}
	
void Tetra::World::cull_chunk(Chunk *chunk)
{
	//Cull the chunk's voxels
	for(uint8_t z{}; z < CHUNK_SIZE; ++z)
		for(uint8_t y{}; y < CHUNK_SIZE; ++y)
			for(uint8_t x{}; x < CHUNK_SIZE; ++x)
			{
				//If null material, cull the whole voxel
				if(!chunk->get_voxel({x, y, z}).material)
					chunk->set_voxel_culled({x, y, z}, true);

				//Cull faces if their neigbour is opaque
				else transparent_neighbor_cull(chunk, {x, y, z});
			}

	//Cull the chunk if it only contains culled voxels
	for(uint8_t z{}; z < CHUNK_SIZE; ++z)
		for(uint8_t y{}; y < CHUNK_SIZE; ++y)
			for(uint8_t x{}; x < CHUNK_SIZE; ++x)
				if(!chunk->is_voxel_culled({x, y, z}))
				{ chunk->set_culled(false); return; }
	chunk->set_culled(true);
}