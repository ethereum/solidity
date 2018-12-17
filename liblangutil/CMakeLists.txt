# Solidity Commons Library (Solidity related sharing bits between libsolidity and libyul)
set(sources
	CharStream.cpp
	CharStream.h
	ErrorReporter.cpp
	ErrorReporter.h
	EVMVersion.h
	Exceptions.cpp
	Exceptions.h
	ParserBase.cpp
	ParserBase.h
	Scanner.cpp
	Scanner.h
	SourceLocation.h
	SourceReferenceExtractor.cpp
	SourceReferenceExtractor.h
	SourceReferenceFormatter.cpp
	SourceReferenceFormatter.h
	Token.cpp
	Token.h
	UndefMacros.h
)

add_library(langutil ${sources})
target_link_libraries(langutil PUBLIC devcore)
