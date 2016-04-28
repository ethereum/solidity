# Set necessary compile and link flags

# C++11 check and activation
if ("${CMAKE_CXX_COMPILER_ID}" MATCHES "GNU")

	set(CMAKE_CXX_FLAGS "-std=c++11 -Wall -Wno-unknown-pragmas -Wextra -Werror -pedantic -DSHAREDLIB -fPIC ${CMAKE_CXX_FLAGS}")
	if (GCC_VERSION VERSION_GREATER 4.9 OR GCC_VERSION VERSION_EQUAL 4.9)
		set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -fstack-protector-strong -Wstack-protector")
	endif()
	set(CMAKE_CXX_FLAGS_DEBUG          "-O0 -g -DETH_DEBUG")
	set(CMAKE_CXX_FLAGS_MINSIZEREL     "-Os -DNDEBUG -DETH_RELEASE")
	set(CMAKE_CXX_FLAGS_RELEASE        "-O3 -DNDEBUG -DETH_RELEASE")
	set(CMAKE_CXX_FLAGS_RELWITHDEBINFO "-O2 -g -DETH_RELEASE")

	execute_process(
		COMMAND ${CMAKE_CXX_COMPILER} -dumpversion OUTPUT_VARIABLE GCC_VERSION)
	if (NOT (GCC_VERSION VERSION_GREATER 4.7 OR GCC_VERSION VERSION_EQUAL 4.7))
		message(FATAL_ERROR "${PROJECT_NAME} requires g++ 4.7 or greater.")
	endif ()

elseif ("${CMAKE_CXX_COMPILER_ID}" MATCHES "Clang")

	set(CMAKE_CXX_FLAGS -std=c++11)
	add_compile_options(-Wall)
	add_compile_options(-Wextra)
	add_compile_options(-Wno-unknown-pragmas)
	add_compile_options(-Wno-unused-function)			# Needed for sepc256k1
	add_compile_options(-Wno-dangling-else)				# (clog, cwarn) macros
	add_compile_options(-fPIC)
	add_compile_options(-fstack-protector-strong)
	add_compile_options(-fstack-protector)
	add_definitions(-DSHAREDLIB)

	# Playing it safe, by only enabling warnings-as-errors on OS X for now,
	# not for Clang on Linux and other platforms.
	if ("${CMAKE_CXX_COMPILER_ID}" MATCHES "AppleClang")
		add_compile_options(-Werror)
	endif ()

	if ("${CMAKE_SYSTEM_NAME}" MATCHES "Linux")
		add_compile_options(-stdlib=libstdc++)
		add_compile_options(-fcolor-diagnostics)
		add_compile_options(-Qunused-arguments)
		add_definitions(-DBOOST_ASIO_HAS_CLANG_LIBCXX)
	endif()

	if (EMSCRIPTEN)
		set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} --memory-init-file 0 -O3 -s LINKABLE=1 -s DISABLE_EXCEPTION_CATCHING=0 -s NO_EXIT_RUNTIME=1 -s ALLOW_MEMORY_GROWTH=1 -s NO_DYNAMIC_EXECUTION=1")
		add_definitions(-DETH_EMSCRIPTEN=1)
	endif()

	set(CMAKE_CXX_FLAGS_DEBUG          "-O0 -g -DETH_DEBUG")
	set(CMAKE_CXX_FLAGS_MINSIZEREL     "-Os -DNDEBUG -DETH_RELEASE")
	set(CMAKE_CXX_FLAGS_RELEASE        "-O3 -DNDEBUG -DETH_RELEASE")
	set(CMAKE_CXX_FLAGS_RELWITHDEBINFO "-O2 -g -DETH_RELEASE")

elseif (DEFINED MSVC)

    add_compile_options(/MP)						# enable parallel compilation
	add_compile_options(/EHsc)						# specify Exception Handling Model in msvc
	add_compile_options(/WX)						# enable warnings-as-errors
	add_compile_options(/wd4068)					# disable unknown pragma warning (4068)
	add_compile_options(/wd4996)					# disable unsafe function warning (4996)
	add_compile_options(/wd4503)					# disable decorated name length exceeded, name was truncated (4503)
	add_compile_options(/wd4267)					# disable conversion from 'size_t' to 'type', possible loss of data (4267)
	add_compile_options(/wd4180)					# disable qualifier applied to function type has no meaning; ignored (4180)
	add_compile_options(/wd4290)					# disable C++ exception specification ignored except to indicate a function is not __declspec(nothrow) (4290)
	add_compile_options(/wd4244)					# disable conversion from 'type1' to 'type2', possible loss of data (4244)
	add_compile_options(/wd4800)					# disable forcing value to bool 'true' or 'false' (performance warning) (4800)
	add_compile_options(-D_WIN32_WINNT=0x0600)		# declare Windows Vista API requirement
	add_compile_options(-DNOMINMAX)					# undefine windows.h MAX && MIN macros cause it cause conflicts with std::min && std::max functions
	add_compile_options(-DMINIUPNP_STATICLIB)		# define miniupnp static library

	# disable empty object file warning
	set(CMAKE_STATIC_LINKER_FLAGS "${CMAKE_STATIC_LINKER_FLAGS} /ignore:4221")
	# warning LNK4075: ignoring '/EDITANDCONTINUE' due to '/SAFESEH' specification
	# warning LNK4099: pdb was not found with lib
	# stack size 16MB
	set(CMAKE_EXE_LINKER_FLAGS "${CMAKE_EXE_LINKER_FLAGS} /ignore:4099,4075 /STACK:16777216")

	# windows likes static
	if (NOT ETH_STATIC)
		message("Forcing static linkage for MSVC.")
		set(ETH_STATIC 1)
	endif ()
else ()
	message(WARNING "Your compiler is not tested, if you run into any issues, we'd welcome any patches.")
endif ()

if (SANITIZE)
	set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -fno-omit-frame-pointer -fsanitize=${SANITIZE}")
	if (${CMAKE_CXX_COMPILER_ID} MATCHES "Clang")
		set(CMAKE_CXX_FLAGS  "${CMAKE_CXX_FLAGS} -fsanitize-blacklist=${CMAKE_SOURCE_DIR}/sanitizer-blacklist.txt")
	endif()
endif()

if (PROFILING AND (("${CMAKE_CXX_COMPILER_ID}" MATCHES "GNU") OR ("${CMAKE_CXX_COMPILER_ID}" MATCHES "Clang")))
	set(CMAKE_CXX_FLAGS "-g ${CMAKE_CXX_FLAGS}")
	set(CMAKE_C_FLAGS "-g ${CMAKE_C_FLAGS}")
	add_definitions(-DETH_PROFILING_GPERF)
	set(CMAKE_SHARED_LINKER_FLAGS "${CMAKE_SHARED_LINKER_FLAGS} -lprofiler")
#	set(CMAKE_STATIC_LINKER_FLAGS "${CMAKE_STATIC_LINKER_FLAGS} -lprofiler")
	set(CMAKE_EXE_LINKER_FLAGS "${CMAKE_EXE_LINKER_FLAGS} -lprofiler")
endif ()

if (PROFILING AND (("${CMAKE_CXX_COMPILER_ID}" MATCHES "GNU")))
        set(CMAKE_CXX_FLAGS "-g --coverage ${CMAKE_CXX_FLAGS}")
        set(CMAKE_C_FLAGS "-g --coverage ${CMAKE_C_FLAGS}")
        set(CMAKE_SHARED_LINKER_FLAGS "--coverage ${CMAKE_SHARED_LINKER_FLAGS} -lprofiler")
        set(CMAKE_EXE_LINKER_FLAGS "--coverage ${CMAKE_EXE_LINKER_FLAGS} -lprofiler")
endif ()

if (("${CMAKE_CXX_COMPILER_ID}" MATCHES "GNU") OR ("${CMAKE_CXX_COMPILER_ID}" MATCHES "Clang"))
	option(USE_LD_GOLD "Use GNU gold linker" ON)
	if (USE_LD_GOLD)
		execute_process(COMMAND ${CMAKE_C_COMPILER} -fuse-ld=gold -Wl,--version ERROR_QUIET OUTPUT_VARIABLE LD_VERSION)
		if ("${LD_VERSION}" MATCHES "GNU gold")
			set(CMAKE_C_FLAGS "${CMAKE_C_FLAGS} -fuse-ld=gold")
			set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -fuse-ld=gold")
		endif ()
	endif ()
endif ()

if(ETH_STATIC)
	set(BUILD_SHARED_LIBS OFF)
else()
	set(BUILD_SHARED_LIBS ON)
endif(ETH_STATIC)
