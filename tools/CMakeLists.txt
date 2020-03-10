add_executable(solidity-upgrade
    solidityUpgrade/main.cpp
    solidityUpgrade/UpgradeChange.h
    solidityUpgrade/UpgradeChange.cpp
    solidityUpgrade/UpgradeSuite.h
    solidityUpgrade/Upgrade050.cpp
    solidityUpgrade/Upgrade060.cpp
    solidityUpgrade/SourceTransform.h
    solidityUpgrade/SourceUpgrade.cpp
)
target_link_libraries(solidity-upgrade PRIVATE solidity Boost::boost Boost::program_options Boost::system)

include(GNUInstallDirs)
install(TARGETS solidity-upgrade DESTINATION "${CMAKE_INSTALL_BINDIR}")

add_executable(yul-phaser
	yulPhaser/main.cpp
	yulPhaser/GeneticAlgorithms.h
	yulPhaser/GeneticAlgorithms.cpp
	yulPhaser/Population.h
	yulPhaser/Population.cpp
	yulPhaser/FitnessMetrics.h
	yulPhaser/FitnessMetrics.cpp
	yulPhaser/Chromosome.h
	yulPhaser/Chromosome.cpp
	yulPhaser/Selections.h
	yulPhaser/Selections.cpp
	yulPhaser/Program.h
	yulPhaser/Program.cpp
	yulPhaser/SimulationRNG.h
	yulPhaser/SimulationRNG.cpp
)
target_link_libraries(yul-phaser PRIVATE solidity Boost::program_options)

install(TARGETS yul-phaser DESTINATION "${CMAKE_INSTALL_BINDIR}")
