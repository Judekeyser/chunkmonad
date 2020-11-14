#include <stdlib.h>
#include "chunkmonad.h"

typedef struct ChunkLastMnemoized ChunkLastMnemoized;

struct ChunkMonad {
	int restore;
	void* ref;
	ChunkMonad* next;
	ChunkLastMnemoized* lastIndirRef;
};

struct ChunkLastMnemoized {
	ChunkMonad* ref;
};

void* chunkmonad_getvalue(ChunkMonad* chunk) {
	return chunk -> ref;
}

int chunkmonad_restorepoint(ChunkMonad* chunk) {
	return chunk -> restore;
}

ChunkMonad* chunkmonad_unit(void* ref) {
	ChunkMonad* chunk = malloc(sizeof(ChunkMonad));
	ChunkLastMnemoized* lastIndirRef = malloc(sizeof(ChunkLastMnemoized));
	chunk -> restore = 0;
	chunk -> ref = ref;
	chunk -> next = (void*)0;
	chunk -> lastIndirRef = lastIndirRef;
	lastIndirRef -> ref = chunk;
	return chunk;
}

static ChunkMonad* dispose_and_returnnext(ChunkMonad* chunk) {
	ChunkMonad* next = chunk -> next;
	if (!next) {
		free(chunk -> lastIndirRef);
	}
	free(chunk);
	return next;
}

ChunkMonad* chunkmonad_moveforward(ChunkMonad* chunk, NextVisitor nextVisitor, DisposeVisitor disposeVisitor) { 
	if (chunk -> ref) {
		void* ref = chunk -> ref;
		chunk -> ref = nextVisitor(chunk -> ref);
		disposeVisitor(ref);
		if (!(chunk -> ref)) {
			return chunkmonad_moveforward(chunk,nextVisitor,disposeVisitor);
		}
		return chunk;
	} else return dispose_and_returnnext(chunk);
}

void chunkmonad_emitevent(ChunkMonad* lastNonNull, void* event, int restore) {
	/*while(lastNonNull -> next) {
		lastNonNull = lastNonNull -> next;
	}*/

	ChunkMonad* newChunk = malloc(sizeof(ChunkMonad));
	newChunk -> restore = restore;
	newChunk -> ref = event;
	newChunk -> lastIndirRef = lastNonNull -> lastIndirRef;

	lastNonNull -> lastIndirRef -> ref -> next = newChunk;
	lastNonNull -> lastIndirRef -> ref = newChunk;
}

