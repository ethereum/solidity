# Solidity Commons Library (Solidity related sharing bits between libsolidity and libyul)
set(sources
	CharStream.cpp
	ErrorReporter.cpp
	Exceptions.cpp
	ParserBase.cpp
	Scanner.cpp
	Token.cpp
)

add_library(langutil ${sources})
target_link_libraries(langutil PUBLIC devcore)
