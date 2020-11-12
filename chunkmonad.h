/**
 * In order to use the RUN_WORKFLOW macro, make sure you have defined in your scope
 * a macro __T_NEXT_VISITOR__ that is the name of a method (void* -> void*) that
 * extracts from a monad element it's next element to visit.
 *
 * For example, working with linked list, this visitor is the tail visitor, hence
 * the method could be
 * void* visitTailWithForget(void* provenList) {
 *    if (provenList) return (void*)(provenList -> tail);
 *    return (void*)0;
 * }
 * and the macro will be
 * __T_NEXT_VISITOR__ visitTailWithForget
 *
 */

#define RUN_WORKFLOW(_START_SYMB,_CONSUMES,...) do {\
	ChunkZip* $$ = unit(_START_SYMB); do {_CONSUMES;\
		__VA_ARGS__\
		$$ = moveforward($$,__T_NEXT_VISITOR__,__T_DISPOSE_VISITOR__);\
		} while($$);\
	} while(0);

typedef struct ChunkZip ChunkZip;

typedef void*(NextVisitor)(void*);
typedef void(DisposeVisitor)(void*);

void* getvalue(ChunkZip* chunk);

int restorepoint(ChunkZip* chunk);

ChunkZip* unit(void* ref);

ChunkZip* moveforward(ChunkZip* chunk, NextVisitor nextVisitor, DisposeVisitor disposeVisitor);

void emitevent(ChunkZip* lastNonNull, void* event, int restore);

#define MAP(value,method) value = method(value);
#define FILTER(value,method) if(!method(value)) break;
#define FLATMAP(value,method,flag) emitevent($$,method(value),flag);break;case flag:
#define LCOLLECT(...) default: __VA_ARGS__;
#define WORKFLOW(...) switch(restorepoint($$)) { case 0: __VA_ARGS__ }
#define LMAP(value,...) value = __VA_ARGS__;
#define LFILTER(...) if(!(__VA_ARGS__)) break;
