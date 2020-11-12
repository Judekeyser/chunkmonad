#include <stdlib.h>
#include "chunkmonad.h"

struct ChunkZip {
	int restore;
	void* ref;
	ChunkZip* next;
};

void* getvalue(ChunkZip* chunk) {
	return chunk -> ref;
}

int restorepoint(ChunkZip* chunk) {
	return chunk -> restore;
}

ChunkZip* unit(void* ref) {
	ChunkZip* chunk = malloc(sizeof(ChunkZip));
	chunk -> restore = 0;
	chunk -> ref = ref;
	chunk -> next = (void*)0;
	return chunk;
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
	} else {
		ChunkZip* next = chunk -> next;
		free(chunk);
		return next;
	}
}

void emitevent(ChunkZip* lastNonNull, void* event, int restore) {
	while(lastNonNull -> next) {
		lastNonNull = lastNonNull -> next;
	}

	ChunkZip* newChunk = malloc(sizeof(ChunkZip));
	newChunk -> restore = restore;
	newChunk -> ref = event;

	lastNonNull -> next = newChunk;
}

