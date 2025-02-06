#include <stdio.h>
#include <stddef.h>
#include <unistd.h>
#include <stdlib.h>
#include "my_malloc.h"

#define INCR_VAL 4096

static freeNode_t *first_free_ptr = NULL;
static size_t memory_size = 0;
static void *end_address = NULL; 
static void *start_address = NULL;

//first fit malloc
void *ff_malloc(size_t size){
	void *end_address = NULL; 
  	void *new_address = NULL; 
	int new_increment_size = 0; 
	freeNode_t *node_ptr = NULL;
	void *return_ptr = NULL;

	if(size == 0){
		return NULL;
	}

	//minimum bytes allowed to allocate 
	if((size + sizeof(int)) < sizeof(freeNode_t)){
		size = sizeof(freeNode_t) - sizeof(int);
	}
	
	//check if there is there is no free node using first free pointer	
		//this is happen either on the first time a malloc is called or when you have run out of free space 
  	if(first_free_ptr == NULL){
    	//call sbrk() -- persistent memory
    	start_address = sbrk(0); 
		if (start_address == (void*) -1) {
			exit(EXIT_FAILURE);
		}
		//printf("sbrk 1 = %p\n", start_address);

		//check if 4092 is big enough to hold requested memory 
		if(INCR_VAL > size + sizeof(int)){
			new_increment_size = INCR_VAL; 
		}else{
			new_increment_size = size*2; 
			// new_increment_size = (size + sizeof(int))*2; 
		}
		//create heap with incremental size 
		new_address = sbrk(new_increment_size);
		if (new_address == (void*) -1) {
			exit(EXIT_FAILURE);
		}
		//if(new_address = ())
		//printf("sbrk 2 = %p\n", new_address);
		memory_size += new_increment_size;

		// new end address after allocating 
		end_address = sbrk(0);
		if (end_address == (void*) -1) {
			exit(EXIT_FAILURE);
		}
		//printf("sbrk 3 = %p\n", end_address);

		first_free_ptr = (freeNode_t *)new_address;
		node_ptr = (freeNode_t *)first_free_ptr;

		node_ptr->length = new_increment_size;
		node_ptr->next_ptr = NULL;
			//node_ptr->free = 100;
  	}

	int found = 0;
	freeNode_t *curr_first_free = first_free_ptr;
	freeNode_t *previous_free_node = NULL;
	freeNode_t *prev_previous_free_node = NULL;
	
	//loop through all free spaces and find first free region that fits requested size + sizeof(int) <-- used to keep length of used block
	while(found == 0 && curr_first_free != NULL){
		//if size is too small, go to next free block
		if(curr_first_free->length < (size + sizeof(int))){
			prev_previous_free_node = previous_free_node;
			previous_free_node = curr_first_free;
			curr_first_free = curr_first_free->next_ptr;
		}else{
			found = 1;
		}
	}

	//condition: there is no suitable free region of memory for requested size 
 	if(found == 0){
		void *prev_end_address = end_address;
		freeNode_t *new_node = NULL;
		
		new_increment_size = size + sizeof(int);
		//increment heap and store new address of memory
		new_address = sbrk(new_increment_size);
		if (new_address == (void*) -1) {
			exit(EXIT_FAILURE);
		}
		//printf("sbrk 4 = %p\n", new_address);
		memory_size += new_increment_size;

		//get the new end address after allocating
		end_address = sbrk(0);
		if (end_address == (void*) -1) {
			exit(EXIT_FAILURE);
		}
		//printf("sbrk 5 = %p\n", end_address);
		
		//check if the previous free region + length equals new address to combine free regions
		if(previous_free_node != NULL && ((char *)previous_free_node + previous_free_node->length) == new_address){
			//combining
			new_node = (freeNode_t *)((char *)previous_free_node + sizeof(int) + size);
			new_node->length = previous_free_node->length + new_increment_size - sizeof(int) - size;
			new_node->next_ptr = NULL;
			//new_node->free = 100;

			previous_free_node->next_ptr = NULL;

			int *ptr_int = (int *)previous_free_node;
			*ptr_int = size + sizeof(int);
			return_ptr = (char *)ptr_int + sizeof(int);
			
			//check if first free pointer needs to be updates 
			if(prev_previous_free_node != NULL){
				prev_previous_free_node->next_ptr = new_node;
			}else{
				first_free_ptr = new_node;
			}

		}else {
			//dont need to combine and add new node and new free region to heap 
			// new_node = (freeNode_t *)((char *)new_address + size + sizeof(int));
			// new_node->length = new_increment_size - size - sizeof(int);
			// new_node->next_ptr = NULL;
			// previous_free_node->next_ptr = new_node;

			int *ptr_int = new_address;

			*ptr_int = size + sizeof(int);
			return_ptr = (char *)ptr_int + sizeof(int);
		}
	
	}else if(found == 1 && (curr_first_free == first_free_ptr)){
		// if(curr_first_free->free != 100){
		// 	exit(EXIT_FAILURE);
		// }
		//a free block was found and it was the first free region
		freeNode_t *new_node = NULL;
		if(curr_first_free->length > (size + sizeof(int))){ //leftover space in free block
			int total_size_of_node = 0;

			//check that leftover space is big enough to hold metadata
			if((curr_first_free->length - size - sizeof(int)) < sizeof(freeNode_t)){ 
				//not big enough for metadata --> store leftover memory with malloc call
				first_free_ptr = curr_first_free->next_ptr;

				int *ptr_int = (int *)curr_first_free;
				*ptr_int = curr_first_free->length;
				return_ptr = (char *)ptr_int + sizeof(int);									
			}else{ 
				//enough leftover memory --> create new free region
				total_size_of_node = curr_first_free->length - size - sizeof(int);

				new_node = (freeNode_t *)((char *)curr_first_free + size + sizeof(int));
				new_node->length = total_size_of_node;
				new_node->next_ptr = curr_first_free->next_ptr;
				//new_node->free = 100;

				curr_first_free->next_ptr = NULL;

				int *ptr_int = (int *)curr_first_free;
				*ptr_int = size + sizeof(int);
				return_ptr = (char *)ptr_int + sizeof(int);
				first_free_ptr = new_node;
			}
		}else{ //size + sizeof = first free length 
			//no leftover space in free region -- perfect fit 
			first_free_ptr = first_free_ptr->next_ptr;

			curr_first_free->next_ptr = NULL;

			//possible error
			int *ptr_int = (int *)curr_first_free;
			*ptr_int = size + sizeof(int);
			return_ptr = (char *)ptr_int + sizeof(int);
		}
	}else if(found == 1 && (curr_first_free != first_free_ptr)){
		// if(curr_first_free->free != 100){
		// 	exit(EXIT_FAILURE);
		// }
		//found a free block that is in the middle of the heap 
		freeNode_t *new_node = NULL;
		if(curr_first_free->length > (size + sizeof(int))){ //leftover space in free block
			//check that leftover space is big enough to hold metadata
			if((curr_first_free->length - size - sizeof(int)) < sizeof(freeNode_t)){ 
				//not big enough for metadata --> store leftover memory with malloc call
				previous_free_node->next_ptr = curr_first_free->next_ptr;
				
				curr_first_free->next_ptr = NULL;

				int *ptr_int = (int *)curr_first_free;
				int length1 = curr_first_free->length;
				*ptr_int = length1;
				return_ptr = (char *)ptr_int + sizeof(int);					
			}else{
				//enough leftover memory --> create new free region
				new_node = (freeNode_t *)((char *)curr_first_free + size + sizeof(int));
				new_node->length = curr_first_free->length - size - sizeof(int);
				new_node->next_ptr = curr_first_free->next_ptr;
				// new_node->free = 100;
				previous_free_node->next_ptr = new_node;

				curr_first_free->next_ptr = NULL;

				int *ptr_int = (int *)curr_first_free;
				*ptr_int = size + sizeof(int);
				return_ptr = (char *)ptr_int + sizeof(int);
			}
		}else{ //size + sizeof = first free length
			previous_free_node->next_ptr = curr_first_free->next_ptr;

			curr_first_free->next_ptr = NULL;

			int *ptr_int = (int *)curr_first_free;
			int length1 = curr_first_free->length;
			*ptr_int = length1;
			return_ptr = (char *)ptr_int + sizeof(int);
		}
	}
  return return_ptr;
}

/* have free regions be doubly linked
	when inserting node at beginning loop through 
*/

//first fit free
void ff_free(void * ptr){
	if (ptr == NULL || ptr < start_address || ptr > end_address) {
		return;  // Ignore invalid frees
	}
	/*
  	1. have temp value to look at previous block (sizeof(int)) to find length of memory block needed to be freed
    2. check if location to be freed is less than address of first free pointer
				-- if yes, update first free pointer and other
				-- if no, update last free pointer to point to that address
    3. free the region specified by the length
				-- at the address of memory length, create new node with length of memory and next pointer, of free node type
    4. check if previous adjacent regions are free????
				-- if yes, combine follow first free pointer, check is first free + length = start of new free region
				-- combine, add lengths togther, next pointer = pointer of second region
				-- assume that arrows are in order, and when updating keep in order
    5. check is next region behind is free
				-- if yes, combine
  */

 	// freeNode_t *curr_temp_free = first_free_ptr;
	// int already_freed = 0;
	// while(curr_temp_free != NULL && already_freed == 0){
	// 	if(ptr == curr_temp_free){
	// 		already_freed = 1;
	// 	}
	// }

	// int within_range = 0;
	// if(ptr >= start_address && ptr <= (end_address - sizeof(freeNode_t))){
	// 	within_range = 1;
	// }

	// if(already_freed == 0 && within_range == 1){
	freeNode_t *new_node_ptr = (freeNode_t *)((char *)ptr - sizeof(int));

	int *int_length_ptr = (int *)((char *)ptr - sizeof(int)); //int pointer pointing at address holding length of used mem
	int length_used_mem = *int_length_ptr;
	new_node_ptr->length = length_used_mem ;
	new_node_ptr->next_ptr = NULL;

	/* check if location to be freed is less than address of first free pointer
			-- if yes, update first free pointer, check if the regions are adjacent
					-- if yes, combine
					-- if no, update next pointers 
			-- if no, loop through free nodes, updating previous and curr pointers until you find the location to free
					-- check if the region to free has adjacent free regions and combine 
	*/

	if((char *)new_node_ptr < (char *)first_free_ptr){
		if((new_node_ptr->length + (char *)new_node_ptr) == (char *)first_free_ptr){
			new_node_ptr->length += first_free_ptr->length;
			new_node_ptr->next_ptr = first_free_ptr->next_ptr;
			first_free_ptr->next_ptr = NULL;
		}else{
			new_node_ptr->next_ptr = first_free_ptr;
		}
		first_free_ptr = new_node_ptr;
	}else{
		int found = 0;
		freeNode_t *curr_free_node = first_free_ptr;
		freeNode_t *prev_free_node = first_free_ptr;

		while(found == 0 && (curr_free_node != NULL)){
			if((char *)curr_free_node < (char *)new_node_ptr){ 
				prev_free_node = curr_free_node;
				curr_free_node = curr_free_node->next_ptr;
			}else{
				new_node_ptr->next_ptr = curr_free_node;
				prev_free_node->next_ptr = new_node_ptr;
				found =1;

				//combine with adjacent region with after
				if (((char*)new_node_ptr + new_node_ptr->length) == (char *)new_node_ptr->next_ptr) {
					new_node_ptr->length += new_node_ptr->next_ptr->length;
					new_node_ptr->next_ptr = new_node_ptr->next_ptr->next_ptr;

					new_node_ptr->next_ptr->next_ptr = NULL;

				}

				//combine with adjacent with addresses before
				if ((char*)prev_free_node + prev_free_node->length == (char *)new_node_ptr) {
					prev_free_node->length += new_node_ptr->length;
					prev_free_node->next_ptr = new_node_ptr->next_ptr;

					new_node_ptr->next_ptr = NULL;

				}
			}
		}

		if(found == 0 && (curr_free_node == NULL)){ //adding free region to the end 
			prev_free_node->next_ptr = new_node_ptr;
			if ((char*)prev_free_node + prev_free_node->length == (char *)new_node_ptr) { //check if adjacent region is free 
				prev_free_node->length += new_node_ptr->length;
				prev_free_node->next_ptr = NULL; 
			}
		}
	}	
	// }
}

//best fit malloc
void *bf_malloc(size_t size){
	//void *end_address = NULL; 
  void *new_address = NULL; 
	int new_increment_size = 0; 
	freeNode_t *node_ptr = NULL;
	void *return_ptr = NULL;

	if(size == 0){
		return NULL;
	}
	//printf("1\n");
	//minimum bytes allowed to allocate 
	if((size + sizeof(int)) < sizeof(freeNode_t)){
		size = sizeof(freeNode_t) - sizeof(int);
	} 

	//check if there is there is no free node using first free pointer	
		//this is happen either on the first time a malloc is called or when you have run out of free space 
	//printf("2\n");
  if(first_free_ptr == NULL){
    //start_address = sbrk(0);
		//printf("sbrk 6 = %p\n", start_address);

		//check if 4092 is big enough to hold requested memory 
		if(INCR_VAL > size + sizeof(int)){
			new_increment_size = INCR_VAL; 
		}else{
			new_increment_size = size*2; 
		}
		new_address = sbrk(new_increment_size);
		if(new_address == (void*) -1) {
			exit(EXIT_FAILURE);
		}
		//printf("sbrk 7 = %p\n", new_address);
		//printf("new address, sbrk called = %p \n", new_address);
		//printf("new address = %p\n", new_address);
		// if(new_address == NULL || *(int *)new_address == -1){
		// 	exit(EXIT_FAILURE);
		// }
		memory_size += new_increment_size;

		// new end address after allocating 
		//end_address = sbrk(0);
		//printf("sbrk 8 = %p\n", end_address);

    //initialize memory
      //initialize head, first free, first free length, next free chunk pointer (NULL)
    first_free_ptr = (freeNode_t *)new_address;
    node_ptr = (freeNode_t *)first_free_ptr;

    node_ptr->length = new_increment_size;
    node_ptr->next_ptr = NULL;
  }
	//printf("3\n");

	// printf("print free regions\n");
	// freeNode_t *test_free = first_free_ptr;
	// while(test_free != NULL){
	// 	printf("free node = %p && length = %d\n", (char *)test_free + sizeof(int), test_free->length);
	// 	test_free = test_free->next_ptr;
	// }

	int found = 0;
	freeNode_t *curr_first_free = first_free_ptr;
	freeNode_t *previous_free_node = NULL;
	freeNode_t *prev_previous_free_node = NULL;
	freeNode_t *best_fit_node = NULL;
	freeNode_t *previous_to_best_fit_node = NULL;
	int best_fit_leftover= 0;
	int curr_leftover = 0;
	//printf("4\n");
	//loop through all free regions and keep track of free region with least amount of leftover space 
	while(curr_first_free != NULL){
		//printf("curr free length = %d && size = %ld \n", curr_first_free->length,size );
		if(curr_first_free != NULL && curr_first_free->length >= (size + sizeof(int))){
			curr_leftover = curr_first_free->length - size - sizeof(int);
			if(best_fit_node == NULL){
				best_fit_node = curr_first_free;
				previous_to_best_fit_node = previous_free_node;
				best_fit_leftover = curr_leftover;
			}else if(curr_leftover < best_fit_leftover){
				best_fit_leftover = curr_leftover;
				best_fit_node = curr_first_free;
				previous_to_best_fit_node = previous_free_node;
			}
			found = 1;
		}
		prev_previous_free_node = previous_free_node;
		previous_free_node = curr_first_free;
		curr_first_free = curr_first_free->next_ptr;
	}
	//printf("5\n");
	
	
	//condition: there is no suitable free region of memory for requested size 
 	if(found == 0){
		//printf("6\n");
		//void *prev_end_address = end_address;
		freeNode_t *new_node = NULL;
		
		new_increment_size = size + sizeof(int);

		new_address = sbrk(new_increment_size);
		if (new_address == (void*) -1) {
			exit(EXIT_FAILURE);
		}
		end_address = sbrk(0);
		//printf("sbrk 9 = %p\n", new_address);
		//printf("size = %ld, new address, sbrk called = %p \n", size, new_address);
		//printf("new address = %p\n", new_address);
		// if(new_address == NULL || *(int *)new_address == -1){
		// 	exit(EXIT_FAILURE);
		// }
		memory_size += new_increment_size;

		//get the new end address after allocating
		//end_address = sbrk(0);
		//printf("sbrk 10 = %p\n", end_address);
		//printf("7\n");
		//check if the previous free region + length equals new address to combine free regions
		if(previous_free_node != NULL && ((char *)previous_free_node + previous_free_node->length) == new_address){
			//combining
			//printf("8\n");
			new_node = (freeNode_t *)((char *)previous_free_node + sizeof(int) + size);
			new_node->length = previous_free_node->length + new_increment_size - sizeof(int) - size;
			new_node->next_ptr = NULL;

			int *ptr_int = (int *)previous_free_node;
			*ptr_int = size + sizeof(int);
			return_ptr = (char *)ptr_int + sizeof(int);
			
			//check if first free pointer needs to be updates 
			if(prev_previous_free_node != NULL){
				prev_previous_free_node->next_ptr = new_node;
			}else{
				first_free_ptr = new_node;
			}
			// printf("9\n");
		}else {
			// printf("10\n");
			//dont need to combine and add new node and new free region to heap 
			//printf("new address = %p\n", new_address);
			// new_node = (freeNode_t *)((char *)new_address + size + sizeof(int));
			// new_node->length = new_increment_size - size - sizeof(int);
			// new_node->next_ptr = NULL;
			// previous_free_node->next_ptr = new_node;

			int *ptr_int = new_address;
			*ptr_int = size + sizeof(int);
			return_ptr = (char *)ptr_int + sizeof(int);
			// printf("11\n");
		}
		// printf("12\n");
	}else if(found == 1 && (best_fit_node == first_free_ptr)){
		// printf("13\n");
		//a free block was found and it was the first free region
		freeNode_t *new_node = NULL;
		if(best_fit_node->length > (size + sizeof(int))){ //leftover space in free block
			// printf("14\n");
			if(best_fit_node->length - size - sizeof(int) < sizeof(freeNode_t)){
				//not big enough for metadata --> store leftover memory with malloc call
				// printf("15\n");
				first_free_ptr = best_fit_node->next_ptr;

				int *ptr_int = (int *)best_fit_node;
				int len = best_fit_node->length;
				*ptr_int = len;
				return_ptr = (char *)ptr_int + sizeof(int);
				// printf("16\n");
			}else{
				// printf("17\n");
				//enough leftover memory --> create new free region
				new_node = (freeNode_t *)((char *)best_fit_node + size + sizeof(int));
				new_node->length = best_fit_node->length - size - sizeof(int);
				new_node->next_ptr = best_fit_node->next_ptr;
				
				int *ptr_int = (int *)best_fit_node;
				*ptr_int = size + sizeof(int);
				return_ptr = (char *)ptr_int + sizeof(int);
				first_free_ptr = new_node;
				// printf("18\n");
			}
			// printf("19\n");

		}else{ //size + sizeof = first free length 
			//no leftover space in free region -- perfect fit 
			// printf("20\n");
			first_free_ptr = first_free_ptr->next_ptr;

			int *ptr_int = (int *)best_fit_node;
			*ptr_int = size + sizeof(int);
			return_ptr = (char *)ptr_int + sizeof(int);
			// printf("21\n");
		}
		// printf("22\n");
	}else if(found == 1 && (best_fit_node != first_free_ptr)){
		// printf("23\n");
		// found a free block that is in the middle of the heap
		freeNode_t *new_node = NULL;
		if(best_fit_node->length > (size + sizeof(int))){ //leftover space in free block
		// printf("24\n");
			//check that leftover space is big enough to hold metadata
			if(best_fit_node->length - size - sizeof(int) < sizeof(freeNode_t)){
				// printf("25\n");
				//not big enough for metadata --> store leftover memory with malloc call
				previous_to_best_fit_node->next_ptr = best_fit_node->next_ptr;
				
				int *ptr_int = (int *)best_fit_node;
				int len = best_fit_node->length;
				*ptr_int = len;
				return_ptr = (char *)ptr_int + sizeof(int);
				// printf("26\n");
			}else{
				// printf("27\n");
				//enough leftover memory --> create new free region
				new_node = (freeNode_t *)((char *)best_fit_node + size + sizeof(int));
				new_node->length = best_fit_node->length - size - sizeof(int);
				new_node->next_ptr = best_fit_node->next_ptr;
				previous_to_best_fit_node->next_ptr = new_node;

				int *ptr_int = (int *)best_fit_node;
				*ptr_int = size + sizeof(int);
				return_ptr = (char *)ptr_int + sizeof(int);
				// printf("28\n");
			}
		}else{ //size + sizeof = first free length
			// printf("29\n");
			previous_to_best_fit_node->next_ptr = best_fit_node->next_ptr;

			int *ptr_int = (int *)best_fit_node;
			int len = best_fit_node->length;
			*ptr_int = len;
			return_ptr = (char *)ptr_int + sizeof(int);
			// printf("30\n");
		}
		// printf("31\n");
	}
	// printf("32\n");
  return return_ptr;
}

//best fit free
void bf_free(void *ptr){
	//same algorithm as first fit free 
	ff_free(ptr);
}

/* entire heap memory
    --> this includes memory used to save metadata
*/
unsigned long get_data_segment_size(){
	return memory_size;

}//in bytes

/* size of the "free list"
    --> actual usable free space + space occupied by metadata of the blocks in your free list
*/
unsigned long get_data_segment_free_space_size(){
	unsigned long return_value = 0;
	
	freeNode_t *curr_free_node = first_free_ptr;
	while(curr_free_node != NULL){
		//printf("free node = %p && length = %d\n", (char *)curr_free_node + sizeof(int), curr_free_node->length);
		return_value += curr_free_node->length;
		curr_free_node = curr_free_node->next_ptr;
	}
	return return_value;
}



