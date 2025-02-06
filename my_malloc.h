/*
Your  implementation  is  required  to  coalesce  in  this situation  by  
merging  the  adjacent  free  regions  into  a  single  free  region  of  memory. 
Similarly,  it  is  also  required  to  split  free  regions  if  the  ideal  free  
region  is  larger  than requested size. 
*/

//first fit = first block of free memory that fits malloc
//best fit = block of free memory that has the least amount of leftover space 

//use sbrk() to get a big block of memory 
//will keep using sbrk() to grow memory 
  //void *sbrk(intptr_t increment);

//keep a data structure that represents a list of free memory regions

struct _freeNode {
  int length;
  struct _freeNode *next_ptr;
  //int free; //0 for not free, 100 for free
  //struct _freeNode *prev_ptr; 
};
typedef struct _freeNode freeNode_t;

// struct _mallocNode {
//   int length;
//   int flag; //100 for not free, 0 for free
//   //struct _freeNode *prev_ptr; 
// };
// typedef struct _freeNode freeNode_t;

//first fit malloc
void *ff_malloc(size_t size);

//first fit free
void ff_free(void * ptr);

//best fit malloc
void *bf_malloc(size_t size);

//best fit free
void bf_free(void *ptr);

/* entire heap memory
    --> this includes memory used to save metadata
*/
unsigned long get_data_segment_size(); //in bytes

/*size of the "free list"
  --> actual usable free space + space occupied by metadata of the blocks in your free list
  */
unsigned long get_data_segment_free_space_size(); //in bytes


