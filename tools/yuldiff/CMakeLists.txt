add_library(libyuldiff
	ASTComparator.cpp
	ASTComparator.h
	ScopeBimap.h
)
target_link_libraries(libyuldiff PUBLIC solidity)

add_executable(yuldiff main.cpp)
target_link_libraries(yuldiff PRIVATE libyuldiff)
