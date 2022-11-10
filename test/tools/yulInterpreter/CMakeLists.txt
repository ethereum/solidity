set(sources
	EVMInstructionInterpreter.h
	EVMInstructionInterpreter.cpp
	EwasmBuiltinInterpreter.h
	EwasmBuiltinInterpreter.cpp
	Interpreter.h
	Interpreter.cpp
	Inspector.h
	Inspector.cpp
)

add_library(yulInterpreter ${sources})
target_link_libraries(yulInterpreter PUBLIC yul solidity solutil)
