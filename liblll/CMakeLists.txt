set(sources
	CodeFragment.cpp
	Compiler.cpp
	CompilerState.cpp
	Parser.cpp
)

add_library(lll ${sources})
target_link_libraries(lll PUBLIC evmasm devcore)
