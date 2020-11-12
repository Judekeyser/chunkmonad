#include <stdlib.h>
#include <stdio.h>

typedef struct IntList IntList;
struct IntList {
	int head;
	IntList* tail;
};
IntList* visitTail(IntList* list) {
	if(list) return list -> tail;
	return list;
}

IntList* rangeClose(int value) {
	IntList* current = (void*)0;
	int counter = value;
	while(counter > 0) {
		IntList* newOne = malloc(sizeof(IntList));
		newOne -> head = counter;
		newOne -> tail = current;
		current = newOne;
		counter = counter - 1;
	}
	return current;
}

typedef struct SumAccumulator {
	int accu;
} SumAccumulator;

void accumulate(SumAccumulator* accumulator, int value) {
	accumulator -> accu += value;
}

#define __T_NEXT_VISITOR__ visitTailWithForget
#define __T_DISPOSE_VISITOR__ free
#include "chunkmonad.h"
void* visitTailWithForget(void* provenList) {
	return (void*) visitTail((IntList*) provenList);
}

int heavyComputation() {
	SumAccumulator acc = {.accu = 0};
	RUN_WORKFLOW(
		rangeClose(7), //Starting declaration (IntList)
		int value = ((IntList*) getvalue($$)) -> head, //how to consume value from IntList = (getvalue($$): void*)
		WORKFLOW( //monad workflow
			LMAP(value, value * 2 + value)
			LFILTER(value % 2 == 0)
			FLATMAP(value, rangeClose, 4) //4 flag here is mandatory
			LMAP(value, value + 1)
			LCOLLECT(acc.accu += value)
		)
	)
	return acc.accu;
}

#undef __T_NEXT_VISITOR__
#undef __T_DISPOSE_VISITOR__

int main() {
	printf("Result of heavy computation is %d:\n", heavyComputation());
	return 0;
}

