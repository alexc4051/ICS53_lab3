// Chung, Po-Chien: 63533832
// Gupta, Nikki: 11391554
// Dubon, Jeremy: 19812840

#include <ctype.h>
#include <stdbool.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <stdint.h>

typedef uint16_t header_t;

/*** struct Command ***
This structure stores a single line read in from getline() formatted as a
two-dimensional array of strings stored in Command.array.
*/
typedef struct Command {
  char* program; // command, also array[0].
  char** array; // A array of strings which represent the given command.
  unsigned int len; // The number of strings in this.array.

  void* _line_alloc; // The address of the allocated line.
  void* _strings_alloc; // The address of the array of strings.
} Command;

// Prototypes
header_t* next_block(header_t* header);
int block_count = 0;
int read_command(struct Command* container);
void allocate_block(char*heap, char** input);
void free_block(char*heap, char** input);
void free_command(struct Command* target);
void heap_alloc();
void print_blocklist(char*heap, char** input);
void print_heap(char*heap, char** input);
void read_block(header_t* header, size_t* size, bool* allocated, size_t* blockID);
void write_block(char*heap, char** input);

int main() {
  heap_alloc();
  return 0;
}

/*** function run_heap ***
Run the memory allocation as outlined in lab 3:
https://eee.uci.edu/16s/36680/labs/lab3_malloc.pdf
*/
void heap_alloc() {
  char *heap = malloc (400);
  struct Command input; // Input read in from the command line.
  // Set all values in the heap to zero
  memset(heap, '\0', 400);
  *(header_t*)heap = 400;
  // Run the heap_alloc's loop.
  while(1) {
    // Read in input from the console and run the command.
    if(read_command(&input) > 0) {
      // Quit condition.
      if(strcmp(input.program, "quit") == 0) {
        // End Program
        free_command(&input);
        break;
      }
      else if (strcmp(input.program, "allocate") == 0 && input.len == 2){
		    //Allocate Heap Space
        allocate_block(heap, input.array);
  	  }
  	  else if (strcmp(input.program, "free") == 0 && input.len == 2){
  		  //Free Heap Space
        free_block(heap, input.array);
  	  }
  	  else if (strcmp(input.program, "blocklist") == 0 && input.len == 1){
  		  //Prints out Block Info
        print_blocklist(heap, input.array);
  	  }
  	  else if (strcmp(input.program, "writeheap") == 0 && input.len == 4){
  		  //Writes X chars to heap block
        write_block(heap, input.array);
  	  }
  	  else if (strcmp(input.program, "printheap") == 0 && input.len == 3){
  		  //Prints out heap w/out header
        print_heap(heap, input.array);
  	  }
  	  else{
  		  fprintf(stderr, "Error: Invalid command or invalid arguments.\n");
  	  }
  	} else {
        fprintf(stderr, "Error: Unable to read in input.\n");
  	}
    free_command(&input);
  }
  // Deallocate the heap.
  free(heap);
}

/*** function read_command ***
Read in a line from stdin and build a Command structure based off of the
information read in. The function getline() will be used to read in data.
See the definition of a Command structure for information about the Command
struct created.
Input:
# struct Command* container : The address of a Command structure.
Returns:
# Success : int container->len
# Failure : int -1
*/
int read_command(struct Command* container) {
  char* line = NULL; // The line read in from stdin
  char** strings = NULL; // Pointers to different words in "line"
  int j = 0; // Index
  size_t i = 0; // Index
  size_t len = 0; // The length of "line"
  size_t strings_len = 0; // The starting size of "strings"
  // Initialize "container"
  container->_line_alloc = NULL;
  container->_strings_alloc = NULL;
  container->array = NULL;
  container->len = 0;
  container->program = NULL;
  // Read in line from stdin (getline() will allocate space.)
  printf("> ");
  if(getline(&line, &len, stdin) < 0) {
    fprintf(stderr, "read_command(): Error: Unable to read a line from stdin.\n");
    free(line);
    return -1;
  }
  // Format line as needed for the Command structure.
  for(i = 0; i < len && line[i] != '\0';) {
    // Read over whitespace and set all whitespace characters to '\0'.
    while(line[i] != '\0' && isspace(line[i]) != 0) {
      line[i++] = '\0';
    }
    // Break once we have reached the end of the string.
    if(line[i] == '\0') break;
    // If a new word is detected, increment strings_len.
    strings_len++;
    // Read past all none-whitespace characters.
    while(isspace(line[i]) == 0) {
      i++;
    }
  }
  // If the user entered blank input, do not modify the container.
  if(strings_len == 0) {
    free(line);
    return 0;
  }
  // Allocate space for "strings"
  strings = calloc(strings_len + 1, sizeof(char*));
  if(!strings) {
    fprintf(stderr, "read_command(): Error: calloc() failed for \"strings\".\n");
    free(line);
    return -1;
  }
  // Point the values of "strings" at different words in "line".
  for(i = 0, j = 0; j < strings_len;) {
    // Advance past the null characters
    while(line[i] == '\0') {
      i++;
    }
    // Point to the next non-null character
    strings[j] = line + i;
    j++;
    // Advance to the next null character
    while(line[i] != '\0') {
      i++;
    }
  }

  // Store the data in "container"
  container->_line_alloc = line;
  container->_strings_alloc = strings;
  container->array = strings;
  container->len = strings_len;
  container->program = strings[0];

  return container->len;
}

/*** function free_command ***
Deallocate a previously-allocated Command structure and set it's values to zero.
Input:
# struct Command* target : The address of the target Command structure.
Returns: void
*/

void free_command(struct Command* target) {
  if(!target) {
    fprintf(stderr, "free_command(): Error: null target provided.\n");
    return;
  }

  if(target->_line_alloc) free(target->_line_alloc);
  if(target->_strings_alloc) free(target->_strings_alloc);

  target->_line_alloc = NULL;
  target->_strings_alloc = NULL;
  target->array = NULL;
  target->len = 0;
  target->program = NULL;
}

/*** function allocate_block ***
Create a new block of the target size.
Input:
# Heap heap : The target heap
# char** input : The users command-line input
  input[0] : The name of this program
  input[1] : Number of bytes to be allocated
Returns: void
*/
void allocate_block(char*heap, char** input) {
	size_t allocationSize = atoi(input[1]);
	size_t size;
	bool allocated = false;
	char *point = heap;
	header_t header = *point;
	size_t blockID;
	if(allocationSize <= 2) {
		puts("Invalid allocation size. Header is 2 bytes. Minimum Allocation Size is 3 bytes");
		return;
	}
  allocationSize += 2;
	read_block((header_t*) point, &size, &allocated, &blockID);
	while (header && (allocated == true || size < allocationSize)){
		// header = (header_t*)(((char*) header) + size);
		point = (char*) next_block((header_t*) point);
		header = *point;
		read_block((header_t*) point, &size, &allocated, &blockID);
	}
	++block_count;
	printf("%d\n", block_count);
	if (size == allocationSize){
		*(header_t*)point = (allocationSize | 0x8000) + block_count*512;
	}
	else if((size - allocationSize) > 2){
		*(header_t*)point = (allocationSize | 0x8000) + block_count*512;
		point = point + allocationSize;
		if (!*(header_t*)point){
			*(header_t*)point = size - allocationSize;
		}
	}
	else{
		printf("Remaining blocks remaining will lead to fragmentation, block size increased to eliminate fragmentation.\n");
		*(header_t*)point = (size | 0x8000) + block_count*512;
		point = point + size;
		if (!*(header_t*)point){
			*(header_t*)point = size;
		}
	}
}

/*** function free_block ***
Deallocate a target block.
Input:
# Heap heap : The target heap
# char** input : The users command-line input
  input[0] : The name of this program
  input[1] : The number of the block which will be deallocated.
Returns: void
*/
void free_block(char*heap, char** input) {
	int blockDelete = atoi(input[1]);
	size_t blockID;
	size_t size;
	bool allocated;
	char *point = heap;
  size_t totalSize = 0;
  if(blockDelete <= 0) {
    puts("Invalid block number.");
    return;
  }
	read_block((header_t*) point, &size, &allocated, &blockID);
	while(blockID != blockDelete){
	  point = (char*) next_block((header_t*) point);
	  read_block((header_t*) point, &size, &allocated, &blockID);
    totalSize += size;
	  if (blockID == 0 || totalSize >= 400){
			puts("Invalid block number.");
			return;
	  }
  }
	if (blockID == blockDelete){
		if (!allocated){
			puts("Invalid block number.");
			return;
		}
		*(header_t*)point = *(header_t*)point & ~0x8000;
	}
}

/*** function print_blocklist ***
Print a list of all blocks in the heap.
Input:
# Heap heap : The target heap
# char** input : The users command-line input
  input[0] : The name of this program
Returns: void
*/
void print_blocklist(char*heap, char** input) {
  size_t size; // The size of a single block.
  size_t blockID;
  bool allocated; // The allocation status of a single block.
  char* start = heap; // The starting address of a single block.
  char* end; // The ending address of a single block
  size_t totalSize = 0; // The total amount of blocks we have examined.

  // printf("Size\tAllocated\tStart\tEnd\t\n");
  printf("%-7s%-12s%-16s%-16s\n", "Size", "Allocated", "Start", "End");

  // Loop through the blocks
  while(*(header_t*)start) { // && blockID !=0
    // Read in the target block.
    read_block((header_t*) start, &size, &allocated, &blockID);
    // Loop until we are out of the bounds of our heap. (400 blocks).
    totalSize += size;
    if(totalSize >= 400) {
      break;
    }
    // Point to it's end.
    end = start + size - 1;
    // Print it's information to stdout.
    printf("%-7ld%-12s%-16p%-16p\n", size, allocated == true ? "yes" : "no", start, end);
    // Advance to the next block.
    start = (char*) next_block((header_t*) start);
  }
}

/*** function write_block ***
Write characters to a target block.
Input:
# Heap heap : The target heap
# char** input : The users command-line input
  input[0] : The name of this program
  input[1] : Target block number
  input[2] : The character which will be written
  input[3] : The number of characters written
Returns: void
*/
void write_block(char*heap, char** input) {
  int targetBlock = atoi(input[1]); // Target block number
  char character = input[2][0]; // The character which will be written
  int charNum = atoi(input[3]); // The number of characters requested to be written
  size_t size; // The allocated space
  size_t blockID; // The block id of the examined block.
  bool allocated; // Check block to make sure it is allocated to write to?
  header_t* point = (header_t*) heap; // The examined header.
  int i; // Index

  // Validate input
  if(targetBlock <= 0 || charNum <= 0 || strlen(input[2]) != 1) {
    puts("Invalid arguments.");
    return;
  }

  // Find the target block
  while(1) {
    read_block(point, &size, &allocated, &blockID);
    if(blockID == targetBlock) {
      break;
    } else if(blockID == 0) {
      puts("Unable to find target block");
      return;
    } else {
      point = next_block(point);
    }
  }
	printf("%ld\n", blockID);
  if (!allocated){
		puts("Invalid block number. Block must be reallocated to use.");
		return;
	}
  if (size-2 < charNum) printf("ERROR: YOU ARE NOW CORRUPTING THE HEAP!\n");

  for(i = 0; i < charNum; i++) {
    *((char*) point + 2 + i) = character;
  }

}

/*** function print_heap ***
Print the contents of a portion of the heap.
Input:
# Heap heap : The target heap
# char** input : The users command-line input
  input[0] : The name of this program
  input[1] : Target block number
  input[2] : Number of bytes (chars) to print
Returns: void
*/
void print_heap(char*heap, char** input) {
  int targetBlock = atoi(input[1]); // The target block
  int numBytes = atoi(input[2]); // The number of bytes to print
  size_t size; // The allocated space
  size_t blockID; // The block id of the examined block.
  bool allocated; // Check block to make sure it is allocated to write to?
  header_t* point = (header_t*) heap; // The examined header.
  int i; // Index

  if(targetBlock <= 0 || numBytes <= 0) {
    puts("Invalid arguments.");
    return;
  }

  // Find the target block
  while(1) {
    read_block(point, &size, &allocated, &blockID);
    if(blockID == targetBlock) {
      break;
    } else if(blockID == 0) {
      puts("Unable to find target block");
      return;
    } else {
      point = next_block(point);
    }
  }
  if (!allocated){
		puts("Invalid block number.");
		return;
	}

  if (size-2 < numBytes) printf("ERROR: Reading more than block size.\n");

  for(i = 0; i < numBytes; i++) {
    printf("%c", *((char*) point + 2 + i));
  }
  printf("\n");
}

/*** function next_block ***
Read a given header and jump to the next value in the heap.
Input:
# header_t* header : A pointer to the initial header.
Return:
# Success : A pointer to the next header
# Failure : NULL
*/
header_t* next_block(header_t* header) {
  int size; // The size of the provided header.

  if(!header) {
    fprintf(stderr, "Null header passed into next_block\n");
    return NULL;
  }

  // Read in the size of the header.
  // Note: 0x8000 == 1000 0000 0000 0000 in binary, so this line will get rid
  // of the most significant bit (that stores the allocated bit).
  size = ((*header & ~0x8000) - ((*header & ~0x8000)/512)*512);

  // Return a pointer to the next header.
  return (header_t*)(((char*) header) + size);
}

/*** function read_block ***
Read data from a header.
Input:
# header_t* header : A pointer to the target header.
# size_t* size : The size read in from "header"
# bool* allocated : A boolean indicating whether or not the block is allocated.
Return: void
*/
void read_block(header_t* header, size_t* size, bool* allocated, size_t* blockID) {
  *allocated = *header & 0x8000 ? true : false; // Check the allocated bit
  *blockID = (size_t)((*header & ~0x8000)/512);
  *size = (size_t)((*header & ~0x8000) - (*blockID)*512); // Read in the size of the header.
}
