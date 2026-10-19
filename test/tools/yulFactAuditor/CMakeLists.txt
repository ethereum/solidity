set(sources
	FactAuditor.h
	FactAuditor.cpp
)

add_library(yulFactAuditor ${sources})
target_link_libraries(yulFactAuditor PUBLIC yul solutil)
