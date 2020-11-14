#include <stdlib.h>
#include "chunkmonad.h"

typedef struct ChunkLastMnemoized ChunkLastMnemoized;

struct ChunkZip {
	int restore;
	void* ref;
	ChunkZip* next;
	ChunkLastMnemoized* lastIndirRef;
};

struct ChunkLastMnemoized {
	ChunkZip* ref;
};

void* getvalue(ChunkZip* chunk) {
	return chunk -> ref;
}

int restorepoint(ChunkZip* chunk) {
	return chunk -> restore;
}

ChunkZip* unit(void* ref) {
	ChunkZip* chunk = malloc(sizeof(ChunkZip));
	ChunkLastMnemoized* lastIndirRef = malloc(sizeof(ChunkLastMnemoized));
	chunk -> restore = 0;
	chunk -> ref = ref;
	chunk -> next = (void*)0;
	chunk -> lastIndirRef = lastIndirRef;
	lastIndirRef -> ref = chunk;
	return chunk;
}

static ChunkZip* dispose_and_returnnext(ChunkZip* chunk) {
	ChunkZip* next = chunk -> next;
	if (!next) {
		free(chunk -> lastIndirRef);
	}
	free(chunk);
	return next;
}

ChunkZip* moveforward(ChunkZip* chunk, NextVisitor nextVisitor, DisposeVisitor disposeVisitor) { 
	if (chunk -> ref) {
		void* ref = chunk -> ref;
		chunk -> ref = nextVisitor(chunk -> ref);
		disposeVisitor(ref);
		if (!(chunk -> ref)) {
			return moveforward(chunk,nextVisitor,disposeVisitor);
		}
		return chunk;
	} else return dispose_and_returnnext(chunk);
}

void emitevent(ChunkZip* lastNonNull, void* event, int restore) {
	/*while(lastNonNull -> next) {
		lastNonNull = lastNonNull -> next;
	}*/

	ChunkZip* newChunk = malloc(sizeof(ChunkZip));
	newChunk -> restore = restore;
	newChunk -> ref = event;
	newChunk -> lastIndirRef = lastNonNull -> lastIndirRef;

	lastNonNull -> lastIndirRef -> ref -> next = newChunk;
	lastNonNull -> lastIndirRef -> ref = newChunk;
}

