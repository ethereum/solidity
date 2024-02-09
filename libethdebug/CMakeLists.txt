set(sources
        util/CreateNode.h
        translator/detail/PrepareTranslator.h
        translator/Translator.h
        ethdebug.h
        ethdebug.cpp
)

add_library(ethdebug ${sources})
target_link_libraries(ethdebug PUBLIC yul evmasm langutil smtutil solutil Boost::boost fmt::fmt-header-only Threads::Threads)
