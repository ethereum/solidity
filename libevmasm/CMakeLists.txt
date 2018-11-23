set(sources
	Assembly.cpp
	AssemblyItem.cpp
	BlockDeduplicator.cpp
	CommonSubexpressionEliminator.cpp
	ConstantOptimiser.cpp
	ControlFlowGraph.cpp
	ExpressionClasses.cpp
	GasMeter.cpp
	Instruction.cpp
	JumpdestRemover.cpp
	KnownState.cpp
	LinkerObject.cpp
	PathGasMeter.cpp
	PeepholeOptimiser.cpp
	SemanticInformation.cpp
	SimplificationRules.cpp
)

add_library(evmasm ${sources})
target_link_libraries(evmasm PUBLIC devcore)
