/*
	This file is part of solidity.

	solidity is free software: you can redistribute it and/or modify
	it under the terms of the GNU General Public License as published by
	the Free Software Foundation, either version 3 of the License, or
	(at your option) any later version.

	solidity is distributed in the hope that it will be useful,
	but WITHOUT ANY WARRANTY; without even the implied warranty of
	MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
	GNU General Public License for more details.

	You should have received a copy of the GNU General Public License
	along with solidity.  If not, see <http://www.gnu.org/licenses/>.
*/
/** @file TestHelper.h
 * @author Marko Simovic <markobarko@gmail.com>
 * @date 2014
 */

#pragma once

#include <functional>
#include <boost/test/unit_test.hpp>
#include <boost/filesystem.hpp>
#include <boost/version.hpp>

namespace dev
{
namespace test
{

#if (BOOST_VERSION >= 105900)
#define ETH_BOOST_CHECK_IMPL(_message, _requireOrCheck) BOOST_TEST_TOOL_DIRECT_IMPL( \
		false,															\
		_requireOrCheck,												\
		_message														\
	)
#else
#define ETH_BOOST_CHECK_IMPL(_message, _requireOrCheck) BOOST_CHECK_IMPL( \
		false,															\
		_message,														\
		_requireOrCheck,												\
		CHECK_MSG														\
	)
#endif

/// Make sure that no Exception is thrown during testing. If one is thrown show its info and fail the test.
/// Our version of BOOST_REQUIRE_NO_THROW()
/// @param _statement     The statement for which to make sure no exceptions are thrown
/// @param _message       A message to act as a prefix to the expression's error information
#define ETH_TEST_REQUIRE_NO_THROW(_statement, _message)				\
	do																	\
	{																	\
		try															\
		{																\
			BOOST_TEST_PASSPOINT();										\
			_statement;												\
		}																\
		catch (boost::exception const& _e)								\
		{																\
			auto msg = std::string(_message " due to an exception thrown by " \
				BOOST_STRINGIZE(_statement) "\n") + boost::diagnostic_information(_e); \
			ETH_BOOST_CHECK_IMPL(msg, REQUIRE);							\
		}																\
		catch (...)														\
		{																\
			ETH_BOOST_CHECK_IMPL(										\
				"Unknown exception thrown by " BOOST_STRINGIZE(_statement),	\
				REQUIRE													\
			);															\
		}																\
	}																	\
	while (0)

/// Check if an Exception is thrown during testing. If one is thrown show its info and continue the test
/// Our version of BOOST_CHECK_NO_THROW()
/// @param _statement    The statement for which to make sure no exceptions are thrown
/// @param _message       A message to act as a prefix to the expression's error information
#define ETH_TEST_CHECK_NO_THROW(_statement, _message)					\
	do																	\
	{																	\
		try															\
		{																\
			BOOST_TEST_PASSPOINT();										\
			_statement;												\
		}																\
		catch (boost::exception const& _e)								\
		{																\
			auto msg = std::string(_message " due to an exception thrown by " \
				BOOST_STRINGIZE(_statement) "\n") + boost::diagnostic_information(_e); \
			ETH_BOOST_CHECK_IMPL(msg, CHECK);							\
		}																\
		catch (...)														\
		{																\
			ETH_BOOST_CHECK_IMPL(										\
				"Unknown exception thrown by " BOOST_STRINGIZE(_statement),	\
				CHECK													\
			);															\
		}																\
	}																	\
	while (0)


struct Options: boost::noncopyable
{
	std::string ipcPath;
	bool showMessages = false;
	bool optimize = false;
	bool disableIPC = false;

	static Options const& get();

private:
	Options();
};

}
}
