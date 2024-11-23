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

		// pre allocate a memory buffer
		void arena_init_allocate_buffer(size_t bufferSize);

		// get a pointer to the next free piece of memory aligned
		// if arena is out of memory return null;
		void* arena_allocate(size_t size, size_t align);
		

		// reset the arena and set all memory in the buffer back to 0 esentially "freeing" it;
		void arena_reset_buffer();
		
		// free the arena memory buffer for real
		void arena_free_all();

		bool has_enough_memory(size_t size);
		size_t bytes_left();

	private:
		bool is_alignment_power_of_two(uintptr_t x);
		uintptr_t align_forward(uintptr_t ptr, size_t align);

		void* m_arena_buffer = nullptr;
		size_t m_arena_buffer_size = 0;
		size_t m_arrena_current_offset = 0;
		
		
	};




}

#endif // FLCRM_ARENA_ALLOC_H