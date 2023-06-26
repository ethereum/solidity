set(sources
	CHCSmtLib2Interface.cpp
	CHCSmtLib2Interface.h
	Exceptions.h
	SMTLib2Interface.cpp
	SMTLib2Interface.h
	SolverInterface.h
	Sorts.cpp
	Sorts.h
	Helpers.h
)




add_library(smtutil ${sources})
target_link_libraries(smtutil PUBLIC solutil Boost::boost)
