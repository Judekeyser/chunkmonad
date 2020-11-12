# chunkmonad
A small lightweight library for faking monad style programming in pure C.

## Scope of the project
The *chunkmonad* project is concerned with bringing a piece of monad style programming
in the C language. The code has been tested under GCC compiler, although it was written
with the most fundamental C features in mind.

The header file defines both macros and methods. The methods maybe of interest for the
user, although the use of macros make stuffs easier to approach.

### What you're expected to write, as a client
As a client of the library, you are expected to define a method of the form
```c
void workflow(ChunkZip* myCommand, T* myAccumulator)
```
In this expression, the `ChunkZip` type is a type of ours. Think of it as a wrapper around
your monad. The `T` type is a type of your choice: it's the accumulator. This is the
collect/reduce operation performed at the very end of your monad flow.

### Examples
Below are described examples. We do not have much materials in pure C, so we have
redefined a very basic monad called `IntList`, together with a very basic accumulator
`SumAcc`.

The implementations are straightforward:
```c
typedef struct SumAcc {
	int accu;
};
void accumulate(SumAcc* self, int value) {
	self -> accu += value;
}
```
and
```c
typedef struct IntList IntList;
struct IntList {
	int head;
	IntList* tail;
};
IntList* visitTail(IntList* list) {
	if(list) return list -> head;
	return list;
}
```
You should directly see the kind of code we have in mind. Before exposing the C-monadic
styled code, we give examples of the example algorithm in Java (using Stream API),
in Python (using for comprehension) and in chunkmonad style.

#### Java
```java
IntStream.rangeClose(1, 7)
	.map(i -> 2*i + i)
	.filter(i -> i % 2 == 0)
	.flatMap(i -> IntStream.rangeClose(1, i))
	.map(i -> i + 1)
	.sum();
```

#### Python
Sorry for the Python lovers, I wasn't brave enough to make it in one line.
```python
def rangeClose(i): return range(1, i+1)

x = [2*i+i for i in rangeClose(7)]
x = [i for i in x if i % 2 == 0]
x = [j for i in x for j in rangeClose(i)]
x = [i+1 for i in x]
sum(x)
```

#### Chunkmonad, explicit content
```c
void workflow(ChunkZip* myCommand, SumAcc* myAccumulator) {
	int value = ((IntList*) getvalue(myCommand)) -> head;
	RESTORE

	LMAP(value, 2*value + value)
	LFILTER(value % 2 == 0)
	FLATMAP(value, rangeClose, FLATMAP_FLAG(lbl_accu))
	LMPAT(value, value + 1)
	COLLECT(accumulate, value)
}
#define __T_NEXT_VISITOR__ visitTailWithForget
#define __T_DISPOSE_VISITOR__ free
#include "chunkmonad.h"
void* visitTailWithForget(void* provenList) {
	return (void*) visitTail((IntList*) provenList);
}
int computeSum(IntList* list) {
	SumAcc accumulator = {.accu = 0};
	RUN_WORKFLOW(workflow, list, &accumulator);
	int result = accumulator -> accu;
	return result;
}
#undef __T_NEXT_VISITOR__
#undef __T_DISPOSE_VISITOR__

computeSum(rangeClose(7));
```
As you see, all the monadic flow is enclosed in a method `workflow`.
The monad operations will be discussed later on, and we'll explain why
FLATMAP requires an extra flag.

The remaining part of the job is to define visitors for easier access.
In comparison to Java, C does not have generic typing; hence the most reusable way of
moving is by typing most of the stuffs to `void*`.

In comparison to Python, variables are typed in C. Therefore, the `void*` typing from
above forces us to define forget-functors like operations.

## Header file explained
This section explains the header file `chunkmonad.h`. It is the only file that requires
to be included in your project.


