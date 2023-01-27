include(GNUInstallDirs)

set(libphaser_sources
	yulPhaser/Common.h
	yulPhaser/Common.cpp
	yulPhaser/AlgorithmRunner.h
	yulPhaser/AlgorithmRunner.cpp
	yulPhaser/Phaser.h
	yulPhaser/Phaser.cpp
	yulPhaser/GeneticAlgorithms.h
	yulPhaser/GeneticAlgorithms.cpp
	yulPhaser/Population.h
	yulPhaser/Population.cpp
	yulPhaser/FitnessMetrics.h
	yulPhaser/FitnessMetrics.cpp
	yulPhaser/Chromosome.h
	yulPhaser/Chromosome.cpp
	yulPhaser/Mutations.h
	yulPhaser/Mutations.cpp
	yulPhaser/PairSelections.h
	yulPhaser/PairSelections.cpp
	yulPhaser/Selections.h
	yulPhaser/Selections.cpp
	yulPhaser/ProgramCache.h
	yulPhaser/ProgramCache.cpp
	yulPhaser/Program.h
	yulPhaser/Program.cpp
	yulPhaser/SimulationRNG.h
	yulPhaser/SimulationRNG.cpp
)
add_library(phaser ${libphaser_sources})
target_link_libraries(phaser PUBLIC solidity Boost::boost Boost::program_options)

add_executable(yul-phaser yulPhaser/main.cpp)
target_link_libraries(yul-phaser PRIVATE phaser)

install(TARGETS yul-phaser DESTINATION "${CMAKE_INSTALL_BINDIR}")
