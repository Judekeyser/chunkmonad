#ifndef __CHUNKMONAD_MODULE__
#define __CHUNKMONAD_MODULE__

typedef struct ChunkMonad ChunkMonad;

typedef void*(NextVisitor)(void*);
typedef void(DisposeVisitor)(void*);

void* chunkmonad_getvalue(ChunkMonad* chunk);

int chunkmonad_restorepoint(ChunkMonad* chunk);

ChunkMonad* chunkmonad_unit(void* ref);

ChunkMonad* chunkmonad_moveforward(ChunkMonad* chunk, NextVisitor nextVisitor, DisposeVisitor disposeVisitor);

void chunkmonad_emitevent(ChunkMonad* lastNonNull, void* event, int restore);

#define CAT(a,b) a ## b
#define __IDENTITY__TO_NOTHING__
#define __IDENTITY__(a) a
#define REVERSE_FUNC_CALL(symb,...) __REVERSE_FUNC_CALL__(symb,__VA_ARGS__,\
		__IDENTITY__, __IDENTITY__, __IDENTITY__, __IDENTITY__, __IDENTITY__,__IDENTITY__, __IDENTITY__)
#define __REVERSE_FUNC_CALL__(symb,a,b,c,d,e,f,g,END,...) CAT(END, TO_NOTHING__)g(f(e(d(c(b(a(symb)))))))

#define __VAL__ $wrappedValue
#define RUN_WORKFLOW(_START_SYMB,_STORAGE,...) do {\
	ChunkMonad* $$ = chunkmonad_unit(_START_SYMB); do {\
		void* __VAL__ = __T_CURRENT_VISITOR__(chunkmonad_getvalue($$));\
		_STORAGE;\
		__VA_ARGS__\
		$$ = chunkmonad_moveforward($$,__T_NEXT_VISITOR__,__T_DISPOSE_VISITOR__);\
		} while($$);\
	} while(0);

#define WORKFLOW_STORAGE(...) __VA_ARGS__
#define MAP(value,target,...) target = REVERSE_FUNC_CALL(value,__VA_ARGS__);
#define FILTER(value,method) if(!method(value)) break;
#define FLATMAP(value,method,flag) chunkmonad_emitevent($$,method(value),flag);break;case flag:
#define LCOLLECT(...) default: __VA_ARGS__;
#define WORKFLOW(...) switch(chunkmonad_restorepoint($$)) { case 0: __VA_ARGS__ }
#define LMAP(value,...) value = __VA_ARGS__;
#define LFILTER(...) if(!(__VA_ARGS__)) break;

#endif

