#include <Include/Core/flcrm_arena_alloc.h>

#include <memory>
#include <cassert>
#include <stdio.h>

namespace flcrm{

	void Arena::arena_init_allocate_buffer(size_t bufferSize){

		if(m_arena_buffer){
			free(m_arena_buffer);
		}

		m_arena_buffer = malloc(bufferSize);
		memset(m_arena_buffer, 0, bufferSize);

		m_arena_buffer_size = bufferSize;
		m_arrena_current_offset = 0;

	}


	void* Arena::arena_allocate(size_t size, size_t align = FLCRM_ARENA_DEFAULT_ALIGNMENT)
	{

		uintptr_t curr_ptr = (uintptr_t)m_arena_buffer + (uintptr_t)m_arrena_current_offset;

		uintptr_t aligned_ptr = align_forward(curr_ptr, align);
		uintptr_t offset = aligned_ptr - (uintptr_t)m_arena_buffer; // change to relative offset;
		

		// check if there is space left

		if(offset + size > m_arena_buffer_size){
			printf("WARNING! ========  Arena Allocator is out of memory!!!!\n");
			return nullptr;
		}

		// calc new offset
		m_arrena_current_offset = offset + size;

		return (void*)aligned_ptr;
	}

	void Arena::arena_reset_buffer(){

		if(m_arena_buffer){
			memset(m_arena_buffer,0,m_arena_buffer_size);
			m_arrena_current_offset = 0;
		}
	}


	void Arena::arena_free_all(){
		
		if(m_arena_buffer){
			free(m_arena_buffer);
		}
		m_arena_buffer_size = 0;
		m_arrena_current_offset = 0;
	}

	bool Arena::has_enough_memory(size_t size){

		if(m_arrena_current_offset + size > m_arena_buffer_size){
			return false;
		}

		return true;
	}

	size_t Arena::bytes_left()
	{
		return m_arena_buffer_size - m_arrena_current_offset;
	}

// private

	bool Arena::is_alignment_power_of_two(uintptr_t x){
		return (x & (x-1)) == 0;
	}

	uintptr_t Arena::align_forward(uintptr_t ptr, size_t align){

		uintptr_t p, a, modolo;

		assert(is_alignment_power_of_two(align));

		p = ptr;
		a = (uintptr_t) align;

		// same as p% a but faster as 'a' is a power of 2
		modolo = p & (a-1);

		if(modolo != 0){
			p += a - modolo;
		}

		return p;
	}

}