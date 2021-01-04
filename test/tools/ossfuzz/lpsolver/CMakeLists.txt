set(sources
  ConstraintGenerator.cpp
  ConstraintGenerator.h
  FuzzerSolverInterface.cpp
  FuzzerSolverInterface.h
)
add_library(lpsolvergen)
target_sources(lpsolvergen
  PUBLIC
  ${sources}
)
target_link_libraries(lpsolvergen PUBLIC solutil)
