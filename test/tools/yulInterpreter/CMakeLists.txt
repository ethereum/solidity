set(sources
	Inspector.h
	Inspector.cpp
)

add_library(yulInterpreter ${sources})
target_link_libraries(yulInterpreter PUBLIC yul solidity solutil)
