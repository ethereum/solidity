set(sources
	CHCSmtLib2Interface.cpp
	CHCSmtLib2Interface.h
	Exceptions.h
	SMTLib2Context.cpp
	SMTLib2Context.h
	SMTLib2Interface.cpp
	SMTLib2Interface.h
	SMTLib2Parser.cpp
	SMTLib2Parser.h
	SMTPortfolio.cpp
	SMTPortfolio.h
	SolverInterface.h
	Sorts.cpp
	Sorts.h
	Helpers.h
)


add_library(smtutil ${sources})
target_link_libraries(smtutil PUBLIC solutil Boost::boost)
