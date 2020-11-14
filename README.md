# chunkmonad
A small lightweight library for faking monad style programming in pure C.

## Scope of the project
The *chunkmonad* project is concerned with bringing a piece of monad style programming
in the C language. The code has been tested under GCC compiler, although it was written
with the most fundamental C features in mind.

The header file defines both macros and methods. The methods maybe of interest for the
user, although the use of macros make stuffs easier to approach.

### Examples
Below are described examples. We do not have much materials in pure C, so we have
redefined a very basic monad called `IntList`, together with a very basic accumulator
`SumAcc`.

The implementations are straightforward:
```c
typedef struct SumAccumulator {
	int accu;
};
void accumulate(SumAccumulator* self, int value) {
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
Before exposing the C-monadic
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

## Getting started

### `RUN_WORKFLOW`
All the concern of the chunkmonad library is to run a workflow, that consists of maps, filters
and flatmaps.

Running a workflow is all about invoking the macro `RUN_WORKFLOW`, whose syntax is
```c
RUN_WORFLOW(
	element_on_which_computation_applies,
	workflow_storage_section,
	workflowd_definition
)
```
The below sections explain how to configure the workflow and how to define it.

### Workflow configuration
Before runnin the workflow, you'll need to include the header file `chunkmonad.h`. This file
exposes both methods and macros. If you go in plain macro style, you'll never need to use the methods.
Still, you may find them useful, maybe one day.

To make the macros run, you'll need to configure your environment with three macros you have to define
yourself:
```c
#define __T_CURRENT_VISITOR__ visit_current_value_with_type_forget_method
#define __T_NEXT_VISITOR__ visit_next_monadic_element_with_type_forget
#define __T_DISPOSE_VISITOR__ dispose_current_monadic_element_with_type_forget
```
Those macros need to refer to methods in your program, satisfying the following contracts:
1. `__T_CURRENT_VISITOR__` is of type `void* -> void*` and returns, given a monadic element, the wrapped value the element represents.
2. `__T_NEXT_VISITOR__` is of type `void* -> void*` and returns, given a monadic element, the next element to visit.
3. `__T_DISPOSE_VISITOR__` if of type `void* -> void` and performs, given a monadic element, a kill of it.

The workflow will run with the following specification:
1. Given the first element to visit (the `element_on_which_computation_applies` argument of the workflow), a *chunk* is formed containing this element. The chunk will be dropped only when no other elements can be reached from the current enclosed one; iteration is done using the `next_visitor`.
2. For each iterated element, the wrapped value is extracted (using the `current_visitor`) and stored in a macro `__VAL__`. The type will be `void*`, so it is your responsibility to defer the pointer correctly.
3. One an element in a chunk is visited, the chunk will call the `dispose_visitor` to dispose it, before getting the next element to visit.

As such, there will be only *one chunk*, and this situation will not change *unless* you're using FLATMAP.
Each FLATMAP operation will append a new chunk afte the list of chunks to visit, with a given restore point.

Restore points (the flag in the FLATMAP, see below) should thus be unique inside a workflow definition;
but the ordering does not matter.

### `WORKFLOW_STORAGE`
Once the workflow configured and the first element written down, the second parameter is the workflow storage
section, which should be of the form
```c
WORKFLOW_STORAGE(
	/* variable declarations, each ended by a semi-column */
)
```
The variables enclosed in the workflow storage section will be available inside the workflow definition
and can be used as temporary variables. This is important because, as a reminder, the `__VAL__` macro
refers to a `void*` variable, and it can become quickly annoying to perform multiple casts. 

Because C is strongly typed but has no generics, the workflow storage section can be seen as a wild card to
maintain a pool of variables (the shortest the pool, the better, off course).

### `WORKFLOW` definition
The definition of the workflow goes in the third parameter, of the form
```c
WORKFLOW(
	/* operations to perform, each as valid C instructions */
)
```
The operations to perform should be C instructions or macro calls. See the `main.c` for a full example.

## Further developments

### Get rid of FLATMAP flag
If possible, get rid of FLATMAP flag. That's not really blocking as the ordering does not matter, but it is
quite ugly.

### Review syntax for MAP and FLATMAP
Currently, the syntax for MAP and FLATMAP are quite asymmetric and there is a need to call again a
variable initializer after FLATMAP, although it is not clearly shown in the syntax.

We would like to homogeneized this and rely on a syntax.

