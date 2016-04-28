# Should be used to run ctest
# 
# example usage:
# cmake -DETH_TEST_NAME=TestInterfaceStub -DCTEST_COMMAND=/path/to/ctest -P scripts/runtest.cmake

if (NOT CTEST_COMMAND)
	message(FATAL_ERROR "ctest could not be found!")
endif()

# verbosity is off, cause BOOST_MESSAGE is not thread safe and output is a trash
# see https://codecrafter.wordpress.com/2012/11/01/c-unit-test-framework-adapter-part-3/
#
# output might not be usefull cause of thread safety issue 
execute_process(COMMAND ${CTEST_COMMAND} --force-new-ctest-process -C Debug --output-on-failure -j 4 -R "${ETH_TEST_NAME}[.].*")

