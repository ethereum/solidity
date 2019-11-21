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
/**
 * @author Christian <c@ethdev.com>
 * @date 2014
 * Public compiler API.
 */

#include <libsolc/libsolc.h>
#include <libsolidity/interface/StandardCompiler.h>
#include <libsolidity/interface/Version.h>
#include <libyul/YulString.h>
#include <libdevcore/Common.h>
#include <libdevcore/JSON.h>

#include <string>

#include "license.h"

using namespace std;
using namespace dev;
using namespace solidity;

namespace
{

ReadCallback::Callback wrapReadCallback(CStyleReadFileCallback _readCallback = nullptr)
{
	ReadCallback::Callback readCallback;
	if (_readCallback)
	{
		readCallback = [=](string const& _path)
		{
			char* contents_c = nullptr;
			char* error_c = nullptr;
			_readCallback(_path.c_str(), &contents_c, &error_c);
			ReadCallback::Result result;
			result.success = true;
			if (!contents_c && !error_c)
			{
				result.success = false;
				result.responseOrErrorMessage = "Callback not supported.";
			}
			if (contents_c)
			{
				result.success = true;
				result.responseOrErrorMessage = string(contents_c);
				free(contents_c);
			}
			if (error_c)
			{
				result.success = false;
				result.responseOrErrorMessage = string(error_c);
				free(error_c);
			}
			return result;
		};
	}
	return readCallback;
}

string compile(string _input, CStyleReadFileCallback _readCallback = nullptr)
{
	StandardCompiler compiler(wrapReadCallback(_readCallback));
	return compiler.compile(std::move(_input));
}

}

static string s_outputBuffer;

extern "C"
{
extern char const* solidity_license() noexcept
{
	static string fullLicenseText = otherLicenses + licenseText;
	return fullLicenseText.c_str();
}
extern char const* solidity_version() noexcept
{
	return VersionString.c_str();
}
extern char const* solidity_compile(char const* _input, CStyleReadFileCallback _readCallback) noexcept
{
	s_outputBuffer = compile(_input, _readCallback);
	return s_outputBuffer.c_str();
}
extern void solidity_free() noexcept
{
	// This is called right before each compilation, but not at the end, so additional memory
	// can be freed here.
	yul::YulStringRepository::reset();
	s_outputBuffer.clear();
}
}
