set(sources
        SolidityBaseVisitor.h
        SolidityVisitor.h
        SolidityMutator.cpp
        SolidityMutator.h
        SolidityParser.cpp
        SolidityParser.h
        SolidityLexer.cpp
        SolidityLexer.h
        )
add_library(antlr4mutator ${sources})
target_include_directories(antlr4mutator PUBLIC
        "${CMAKE_SOURCE_DIR}"
        /usr/local/include/antlr4-runtime
)