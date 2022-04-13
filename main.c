/*Dušan Podmanický,DSA LS 2020 zadanie èíslo 1*/
#define _CRT_SECURE_NO_WARNINGS
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <math.h>
#include <time.h>
#define OFFSET 4			//offset from the first 4 bytes dedicated to keeping the total memory size
#define HEADERSIZE 4		//size of the header in BYTES
#define FOOTERSIZE 4
#define FIRSTLISTSIZE 4
#define MINIMALALLOC 8											
#define MAXSIZEOFMEM 1000000
void* p_0;

void* memory_alloc(unsigned int size);
int memory_free(void* valid_ptr);
int memory_check(void* ptr);
void memory_init(void* ptr, unsigned int size);
void* findfooter(void* p_header);								//returns footer of given header
int isend(void* p_footer);										//returns 1 if this is the footer of total memory
void* findheader(void* ptr);									//returns header to the previously allocated pointer
void* mergecheck(void* p_header);								//checks whether the given header can be merged with previous or next
void* mergeprev(void* p_header);								//returns pointer to the header merged with the next header in total memory
void* mergenext(void* p_header);								//returns pointer to the header after it was merged with previous header in total memory
void removefromlist(void* p_header);							//removes the given header from list it was in
void sorttolist(void* p_header);								//sorts to header to the list it belongs to
void settolist(void* p_header, void** p_list);					//sets the pointer to be the new first element in the given list
void* nextinlist(void* p_header);								//returns pointer to the next header in list
void* previnlist(void* p_hedaer);								//returns pointer to the previous header in list
void* allocfromlist(unsigned int size,void* p_list);			//allocates from lists of previously freed memory
void* allocfromrest(unsigned int size);							//allocates from the rest of memory
void listinit(unsigned int size);								//initialises the lists of previously freed headers
void* findrestofmem();											//returns pointer to the rest of memory
int settopositive(int number);									//sets the number to positive if its not
void* findnext(void* p_header);									//returns pointer to next header, doesnt check whether it exists
void* findheader(void* ptr);									//finds the header of a given pointer
int isfirst(void* p_header);									//if the given header is the first one in list, returns 1, otherwise returns 0
int islast(void* p_header);										//if the given header is the last one in list returns 1
void clean(void* p_header);										//sets 8 bytes after header to 0
void deletefooter(void* p_header);								//sets the footer of given header to 0

void listinit(unsigned int size) {
	char help = FIRSTLISTSIZE, blocknum;
	int i;

	blocknum = log(size) - 3;			//gives the amount of lists
	*((char*)p_0 + OFFSET) = blocknum;
	for (i = OFFSET; i < blocknum * (HEADERSIZE + 1) + OFFSET; i += HEADERSIZE + 1) {				//initialises list headers
		*((char*)p_0 + i) += help;
		help++;
	}
}
void memory_init(void* ptr, unsigned int size) {
	unsigned i;
	char blocknum, help = 6;
	int** p_restofmem, *p_restofmemfooter;

	p_0 = ptr;			// setting pointers
	for (i = 0; i < size; i++) {			//initialise memory to 0
		*((char*)p_0 + i) = 0;
	}
	*(unsigned int*)ptr = size;
	listinit(size);
	blocknum = *((char*)p_0 + OFFSET) - FIRSTLISTSIZE;
	if (blocknum < 0) {
		blocknum = 0;
	}
	p_restofmem = ((char*)p_0 + OFFSET + 1 + blocknum * (HEADERSIZE + 1));						//creates a pointer to the rest of memory from which first allocations will be made
	*(int*)p_restofmem = (int*)p_restofmem + 1;													//essentially creates the first block of free memory from which initial allocs will be made
	**(int**)p_restofmem = -1 * ((size - (OFFSET + (blocknum + 1) * (HEADERSIZE + 1))) - 2 * HEADERSIZE);
	p_restofmemfooter = findfooter(*(int*)p_restofmem);											//creates a footer for this block
	*(int*)p_restofmemfooter = *(int*)p_restofmem;
	//printf("(INIT) There are %d BYTES between the frist header and footer.\n", **(int**)p_restofmem * (-1));			//tells the user the maximum amount of bytes that can be requested
}
void* findfooter(void* p_header) {
	int blocksize = *(int*)p_header;

	if (blocksize < 0) {
		blocksize *= -1;
	}
	return (char*)p_header + blocksize + HEADERSIZE;	//sets the pointer to the footer of given block
}
void* findnext(void* p_header) {
	return (char*)p_header + settopositive(*(int*)p_header) + 2 * HEADERSIZE;		
}
void* findrestofmem() {
	void** p_restofmem;
	char* p_listheader;
	int blocknum;

	blocknum = *((char*)p_0 + OFFSET) - FIRSTLISTSIZE;
	if (blocknum < 0) {
		blocknum = 0;
	}
	p_listheader = ((char*)p_0 + OFFSET);
	p_restofmem = (p_listheader + 1) + (blocknum * (HEADERSIZE + 1));
	return p_restofmem;
}
void* findheader(void* ptr) {
	return (char*)ptr - HEADERSIZE;
}
int isend(void* p_footer) {
	unsigned int totalsize = *(unsigned int*)p_0;

	return (char*)p_footer + HEADERSIZE >= (char*)p_0 + totalsize ? 1 : 0;			//returns 1 if the next header would be beyond memory
}
int settopositive(int number) {
	return number < 0 ? (number * (-1)) : number;
}
void* memory_alloc(unsigned int size) {
	int i, sizeoflist;
	void* p_list, * p_return;
	char blocknum, * p_listheader;

	if (size < 8) {
		size = 8;
	}
	if (size == 0) {
		return NULL;
	}
	if (size > *(unsigned int*)p_0) {
		return NULL;
	}
	p_return = NULL;
	p_listheader = ((char*)p_0 + OFFSET);
	(char*)p_list = p_listheader - HEADERSIZE;					//offsets pointer to the first element of list before the for cycle
	blocknum = *((char*)p_0 + OFFSET) - FIRSTLISTSIZE;
	if (blocknum < 0) {
		blocknum = 0;
	}
	for (i = 0; i < blocknum; i++) {							//search through listheaders for a fit
		(char*)p_list += HEADERSIZE + 1;
		sizeoflist = pow(2, *p_listheader);
		if (i == 0) {
			sizeoflist = pow(2, FIRSTLISTSIZE);
		}
		if (*(int*)p_list != 0 && size < sizeoflist) {
			p_return = allocfromlist(size,p_list);						//if there is a possibility for allocation, try to alloc
		}
		if (i == blocknum - 1 && *(int*)p_list != 0) {					//if this is the last list, search it in case there are some blocks that werent sorted elsewhere
			p_return = allocfromlist(size, p_list);
		}
		if (p_return != NULL) {									// if memory was allocated successfully
			removefromlist(p_return);
			if (*(int*)p_return < 0) {
				*(int*)p_return = settopositive(*(int*)p_return);
			}
			return (char*)p_return + HEADERSIZE;
		}
	}	
	p_return = allocfromrest(size);						//if the allocation from previously freed blocks was unsuccessful check the rest of memory
	return p_return;
}
void* allocfromrest(unsigned int size){
	int oldleftover, newleftover;
	void** p_restofmem, * p_newheader, * p_newfooter, ** p_restofmemfooter;

	p_restofmem = findrestofmem();
	p_restofmemfooter = findfooter(*(int*)p_restofmem);
	if (**(int**)p_restofmem > 0) {
		//printf("Not enough memory.\n");
		return NULL;
	}
	oldleftover = **(int**)p_restofmem;
	if (settopositive(oldleftover) == size) {
		**(int**)p_restofmem = settopositive(oldleftover);
		p_newheader = *(int*)p_restofmem;
		return ((char*)p_newheader + HEADERSIZE);
	}
	newleftover = settopositive(oldleftover) - size - 2*HEADERSIZE;
	if (settopositive(oldleftover) < size) {
		//printf("Not enough memory.\n");
		return NULL;
	}
	if (settopositive(newleftover) < MINIMALALLOC) {					//if the rest of memory would become unassignable, assign it all
		p_newheader = *(int*)p_restofmem;								//sets header
		*(int*)p_newheader = settopositive(*(int*)p_newheader);
		p_newfooter = findfooter(p_newheader);				//sets footer
		return ((char*)p_newheader + HEADERSIZE);
	}
	else {												//esle just give the required amount
		p_newheader = *(int*)p_restofmem;
		*(int*)p_newheader = size;
		p_newfooter = findfooter(p_newheader);				//sets footer
		*(int*)p_newfooter = *(int*)p_restofmemfooter;					//copies the address of previous to the new footer
		*(int*)p_restofmem = (char*)p_newfooter + HEADERSIZE;
		**(int**)p_restofmem = - newleftover;
		*(int*)p_restofmemfooter = p_newheader;
		return ((char*)p_newheader + HEADERSIZE);
	}
}
void* allocfromlist(unsigned int size,void *p_list) {
	void* p_header, **p_nextinlist, *p_min,*p_footer,*p_newfooter;
	int min = 5000000, oldleftover, newleftover;

	p_min = NULL;
	p_header = *(int*)p_list;
	while (1){																				//finds best fit
		if (size <= settopositive(*(int*)p_header)) {
			if (settopositive(*(int*)p_header) - size < min) {
				min = settopositive(*(int*)p_header) - size;
				p_min = p_header;
			}
			if (min == 0) {
				return p_min;
			}
		}
		if (islast(p_header) == 0) {
			p_nextinlist = nextinlist(p_header);
			p_header = *(int*)p_nextinlist;
		}
		else {
			break;
		}
	}
	if (p_min != NULL) {							//if p_min isnt null and there is more memory in it, split it and sort to a list
		oldleftover = *(int*)p_min;
		newleftover = settopositive(oldleftover) - size - 2 * HEADERSIZE;
		if (newleftover < MINIMALALLOC) {					//if the rest of block become unassignable, assign it all
			return p_min;
		}
		else {												//else just give the required amount
			p_footer = findfooter(p_min);
			*(int*)p_min = size;
			p_newfooter = findfooter(p_min);				//sets footer
			*(int*)p_newfooter = *(int*)p_footer;					//copies the address of previous to the new footer
			*(int*)p_footer = p_min;
			p_header = findnext(p_header);
			*(int*)p_header = -newleftover;
			sorttolist(p_header);
			return p_min;
		}
	}
	return p_min;
}
int memory_free(void* valid_ptr) {
	void* p_header,*p_restofmem;
	int blocknum;
	
	p_restofmem = findrestofmem();
	blocknum = *((char*)p_0 + OFFSET) - FIRSTLISTSIZE;
	if (blocknum < 0) {
		blocknum = 0;
	}
	p_header=findheader(valid_ptr);
	if (*(int*)p_header < 0) {
		return 1;
	}
	p_header=mergecheck(p_header);								//checks for possible merging
	clean(p_header);											//sets the first 8 bytes to 0 in case there was something left over there
	if (blocknum && p_header!=*(int*)p_restofmem) {
		sorttolist(p_header);
	}
	else {
		*(int*)p_restofmem = p_header;
	}	
	return 0;
}
void* mergecheck(void* p_header) {
	void* p_merged, ** p_footer, * p_next;

	p_footer = findfooter(p_header);
	p_merged = p_header;
	if (*(int*)p_header > 0) {
		*(int*)p_header *= (-1);
	}
	if (isend(p_footer) == 0) {						//merges with next
		p_next = findnext(p_header);
		if (*(int*)p_next < 0) {
			p_merged = mergenext(p_header);
		}
	}
	p_footer = findfooter(p_header);				// have to check if a new footer was created
	if (*(int*)p_footer != p_header) {				// if its not the first block
		if (**(int**)p_footer < 0) {					//and the next one is free
			p_merged = mergeprev(p_header);
		}
	}
	p_footer = findfooter(p_header);
	if (*(int*)p_merged > 0) {						//in case the header wasnt set to negative before
		*(int*)p_merged *= -1;
	}
	return p_merged;
}
void* mergenext(void* p_header) {
	void* p_next, * p_footer, ** p_nextfooter, * p_nextnext, ** p_nextnextfooter,**p_restofmem;		

	p_footer = findfooter(p_header);				//2 cases, there are 2 blocks ahead of the one being freed or there is only one	
	p_next = findnext(p_header);
	p_nextfooter = findfooter(p_next);
	p_restofmem = findrestofmem();
	if (p_next != *(int*)p_restofmem) {
		removefromlist(p_next);
	}
	if (isend(p_nextfooter) == 0) {										//if there are two, rewrite both footers
		p_nextnext = findnext(p_next);
		p_nextnextfooter = findfooter(p_nextnext);
		*(int*)p_nextnextfooter = *(int*)p_nextfooter;
		*(int*)p_nextfooter = *(int*)p_footer;
	}
	else {																//otherwise rewrite just the next one
		*(int*)p_nextfooter = *(int*)p_footer;
		*(int*)p_restofmem = p_header;
	}
	deletefooter(p_header);
	*(int*)p_header = -1 * (settopositive(*(int*)p_header) + settopositive(*(int*)p_next) + 2 * HEADERSIZE);
	return p_header;
}
void* mergeprev(void* p_header) {
	void** p_prevfooter, * p_prev, ** p_footer,*p_newnext,*p_newnextfooter;

	p_footer = findfooter(p_header);
	p_prev = *(int*)p_footer;
	removefromlist(p_prev);
	p_prevfooter = findfooter(p_prev);
	*(int*)p_prev = -1 * (settopositive(*(int*)p_header) + settopositive(*(int*)p_prev) + 2 * HEADERSIZE);
	if (isend(p_footer) == 0) {
		p_newnext = findnext(p_header);										//have to move the pointer in the next one in case there were multiple merges going on
		p_newnextfooter = findfooter(p_newnext);
		*(int*)p_newnextfooter = *(int*)p_footer;
	}
	*(int*)p_footer = *(int*)p_prevfooter;
	return p_prev;
}
void clean(void* p_header) {
	int i;
	for (i = 0; i < 8; i++) {						//sets the first 8B of block to zeroes after it was freed, to make room for pointers
		*((char*)p_header + HEADERSIZE + i) = 0;
	}
}
void removefromlist(void* p_header) {
	void** p_list, **p_nextinlist, **p_previnlist,*p_restofmem,*p_helper;
	char* p_listheader;
	int blocknum;

	p_restofmem = findrestofmem();				//check whether this is the rest of memory pointer, if so do nothing
	if (p_header == *(int*)p_restofmem) {
		return;
	}
	p_listheader = ((char*)p_0 + OFFSET);
	(char*)p_list = p_listheader - HEADERSIZE;					//offsets list because of the for cycle
	blocknum = *((char*)p_0 + OFFSET) - FIRSTLISTSIZE;
	if (islast(p_header) && isfirst(p_header)) {					//if its the only block in the list
		p_previnlist = previnlist(p_header);								//removes the poninter from the list space after listheader
		**(int**)p_previnlist = 0;
		clean(p_header);
		return;
	}
	else if (isfirst(p_header)) {						//if its the first one in list
		p_nextinlist = nextinlist(p_header);
		p_previnlist = previnlist(p_header);
		**(int**)p_previnlist = *(int*)p_nextinlist;					//should rewrite address in list
		p_helper = *(int*)p_nextinlist;
		*((int*)p_helper+2) = *(int*)p_previnlist;					//sets the previous to be the start of the list
		clean(p_header);
		return;
	}
	else if (islast(p_header)) {							// if its the last one in list
		p_previnlist = previnlist(p_header);
		p_helper = *(int*)p_previnlist;
		*((int*)p_helper + 1) = 0;						//sets the next in previous to zero, making it the new last
		clean(p_header);
		return;
	}
	else {												//if it is somewhere in the middle of a list
		p_nextinlist = nextinlist(p_header);
		p_previnlist = previnlist(p_header);
		p_helper = *(int*)p_nextinlist;
		*((int*)p_helper + 2) = *(int*)p_previnlist;
		p_helper = *(int*)p_previnlist;
		*((int*)p_helper + 1) = *(int*)p_nextinlist;
		clean(p_header);
		return;
	}
}
void sorttolist(void* p_header) {
	void** p_restofmem,**p_list;
	char* p_listheader;
	int blocknum, i, sizeoflist;

	p_restofmem = findrestofmem();				//check whether this is the rest of memory pointer, if so do nothing
	if (p_header == *(int*)p_restofmem) {
		return;
	}
	p_listheader = ((char*)p_0 + OFFSET);
	(char*)p_list = p_listheader - HEADERSIZE;					//offsets list because of the for cycle
	blocknum = *((char*)p_0 + OFFSET) - FIRSTLISTSIZE;
	if (blocknum < 0) {
		blocknum = 0;
	}
	for (i = 0; i < blocknum; i++) {
		(char*)p_list += HEADERSIZE + 1;				//searches whether there are any lsits of free memory
		p_listheader = (char*)p_list - 1;
		sizeoflist = pow(2, *p_listheader);
		if (i == 0) {
			sizeoflist = pow(2, FIRSTLISTSIZE);
		}
		if (settopositive(*(int*)p_header) < sizeoflist) {
			settolist(p_header, p_list);
			return;
		}
	}
	settolist(p_header, p_list);						//if there is no big enough blockheader, send it to the last list
}
void settolist(void* p_header,void** p_list) {			//saves the given header as the first block of freed memory in the given list
	void* p_nextinlist;

	if (*(int*)p_list == 0) {					//if the list is empty set header as first in list
		*(int*)p_list = p_header;
		*((int*)p_header + 2) = p_list;			//if its the first block in list, sets the previous one to be the address of the list
		return;
	}
	else {
		p_nextinlist = *(int*)p_list;					//saves the pointer to the next one in list
		*(int*)p_list = p_header;						// sets the pointer as the new first
		*((int*)p_header + 2) = p_list;					//if its the first block in list, sets the previous one to be the address of the list
* ((int*)p_nextinlist + 2) = p_header;			//sets the preious pointer into the new next
*((int*)p_header + 1) = p_nextinlist;			//sets the pointer to the next on into the new first
	}
}
int isfirst(void* p_header) {
	void** p_list;
	char* p_listheader, blocknum;
	int i;

	p_listheader = ((char*)p_0 + OFFSET);
	(char*)p_list = p_listheader - HEADERSIZE;					//offsets list because of the for cycle
	blocknum = *((char*)p_0 + OFFSET) - FIRSTLISTSIZE;
	for (i = 1; i <= blocknum; i++) {
		(char*)p_list += HEADERSIZE + 1;				//searches whether there are any lsits of free memory
		p_listheader = (char*)p_list - 1;
		if (p_header == *(int*)p_list) {
			return 1;
		}
	}
	return 0;
}
int islast(void* p_header) {
	return (*((char*)p_header + HEADERSIZE) == 0) ? 1 : 0;
}
void* nextinlist(void* p_header) {
	return (char*)p_header + HEADERSIZE;
}
void* previnlist(void* p_header) {
	return (char*)p_header + 2 * HEADERSIZE;
}
void deletefooter(void* p_header) {					//sets the footer of given header to 0, not necessary
	void* p_footer;
	p_footer = findfooter(p_header);
	*(int*)p_footer = 0;
}
int memory_check(void* ptr) {
	void* p_header, * p_searched, ** p_footer,*p_newfooter;
	int blocknum;

	p_searched = findheader(ptr);
	blocknum = *((char*)p_0 + OFFSET) - FIRSTLISTSIZE;
	if (blocknum < 0) {
		blocknum = 0;
	}
	p_footer = (char*)p_0 + *(unsigned int*)p_0 - HEADERSIZE;						//set the footer as the last footer first
	p_header = ((char*)p_0 + OFFSET + 1 + HEADERSIZE + blocknum * (HEADERSIZE + 1));				//sets header to be the first header in memory
	while (p_header != p_footer) {
		if (p_header == p_searched && *(int*)p_header > 0) {
			return 1;
		}
		if (p_header == p_searched && *(int*)p_header < 0) {
			return 0;
		}
		p_newfooter = findfooter(p_header);
		if (p_footer == p_newfooter) {
			return 0;
		}
		p_header = findnext(p_header);
	}
	return 0;
}

/*TESTS AND MAIN*/
void numberofblocks_test(int size) {										//returns the % of blocks (of the given size) allocated vs the ideal amount 
	char* pointerarray[100000];
	int i = 0;
	double actual = 0, ideal;
	while (1) {
		pointerarray[i] = memory_alloc(size);
		if (pointerarray[i] != NULL) {
			actual++;
		}
		if (pointerarray[i] == NULL) {
			break;
		}
		i++;
	}
	for (i = 0; i < actual - 1; i++) {
		memory_free(pointerarray[i]);
	}
	ideal = *(unsigned int*)p_0 /size;
	printf("Allocated: %.0f\n", actual);
	printf("Percentual success: %.2f%%\n", 100 * (actual / ideal));
}

void merging_test(int seed) {											//allocates all the memory in block, frees the first half of pointers in a random order
	char* pointerarray[70000];									//subsequently allocs the first half of blocks, ensuring no memory "went missing"
	int array[20000], actual = 0;			
	int i=0, min = 0, j = 0;

	while (1) {
		pointerarray[i] = memory_alloc(8);
		if (pointerarray[i] != NULL) {
			actual++;
		}
		if (pointerarray[i] == NULL) {
			break;
		}
		i++;
	}
	srand(seed);
	for (i = 0; i < actual / 2; i++) {							//generates a random order of indexes
		int num = (rand() % (actual / 2));
		array[i] = num;
		for (j = 0; j < i; j++) {
			if (array[j] == array[i]) {
				i--;
				break;
			}
		}
	}
	for (i = 0; i < actual / 2; i++) {					//frees the first half of pointers in a random order
		memory_free(pointerarray[array[i]]);
	}
	for (i = 0; i < actual / 2; i++) {					//frees the first half of pointers in a random order
		pointerarray[array[i]]=memory_alloc(8);
	}
	for (i = actual / 2; i > 0; --i) {
		if (memory_check(pointerarray[i]) == 0) {
			printf("Unsuccessful test");
			return;
		}
	}
	printf("Successful test\n");
}
	
void check_test(int seed) {												//frees a random pointer in array, then chceks it
	int i = 0, j = 0, actual=0,num;
	char* pointerarray[70000];
	
	while (1) {
		pointerarray[i] = memory_alloc(8);
		if (pointerarray[i] != NULL) {
			actual++;
		}
		if (pointerarray[i] == NULL) {
			break;
		}
		i++;
	}
	srand(seed);
	num = (rand() % (actual));
	memory_free(pointerarray[num]);
	for (i = 0; i < actual; i++) {
		if ((memory_check(pointerarray[i]) == 0) && (i != num)) {
			printf("Unsuccessful test.\n");
		}if ((memory_check(pointerarray[i]) == 1) && (i == num)) {
			printf("Unsuccessful test.\n");
		}
	}
	printf("Successful test.\n");
}

void initialise(char* ptr){			//initialises all Bytes of given block with the number 13
	int i;
	void* p_header;

	p_header = findheader(ptr);
	for (i = 0; i < *(int*)p_header; i++) {
		ptr[i] = 13;
	}
}										

void allocrandomsize_test(int lower, int upper, int seed) {							//tries allocating random sizes into memory, until its full, then returns the amount of memory available to user
	int number, array[6], i = 0;
	double total = 0;
	char* pointerarray[100000];
	int** p_restofmem;

	p_restofmem = findrestofmem();
	srand(seed);
	while (**p_restofmem <= 0) {
		number = (rand() % (upper - lower + 1)) + lower;
		pointerarray[0] = memory_alloc(number);
		if (pointerarray[0] == NULL) {
			continue;
		}
		else {
			total += number;
		}
	}
	total = total - number + **p_restofmem;
	printf("The size of memory available to user is: %.2f\n", total);
	printf("Percentual: %.2f %%\n", total/(*(unsigned int*)p_0)*100);
}

void allocsamesize_test(int size) {
	char* pointer;
	int* p_header;
	double total = 0;

	while (1) {
		pointer = (char*)memory_alloc(size);
		if (pointer != NULL) {
			p_header = findheader(pointer);
			total += *p_header;
		}
		if (pointer == NULL) {
			break;
		}
	}
	printf("%.2f%%\n", 100*total/ (*(int*)p_0));
}

int main() {
	unsigned int size;
	char s, * pointerarray[10000],*region;
	region=(char*)malloc(MAXSIZEOFMEM);
	int index,**p_restofmem,lower,upper;

	printf("Press \"i\" for init.\n");
	printf("Press \"m\" for malloc.\n");
	printf("Press \"f\" for free.\n");
	printf("Press \"c\" for check.\n");
	printf("Press \"t\" for memory_check test.\n");
	printf("Press \"l\" for merging test.\n");
	printf("Press \"h\" for allocsamesize test.\n");
	printf("Press \"g\" for fragmentation test.\n");
	printf("Press \"r\" for rest of memory amount.\n");
	printf("Press \"a\" to get address of header.\n");
	printf("Press \"b\" to get contaitns of given header.\n");
	printf("Press \"p\" to initialise the allocated size with the number 13.\n");
	printf("Press \"w\" for allocating random sized blocks.\n");
	printf("Press \"e\" for end.\n");
	
	while (scanf("%c", &s)) {
		switch (s)
		{
		case 'i':
			printf("Input the total size of memory: ");
			(void)scanf("%d", &size);
			memory_init(&region[0], size);
			printf("Start of memory &: %p\n", p_0);
			break;
		case 'm':
			printf("How much? ");
			(void)scanf("%d", &size);
			printf("To which pointer?(from 0 to 199) ");
			(void)scanf("%d", &index);
			pointerarray[index] = memory_alloc(size);
			break;
		case 'f':
			printf("Index of pointer to be freed: ");
			(void)scanf("%d", &index);
			memory_free(pointerarray[index]);
			break;
		case 'c':
			printf("Index of pointer to be checked: ");
			(void)scanf("%d", &index);
			memory_check(pointerarray[index]) ? printf("Taken\n") : printf("Free\n");
			break;
		case 'l':
			printf("Set seed: ");
			scanf("%d", &index);
			merging_test(index);
			break;
		case 'a':
			printf("Give index: ");
			(void)scanf("%d", &index);
			printf("%p\n", findheader(pointerarray[index]));
			break;
		case 'r':
			p_restofmem = findrestofmem();
			printf("%d\n", **(int**)p_restofmem);
			break;
		case 'g':
			printf("The tested size: ");
			(void)scanf("%d", &index);
			numberofblocks_test(index);
			break;
		case 'b':
			printf("Give index: ");
			(void)scanf("%d", &index);
			p_restofmem = findheader(pointerarray[index]);
			printf("%d\n", *p_restofmem);
			break;
		case 't':
			printf("Set seed: ");
			scanf("%d", &index);
			check_test(index);
			break;
		case 'p':
			printf("Which pointer?\n");
			(void)scanf("%d", &index);
			initialise(pointerarray[index]);
			break;
		case 'w':
			printf("Set lower boundary.\n");
			(void)scanf("%d", &lower);
			printf("Set upper boundary.\n");
			(void)scanf("%d", &upper);
			printf("Set seed.\n");
			(void)scanf("%d", &index);
			allocrandomsize_test(lower,upper,index);
			break;
		case 'h':
			printf("Size to test: ");
			scanf("%d", &index);
			allocsamesize_test(index);
			break;
		case 'e':
			return 0;
			break;
		default:
			break;
		}
	}
}