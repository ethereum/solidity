# Solidity Commons Library (Solidity related sharing bits between libsolidity and libyul)
set(sources
	Common.h
	CharStream.cpp
	CharStream.h
	ErrorReporter.cpp
	ErrorReporter.h
	EVMVersion.h
	EVMVersion.cpp
	Exceptions.cpp
	Exceptions.h
	ParserBase.cpp
	ParserBase.h
	Scanner.cpp
	Scanner.h
	SemVerHandler.cpp
	SemVerHandler.h
	SourceLocation.h
	SourceReferenceExtractor.cpp
	SourceReferenceExtractor.h
	SourceReferenceFormatter.cpp
	SourceReferenceFormatter.h
	SourceReferenceFormatterHuman.cpp
	SourceReferenceFormatterHuman.h
	Token.cpp
	Token.h
	UndefMacros.h
)

add_library(langutil ${sources})
target_link_libraries(langutil PUBLIC devcore)
