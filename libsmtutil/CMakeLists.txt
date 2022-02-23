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
	Helpers.h
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

if (${USE_Z3_DLOPEN})
  file(GLOB Z3_HEADERS ${Z3_HEADER_PATH}/z3*.h)
  set(Z3_WRAPPER ${CMAKE_CURRENT_BINARY_DIR}/z3wrapper.cpp)
  add_custom_command(
    OUTPUT ${Z3_WRAPPER}
    COMMAND ${Python3_EXECUTABLE} genz3wrapper.py ${Z3_HEADERS} > ${Z3_WRAPPER}
    DEPENDS ${Z3_HEADERS} genz3wrapper.py
    WORKING_DIRECTORY ${CMAKE_CURRENT_SOURCE_DIR}
  )
  set(z3_SRCS ${z3_SRCS} ${Z3_WRAPPER} Z3Loader.cpp Z3Loader.h)
endif()

add_library(smtutil ${sources} ${z3_SRCS} ${cvc4_SRCS})
target_link_libraries(smtutil PUBLIC solutil Boost::boost)

if (${USE_Z3_DLOPEN})
  target_include_directories(smtutil PUBLIC ${Z3_HEADER_PATH})
  target_link_libraries(smtutil PUBLIC ${CMAKE_DL_LIBS})
elseif (${Z3_FOUND})
  target_link_libraries(smtutil PUBLIC z3::libz3)
endif()

if (${CVC4_FOUND})
  target_link_libraries(smtutil PUBLIC CVC4::CVC4)
endif()
