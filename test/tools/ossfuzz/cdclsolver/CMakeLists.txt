set(sources
		ConstraintGenerator.cpp
		ConstraintGenerator.h
		FuzzerSolverInterface.cpp
		FuzzerSolverInterface.h
)
add_library(cdclsolvergen)
target_sources(cdclsolvergen PUBLIC	${sources})
target_link_libraries(cdclsolvergen PUBLIC solutil)
