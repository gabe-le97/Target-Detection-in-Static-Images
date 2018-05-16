//
//  Blob.c
//  Project
//
//  Created by Jean-Yves Hervé on 2018-05-02.
//  Edited by Gabe Le on 2018-05-09.
//  Copyright © 2018 URI 3D Group. All rights reserved.
//

#include <stdlib.h>
#include <stdio.h>
//
#include "Blob.h"
#include "gl_frontEnd.h"

//-----------------------------------------------------------
//	Add a segment to a segment list (part of a blob)
//-----------------------------------------------------------
int addSegmentToList(ExtentList* list, unsigned int xL, unsigned int xR,
					unsigned int y) 
{
	Extent seg = {xL, xR, y};
	return addExtentToList(list, seg);
}

int addExtentToList(ExtentList* list, Extent seg) {
	int ok = 0;
	if (list->nbSegs>0 && list->segList[0].y == seg.y) {
		//	Find at which position in the list the new element should
		//	be inserted.
		int index = 0;

		while (index < list->nbSegs && seg.xL > list->segList[index].xR) {
			index++;
		}
		
		//	Allocate a new list, one bigger than the current one
		Extent* newSegList = (Extent*) calloc(list->nbSegs+1, sizeof(Extent));

		if (newSegList == NULL) {
			printf("Failed to allocate segment list in addExtentToList\n");
			exit(84);
		}

		for (int i=0, j=0; i<list->nbSegs+1; i++) {
			if (i==index) {
				newSegList[i] = seg;
			}
			else {
				newSegList[i] = list->segList[j];
				j++;
			}
		}
		
		//	free the old list
		free(list->segList);
		list->segList = newSegList;
		list->nbSegs++;
		ok = 1;
	}
	else if (list->nbSegs==0) {
		list->segList = (Extent*) calloc(1, sizeof(Extent));
		if (list->segList == NULL) {
			printf("Failed to allocate segment list in addExtentToList\n");
			exit(85);
		}
		list->segList[0] = seg;
		list->nbSegs = 1;
		ok = 1;
	}
	return ok;
}

//-----------------------------------------------------------
//	Produces a new extent stack properly initialized
//-----------------------------------------------------------
ExtentStack newExtentStack(void) {
	ExtentStack newStack;
	newStack.stackTop = 0;
	newStack.storageSize = STACK_STORAGE_INCR;
	newStack.stack = (Extent*) calloc(STACK_STORAGE_INCR, sizeof(Extent));
	if (newStack.stack == NULL) {
		printf("Allocation of resized stack failed in newExtentStack\n");
		exit(40);
	}

	return newStack;
}

//-----------------------------------------------------------
//	Add an extent to a stack
//-----------------------------------------------------------
int addExtentToStack(ExtentStack* eStack, Extent seg) {
	eStack->stack[eStack->stackTop] = seg;
	eStack->stackTop++;
	if (eStack->stackTop == eStack->storageSize) {
		//	allocate a larger array to store the stack
		Extent* newStack = (Extent*) calloc(eStack->storageSize + STACK_STORAGE_INCR,
											sizeof(Extent));
		if (newStack == NULL) {
			printf("Allocation of resized stack failed in addExtentToStack\n");
			exit(41);
		}
		
		for (int k=0; k<eStack->stackTop; k++) {
			newStack[k] = eStack->stack[k];
		}
		
		//	all has been copied, so this is the new stack
		free(eStack->stack);
		eStack->stack = newStack;
		eStack->storageSize += STACK_STORAGE_INCR;
	}

	return 1;
}

//-----------------------------------------------------------
//	Add a segment to a stack
//-----------------------------------------------------------
int addSegmentToStack(ExtentStack* eStack, unsigned int xL, unsigned int xR,
					   unsigned int y)
{
	Extent seg = {xL, xR, y};
	return addExtentToStack(eStack, seg);
}

//-----------------------------------------------------------
//	Checks if a stack is empty
//-----------------------------------------------------------
int stackIsEmpty(ExtentStack* eStack) {
	return (eStack != NULL && eStack->stackTop == 0);
}


//-----------------------------------------------------------
//	Pops the top element of an extent stack.
//-----------------------------------------------------------
Extent popStack(ExtentStack* eStack) {
	Extent top;
	
	if (!stackIsEmpty(eStack)) {
		top = eStack->stack[eStack->stackTop-1];
		eStack->stackTop--;
		
		//	If we have more than 2*STACK_STORAGE_INCR free spots on
		//	the stack, we reduce the size
		if (eStack->storageSize >= eStack->stackTop + 2*STACK_STORAGE_INCR) {
			//	alllocate a smaller storage array
			Extent* newStack = (Extent*) calloc(eStack->stackTop + STACK_STORAGE_INCR,
											sizeof(Extent));
			if (newStack == NULL) {
				printf("Allocation of resized stack failed in addExtentToStack\n");
				exit(42);
			}

			for (int k=0; k<eStack->stackTop; k++) {
				newStack[k] = eStack->stack[k];
			}
			
			//	all has been copied, so this is the new stack
			free(eStack->stack);
			eStack->stack = newStack;
			eStack->storageSize -= STACK_STORAGE_INCR;
		}
	}
	else {
		printf("Attempt to pop from an empty stack\n");
		exit(43);
	}
	
	return top;
}


//-----------------------------------------------------------
//	Produces a new blob properly initialized
//-----------------------------------------------------------
Blob newBlob(void) {
	Blob newBlob;
	newBlob.red = newBlob.green = newBlob.blue = 0x00;
	newBlob.nbPixels = newBlob.nbSegs = 0;
	newBlob.deque = NULL;

	return newBlob;
}

//-----------------------------------------------------------
//	Add a segment to a blob
//-----------------------------------------------------------

int addSegmentToBlob(Blob* blob, unsigned int xL, unsigned int xR,
					unsigned int y)
{
	Extent seg = {xL, xR, y};
	return addExtentToBlob(blob, seg);
}


int addExtentToBlob(Blob* blob, Extent seg) {
	int ok = 0;
	
	if (blob->nbSegs > 0) {
		const unsigned int blobHeight = blob->yBottom - blob->yTop + 1;

		//	First check that the new segment is in the y range of the
		//	blob
		if (seg.y >= blob->yTop && seg.y <= blob->yBottom) {
			ok = addExtentToList(blob->deque + (seg.y - blob->yTop), seg);
		}
		else if (seg.y == blob->yTop - 1) {
			//	We need to allocate a new deque (array of ExtentList)
			ExtentList* newDeque = (ExtentList*) malloc((blobHeight + 1)*sizeof(ExtentList));
			if (newDeque == NULL) {
				printf("Failed to allocate deque in addExtentToBlob\n");
				exit(83);
			}
			for (unsigned i=0, j=1; i<blobHeight; i++, j++) {
				newDeque[j] = blob->deque[i];
			}
			newDeque[0].nbSegs = 0;
			newDeque[0].segList = NULL;
			ok = addExtentToList(newDeque + 0, seg);
			free(blob->deque);
			blob->deque = newDeque;
			blob->yTop--;
		}
		else if (seg.y == blob->yBottom + 1) {
			//	We need to allocate a new deque (array of ExtentList)
			ExtentList* newDeque = (ExtentList*) malloc((blobHeight + 1)*sizeof(ExtentList));
			if (newDeque == NULL) {
				printf("Failed to allocate deque in addExtentToBlob\n");
				exit(82);
			}
			for (unsigned i=0; i<blobHeight; i++) {
				newDeque[i] = blob->deque[i];
			}
			newDeque[blobHeight].nbSegs = 0;
			newDeque[blobHeight].segList = NULL;
			ok = addExtentToList(newDeque + blobHeight, seg);
			free(blob->deque);
			blob->deque = newDeque;
			blob->yBottom++;
		}
	}
	else {
		blob->deque = (ExtentList*) malloc(1*sizeof(ExtentList));
		if (blob->deque == NULL) {
			printf("Failed to allocate deque in addExtentToBlob\n");
			exit(81);
		}
		blob->yTop = blob->yBottom = seg.y;
		blob->deque[0].nbSegs = 0;
		ok = addExtentToList(blob->deque + 0, seg);
	}
	
	if (ok) {
		blob->nbSegs++;
		blob->nbPixels += seg.xR - seg.xL + 1;
	}
	
	return ok;
}

//-----------------------------------------------------------
//	Render a blob in its assigned color.
//-----------------------------------------------------------
void renderBlob(Blob* blob) {
	glColor4ub(blob->red, blob->green, blob->blue, 0xFF);

	const unsigned int blobHeight = blob->yBottom - blob->yTop + 1;

    glBegin(GL_QUADS);
	for (unsigned int i=0; i<blobHeight; i++) {
		for (int j=0; j<blob->deque[i].nbSegs; j++) {
			Extent seg = blob->deque[i].segList[j];
			glVertex2i(seg.xL, seg.y);
			glVertex2i(seg.xR+1, seg.y);
			glVertex2i(seg.xR+1, seg.y+1);
			glVertex2i(seg.xL, seg.y+1);
		}
	}
	glEnd();
}

void printoutBlob(Blob* blob) {
	printf("\nBlob with %d segments and %d pixels:\n", blob->nbSegs, blob->nbPixels);
	const unsigned int blobHeight = blob->yBottom - blob->yTop + 1;
 	for (unsigned int i=0; i<blobHeight; i++) {
		for (int j=0; j<blob->deque[i].nbSegs; j++) {
			Extent seg = blob->deque[i].segList[j];
			printf("\tsegment (%d, %d, %d)\n", seg.xL, seg.xR, seg.y);
		}
	}
}

//-----------------------------------------------------------
//	Delete a blob
//-----------------------------------------------------------
void deleteBlob(Blob* blob) {
	free(blob->deque);
}

