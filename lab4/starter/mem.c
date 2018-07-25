/**
 * @file memory.c
 * @brief: ECE254 Lab: memory allocation algorithm comparison template file
 * @author:
 * @date: 2015-11-20
 */

/* includes */
#include <math.h>
#include <limits.h>
#include <stdio.h>
#include <stdlib.h>
#include "mem.h"

/* defines */
#define BLOCK_SIZE 4

/* global varaiables */
memory_list_node_t *worst_fit_block_list;
memory_list_node_t* best_fit_block_list;
void* b_mem_box;

/* Functions */

/* memory initializer */
int best_fit_memory_init(size_t size)
{
	if(size <= sizeof(memory_list_node_t))
		return -1;
	best_fit_block_list = (memory_list_node_t*) malloc(size);
	if(!best_fit_block_list)
		return -1;
	best_fit_block_list->allocated = 0;
	best_fit_block_list->block_size = size - sizeof(memory_list_node_t);
	best_fit_block_list->next = NULL;
	best_fit_block_list->previous = NULL;
	// To be completed by students
	// You call malloc once here to obtain the memory to be managed.
	return 0;
}

int worst_fit_memory_init(size_t size)
{
	if(size <= sizeof(memory_list_node_t))
		return -1;
	worst_fit_block_list = (memory_list_node_t*) malloc(size);
	if(!worst_fit_block_list)
		return -1;
	worst_fit_block_list->allocated = 0;
	worst_fit_block_list->block_size = size - sizeof(memory_list_node_t);
	worst_fit_block_list->next = NULL;
	worst_fit_block_list->previous = NULL;
	// To be completed by students
	// You call malloc once here to obtain the memory to be managed.
	return 0;

}

/* memory allocators */
void *best_fit_alloc(size_t size)
{
	int min_available_block_size = INT_MAX;
	int req_block_size;
	memory_list_node_t* iterator;
	memory_list_node_t* temp_min_node;
	if(size <= 0)
		return NULL;
	if(!best_fit_block_list)
		return NULL;
	if(size%4 != 0)
		size = size + (4 - size%4);
  req_block_size = size + sizeof(memory_list_node_t);
	iterator = best_fit_block_list;
  while(iterator != NULL)
	{
		if(iterator->block_size > req_block_size && iterator->allocated == 0 && iterator->block_size < min_available_block_size)
		{
			min_available_block_size = iterator->block_size;
			temp_min_node = iterator;
		}
		iterator = iterator->next;
	}
  if(min_available_block_size == INT_MAX)
		return NULL;
  else if(req_block_size == min_available_block_size)
	{
		temp_min_node->allocated = 1;
		return temp_min_node;
	}
  else
	{
		memory_list_node_t* new_node = (memory_list_node_t*)(temp_min_node + size);
		new_node->allocated = 0;
		new_node->block_size = temp_min_node->block_size - size - sizeof(memory_list_node_t);
		temp_min_node->allocated = 1;
		temp_min_node->block_size -= size;
		new_node->next = temp_min_node->next;
		if(new_node->next != NULL)
			new_node->next->previous = new_node;
		temp_min_node->next = new_node;
		new_node->previous = temp_min_node;
		return new_node;
	}
}


void *worst_fit_alloc(size_t size)
{
	// To be completed by students
	int max_available_block_size = 0;
	int req_block_size;
	memory_list_node_t* iterator;
	memory_list_node_t* temp_max_node;
	if(size <= 0)
		return NULL;
	if(!worst_fit_block_list)
		return NULL;
	if(size%4 != 0)
		size = size + (4 - size%4);
	req_block_size = size + sizeof(memory_list_node_t);
	iterator = worst_fit_block_list;
	while(iterator != NULL)
	{
		if(iterator->block_size > max_available_block_size && iterator->allocated == 0)
		{
			max_available_block_size = iterator->block_size;
			temp_max_node = iterator;
		}
		iterator = iterator->next;
	}
	if(max_available_block_size == size)
	{
		temp_max_node->allocated = 1;
		return (void*)(temp_max_node + sizeof(memory_list_node_t));
	}
	else if(max_available_block_size < req_block_size)
	{
		return NULL;
	}
	else
	{
		memory_list_node_t* new_node = (memory_list_node_t*)(temp_max_node + sizeof(memory_list_node_t) + size);
		new_node->allocated = 0;
		new_node->block_size = temp_max_node->block_size - size - sizeof(memory_list_node_t);
		temp_max_node->allocated = 1;
		temp_max_node->block_size = size;
		new_node->next = temp_max_node->next;
		if(new_node->next != NULL)
			new_node->next->previous = new_node;
		temp_max_node->next = new_node;
		new_node->previous = temp_max_node;
		return (void*)(new_node + sizeof(memory_list_node_t));
	}
}

/* memory de-allocator */
void best_fit_dealloc(void *ptr)
{
	memory_list_node_t* dealloc_node = (memory_list_node_t*)ptr;
	memory_list_node_t* prev = dealloc_node->previous;
	memory_list_node_t* next = dealloc_node->next;
	if(prev != NULL && prev->allocated == 0)
	{
		prev->block_size += dealloc_node->block_size + sizeof(memory_list_node_t);
		if(next != NULL && next->allocated == 0)
		{
			prev->block_size += next->block_size + sizeof(memory_list_node_t);
			if(next->next)
				next->next->previous = prev;
			prev->next = next->next;
		}
		else
		{
			next->previous = prev;
			prev->next = next;
		}
	}
	else if(next != NULL && next->allocated == 0)
	{
		dealloc_node->block_size += next->block_size + sizeof(memory_list_node_t);
		dealloc_node->next = next->next;
		if(next->next)
			next->next->previous = dealloc_node;
	}
	else
	{
		dealloc_node->allocated = 0;
	}
	return;
}

void worst_fit_dealloc(void *ptr)
{
	memory_list_node_t* dealloc_node = (memory_list_node_t*)(ptr - sizeof(memory_list_node_t));
	memory_list_node_t* prev = dealloc_node->previous;
	memory_list_node_t* next = dealloc_node->next;
	if(prev != NULL && prev->allocated == 0)
	{
		prev->block_size += dealloc_node->block_size + sizeof(memory_list_node_t);
		if(next != NULL && next->allocated == 0)
		{
			prev->block_size += next->block_size + sizeof(memory_list_node_t);
			if(next->next != NULL)
				next->next->previous = prev;
			prev->next = next->next;
		}
		else
		{
			next->previous = prev;
			prev->next = next;
		}
	}
	else if(next != NULL && next->allocated == 0)
	{
		dealloc_node->block_size += next->block_size + sizeof(memory_list_node_t);
		dealloc_node->next = next->next;
		if(next->next != NULL)
			next->next->previous = dealloc_node;
	}
	else
	{
		dealloc_node->allocated = 0;
	}
	// To be completed by students
	return;
}

/* memory algorithm metric utility function(s) */

/* count how many free blocks are less than the input size */
int best_fit_count_extfrag(size_t size)
{
	if(size <= 0)
		return -1;
	int count = 0; 
	memory_list_node_t* iterator = best_fit_block_list;
	while(iterator != NULL)
	{
		if(iterator->allocated == 0 && iterator->block_size < size)
			count++;
		iterator = iterator->next;
	}
	// To be completed by students
	return count;
}

int worst_fit_count_extfrag(size_t size)
{
	if(size <= 0)
		return -1;
	int count = 0;
	memory_list_node_t* iterator = worst_fit_block_list;
	while(iterator != NULL)
	{
		if(iterator->allocated == 0 && iterator->block_size < size)
		{
			count++;
		}
		iterator = iterator->next;
	}
	// To be completed by students
	return count;
}
