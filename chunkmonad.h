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

#define __FALSE__TO_NOTHING__
#define __FALSE__(a) !!0
#define CHAIN_WITH_OR(symb,...) __CHAIN_WITH_OR__(symb,__VA_ARGS__,\
		__FALSE__,__FALSE__,__FALSE__,__FALSE__,__FALSE__,__FALSE__,__FALSE__)
#define __CHAIN_WITH_OR__(symb,a,b,c,d,e,f,g,END,...) \
	a(symb) || b(symb) || c(symb) || d(symb) || e(symb) || f(symb) || g(symb)

/**
 * User can use their own __VAL__ private symbol
 * if ever the one of this library does not fit their needs
 */
#ifndef __VAL__
#define __VAL__ $wrappedValue
#endif

#define RUN_WORKFLOW(_START_SYMB,_STORAGE,...) do {\
	ChunkMonad* $$ = chunkmonad_unit(_START_SYMB); do {\
		void* __VAL__ = __T_CURRENT_VISITOR__(chunkmonad_getvalue($$));\
		_STORAGE;\
		__VA_ARGS__\
		$$ = chunkmonad_moveforward($$,__T_NEXT_VISITOR__,__T_DISPOSE_VISITOR__);\
		} while($$);\
	} while(0);

#define WORKFLOW_STORAGE(...) __VA_ARGS__
#define APPLY(value,target,...) target = REVERSE_FUNC_CALL(value,__VA_ARGS__);
#define FILTER_OR(value,...) if(!(CHAIN_WITH_OR(value,__VA_ARGS__))) break;
#define NOT(method) !method
#define FLATMAP(value,method,flag) chunkmonad_emitevent($$,method(value),flag);break;case flag:
#define LCOLLECT(...) default: __VA_ARGS__;
#define WORKFLOW(...) switch(chunkmonad_restorepoint($$)) { case 0: __VA_ARGS__ }
#define LAPPLY(value,...) value = __VA_ARGS__;
#define LFILTER(...) if(!(__VA_ARGS__)) break;
#define PULL_AS(type,symbol) symbol = (type*)__VAL__;
#define DEFER_PULL_AS(type,symbol) symbol = *((type*)__VAL__);
#endif

