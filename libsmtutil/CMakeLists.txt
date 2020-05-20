set(sources
	CHCSmtLib2Interface.cpp
	CHCSmtLib2Interface.h
	Exceptions.h
	SMTLib2Interface.cpp
	SMTLib2Interface.h
	SMTPortfolio.cpp
	SMTPortfolio.h
	SolverInterface.h
	Sorts.cpp
	Sorts.h
)

if (${Z3_FOUND})
  set(z3_SRCS Z3Interface.cpp Z3Interface.h Z3CHCInterface.cpp Z3CHCInterface.h)
else()
  set(z3_SRCS)
endif()

if (${CVC4_FOUND})
  set(cvc4_SRCS CVC4Interface.cpp CVC4Interface.h)
else()
  set(cvc4_SRCS)
endif()

add_library(smtutil ${sources} ${z3_SRCS} ${cvc4_SRCS})
target_link_libraries(smtutil PUBLIC solutil Boost::boost)

if (${Z3_FOUND})
  target_link_libraries(smtutil PUBLIC z3::libz3)
endif()

if (${CVC4_FOUND})
  target_link_libraries(smtutil PUBLIC CVC4::CVC4)
endif()
