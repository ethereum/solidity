// SPDX-License-Identifier: GPL-3.0
/** @file IsolTestOptions.h
 */

#pragma once

#include <liblangutil/EVMVersion.h>

#include <test/Common.h>

namespace solidity::test
{

struct IsolTestOptions: CommonOptions
{
	bool showHelp = false;
	bool noColor = false;
	std::string testFilter = std::string{};

	IsolTestOptions(std::string* _editor);
	bool parse(int _argc, char const* const* _argv) override;
	void validate() const override;
};

}
