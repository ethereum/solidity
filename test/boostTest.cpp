/*
	This file is part of cpp-ethereum.

	cpp-ethereum is free software: you can redistribute it and/or modify
	it under the terms of the GNU General Public License as published by
	the Free Software Foundation, either version 3 of the License, or
	(at your option) any later version.

	cpp-ethereum is distributed in the hope that it will be useful,
	but WITHOUT ANY WARRANTY; without even the implied warranty of
	MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
	GNU General Public License for more details.

	You should have received a copy of the GNU General Public License
	along with cpp-ethereum.  If not, see <http://www.gnu.org/licenses/>.
*/
/** @file boostTest.cpp
 * @author Marko Simovic <markobarko@gmail.com>
 * @date 2014
 * Stub for generating main boost.test module.
 * Original code taken from boost sources.
 */

#define BOOST_TEST_MODULE EthereumTests
#pragma GCC diagnostic push
#pragma GCC diagnostic ignored "-Wunused-parameter"


	#define BOOST_TEST_NO_MAIN
	#if defined(_MSC_VER)
	#pragma warning(push)
	#pragma warning(disable:4535) // calling _set_se_translator requires /EHa
	#endif
	#include <boost/test/included/unit_test.hpp>
	#if defined(_MSC_VER)
	#pragma warning(pop)
	#endif

	#pragma GCC diagnostic pop

	#include <stdlib.h>
	#include <boost/version.hpp>
	#include "TestHelper.h"

	using namespace boost::unit_test;

	std::vector<char*> parameters;
	static std::ostringstream strCout;
	std::streambuf* oldCoutStreamBuf;
	std::streambuf* oldCerrStreamBuf;

	//Custom Boost Initialization
	test_suite* fake_init_func(int argc, char* argv[])
	{
		//Required for boost. -nowarning
		(void)argc;
		(void)argv;
		return 0;
	}

	//Custom Boost Unit Test Main
	int main(int argc, char* argv[])
	{
		//Initialize options before boost reads it
		dev::test::Options const& opt = dev::test::Options::get(argc, argv);
		return unit_test_main(fake_init_func, opt.tArgc, opt.tArgv);
	}

	/*
#else
	#if defined(_MSC_VER)
	#pragma warning(push)
	#pragma warning(disable:4535) // calling _set_se_translator requires /EHa
	#endif
	#include <boost/test/included/unit_test.hpp>
	#if defined(_MSC_VER)
	#pragma warning(pop)
	#endif

	#pragma GCC diagnostic pop

	#include <test/TestHelper.h>
	using namespace boost::unit_test;
#endif*/