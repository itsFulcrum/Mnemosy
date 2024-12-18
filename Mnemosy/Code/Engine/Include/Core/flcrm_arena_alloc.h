#ifndef FLCRM_ARENA_ALLOC_H
#define FLCRM_ARENA_ALLOC_H

#include <stdint.h>

#ifndef FLCRM_ARENA_DEFAULT_ALIGNMENT
#define FLCRM_ARENA_DEFAULT_ALIGNMENT (2*sizeof(void*))
#endif

namespace flcrm{

	class Arena
	{
	public:
		Arena() = default;
		~Arena() = default;

		// allocate a new memory buffer
		void arena_init_allocate_buffer(size_t bufferSize);

		// get a pointer to the next free piece of aligned memory in the arena buffer
		// returns null, if arena is out of memory
		void* arena_allocate(size_t size, size_t align = FLCRM_ARENA_DEFAULT_ALIGNMENT);
		
		// memset the entire arena buffer to 0.
		void arena_memset_zero();
		
		// free the entire arena memory buffer for real
		void arena_free_all();

		// check if arena has enough memory left in the buffer
		bool arena_has_enough_memory(size_t size);

		// get how much memory is left in the arena buffer in bytes.
		size_t arena_get_bytes_left();

	private:
		bool is_alignment_power_of_two(uintptr_t x);
		uintptr_t align_forward(uintptr_t ptr, size_t align);

		void* m_arena_buffer = nullptr;
		size_t m_arena_buffer_size = 0;
		size_t m_arrena_current_offset = 0;		
	};




}

#endif // FLCRM_ARENA_ALLOC_H