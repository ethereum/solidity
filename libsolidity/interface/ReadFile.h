// SPDX-License-Identifier: GPL-3.0

#pragma once

#include <liblangutil/Exceptions.h>

#include <boost/noncopyable.hpp>
#include <functional>
#include <string>

namespace solidity::frontend
{

class ReadCallback: boost::noncopyable
{
public:
	/// File reading or generic query result.
	struct Result
	{
		bool success;
		std::string responseOrErrorMessage;
	};

	enum class Kind
	{
		ReadFile,
		SMTQuery
	};

	static std::string kindString(Kind _kind)
	{
		switch (_kind)
		{
		case Kind::ReadFile:
			return "source";
		case Kind::SMTQuery:
			return "smt-query";
		default:
			solAssert(false, "");
		}
	}

	/// File reading or generic query callback.
	using Callback = std::function<Result(std::string const&, std::string const&)>;
};

}
