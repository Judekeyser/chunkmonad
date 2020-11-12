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
The assumption of *chunkmonad* is that, all Object Oriented Programming concept
asides, the monad really stands for a description of a computation flow.

In C, there is no generic typing nor objects, so all what's known about monads
are usually unappliable. However, their benefit and the problem they solve can
inspire some code.

The following code is a fully expressive description of what *chunkmonad*
offers, in plain C:
```c
void* visitTailWithForget(void* provenList) {
	return (void*) visitTail((IntList*) provenList);
}
void* visitCurrentValueWithForget(void* provenList) {
	return (void*) &(((IntList*) provenList) -> head);
}

int heavyComputation() {
	SumAccumulator acc = {.acc = 0};
	ChunkZip* $$ = unit(rangeClose(7)); // starting point here
	do {
		// Visit current list value and pass it to extract the wrapped value
		void* $wrappedvalue = visitCurrentValueWithForget(getvalue($$));
		// Working section, as in Cobol, for convenience
		int value;
		// Depending on the restore point of the current chunk, we branch
		switch(restorepoint($$)) {
			case 0: // default case
				value = *((int*) $wrappedvalue);
				value = value * 2 + value;
				if(! (value % 2 == 0) )break;
				/* Flat map trick: emit event to current chunk,
					so that the result of flat map is chained
					after the current processing chunks.

				The newly added chunked will be restored with restore
				state 4.
				*/
				emitevent($$, rangeClose(value), 4);break;
			case 4:
				value = *((int*) $wrappedvalue);
				value = value + 1;
				// Switch fallthrough
			default:
				acc.accu += value;
		}
		/* Move current chunk state:
			we first get rid of the current iteration state with
				a dispose method (free here)
			then we move the head of the chunk: if something remains
				to visit, we visit (using visitTailWithForget)
			otherwise we move chunk itself to next chunk, if any has been
				registered (using flatmap)
		*/
		$$ = moveforward($$, visitTailWithForget, free);
	} while($$);
	return acc.accu;
}
```
In the code above, the `ChunkZip` is a struct that follows a linked list design.
The reader should be able to guess the "tail recursive like" nature of the code.

Simply put, a first chunk is created and refers to the starting list.
Elements of this list are traversed using `visitTailWithForget`, and values are
extracted using `visitCurrentValueWithForget`. Those methods are forced to be typed
to `void*` to satisfy the C type system.

MAP and FILTER operations are really straightforward and can be translated in plain C.
FLATMAP is a bit more subtle. Simply put, each FLATMAP is going to
create a new chunk, marked with a restoration point. Details of this procedure
are explicit by following the implementation `chunkmonad.c`.

#### Chunkmonad, with sugar
Once you got the basic idea, you can benefit from the macros we have developed, to
make the experience a bit more user friendly. Below is an example of exactly the
same procedure, with macros:
```c
int heavyComputation() {
	SumAccumulator acc = {.accu = 0};
	RUN_WORKFLOW(
		rangeClose(7),
		WORFLOW_STORAGE(
			int value;
		),
		WORKFLOW(
			value = *((int*)__VAL__);
			LMAP(value, value * 2 + value);
			LFILTER(value % 2 == 0)
			FLATMAP(value, rangeClose, 4)
			value = *((int*)__VAL__);
			LMAP(value, value + 1)
			LCOLLECT(acc.accu += value)
		)
	)
	return acc.accu;
}
```

## Header file explained
This section explains the header file `chunkmonad.h`. It is the only file that requires
to be included in your project.


