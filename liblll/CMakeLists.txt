set(sources
	CodeFragment.cpp
	CodeFragment.h
	Compiler.cpp
	Compiler.h
	CompilerState.cpp
	CompilerState.h
	Exceptions.h
	Parser.cpp
	Parser.h
)

add_library(lll ${sources})
target_link_libraries(lll PUBLIC evmasm devcore)
