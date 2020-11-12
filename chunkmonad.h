#define __VAL__ $wrappedValue
#define RUN_WORKFLOW(_START_SYMB,_STORAGE,...) do {\
	ChunkZip* $$ = unit(_START_SYMB); do {\
		void* __VAL__ = __T_CURRENT_VISITOR__(getvalue($$));\
		_STORAGE;\
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

#define WORKFLOW_STORAGE(...) __VA_ARGS__
#define MAP(value,method) value = method(value);
#define FILTER(value,method) if(!method(value)) break;
#define FLATMAP(value,method,flag) emitevent($$,method(value),flag);break;case flag:
#define LCOLLECT(...) default: __VA_ARGS__;
#define WORKFLOW(...) switch(restorepoint($$)) { case 0: __VA_ARGS__ }
#define LMAP(value,...) value = __VA_ARGS__;
#define LFILTER(...) if(!(__VA_ARGS__)) break;
