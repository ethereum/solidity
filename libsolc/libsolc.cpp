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
// SPDX-License-Identifier: GPL-3.0
/**
 * @author Christian <c@ethdev.com>
 * @date 2014
 * Public compiler API.
 */

#include <libsolc/libsolc.h>
#include <libsolidity/interface/StandardCompiler.h>
#include <libsolidity/interface/Version.h>
#include <libsolidity/lsp/LanguageServer.h>
#include <libsolidity/lsp/Transport.h>
#include <libyul/YulString.h>

#include <cstdlib>
#include <list>
#include <string>

#include "license.h"

using namespace std;
using namespace solidity;
using namespace solidity::util;

using solidity::frontend::ReadCallback;
using solidity::frontend::StandardCompiler;

namespace
{

// The strings in this list must not be resized after they have been added here (via solidity_alloc()), because
// this may potentially change the pointer that was passed to the caller from solidity_alloc().
static list<string> solidityAllocations;

/// Find the equivalent to @p _data in the list of allocations of solidity_alloc(),
/// removes it from the list and returns its value.
///
/// If any invalid argument is being passed, it is considered a programming error
/// on the caller-side and hence, will call abort() then.
string takeOverAllocation(char const* _data)
{
	for (auto iter = begin(solidityAllocations); iter != end(solidityAllocations); ++iter)
		if (iter->data() == _data)
		{
			string chunk = move(*iter);
			solidityAllocations.erase(iter);
			return chunk;
		}

	abort();
}

/// Resizes a std::string to the proper length based on the occurrence of a zero terminator.
void truncateCString(string& _data)
{
	size_t pos = _data.find('\0');
	if (pos != string::npos)
		_data.resize(pos);
}

ReadCallback::Callback wrapReadCallback(CStyleReadFileCallback _readCallback, void* _readContext)
{
	ReadCallback::Callback readCallback;
	if (_readCallback)
	{
		readCallback = [=](string const& _kind, string const& _data)
		{
			char* contents_c = nullptr;
			char* error_c = nullptr;
			_readCallback(_readContext, _kind.data(), _data.data(), &contents_c, &error_c);
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
				result.responseOrErrorMessage = takeOverAllocation(contents_c);
			}
			if (error_c)
			{
				result.success = false;
				result.responseOrErrorMessage = takeOverAllocation(error_c);
			}
			truncateCString(result.responseOrErrorMessage);
			return result;
		};
	}
	return readCallback;
}

string compile(string _input, CStyleReadFileCallback _readCallback, void* _readContext)
{
	StandardCompiler compiler(wrapReadCallback(_readCallback, _readContext));
	return compiler.compile(move(_input));
}

unique_ptr<lsp::MockTransport> languageServerTransport;
unique_ptr<lsp::LanguageServer> languageServer;

}

extern "C"
{
extern char const* solidity_license() noexcept
{
	static string fullLicenseText = otherLicenses + licenseText;
	return fullLicenseText.c_str();
}

extern char const* solidity_version() noexcept
{
	return frontend::VersionString.c_str();
}

extern char* solidity_compile(char const* _input, CStyleReadFileCallback _readCallback, void* _readContext) noexcept
{
	return solidityAllocations.emplace_back(compile(_input, _readCallback, _readContext)).data();
}

extern int solidity_lsp_start(CStyleReadFileCallback /*TODO(pr) _readCallback*/, void* /*_readContext*/) noexcept
{
	if (languageServer || languageServerTransport)
		return -1;

	languageServerTransport = make_unique<lsp::MockTransport>();
	languageServer = make_unique<lsp::LanguageServer>(*languageServerTransport);
	return 0;
}

extern int solidity_lsp_send(char const* _input) noexcept
{
	if (!languageServer)
		return -1;

	if (languageServer->isRunning())
		return -2;

	solAssert(languageServerTransport, "");

	std::string errors{};
	Json::Value jsonMessage{};
	if (!jsonParseStrict(_input, jsonMessage, &errors))
		return -3;

	languageServerTransport->appendInput(jsonMessage);
	languageServer->runIteration();
	return 0;
}

extern char const* solidity_try_receive() noexcept
{
	if (!languageServerTransport)
		return "";

	optional<Json::Value> messageJson = languageServerTransport->popOutput();
	if (!messageJson)
		return "";

	return solidityAllocations.emplace_back(jsonPrettyPrint(messageJson.value())).data();
}

extern char const* solidity_lsp_send_receive(char const* _input) noexcept
{
	if (solidity_lsp_send(_input) < 0)
		return "";

	return solidity_try_receive();
}

extern char* solidity_alloc(size_t _size) noexcept
{
	try
	{
		return solidityAllocations.emplace_back(_size, '\0').data();
	}
	catch (...)
	{
		// most likely a std::bad_alloc(), if at all.
		return nullptr;
	}
}

extern void solidity_free(char* _data) noexcept
{
	takeOverAllocation(_data);
}

extern void solidity_reset() noexcept
{
	// This is called right before each compilation, but not at the end, so additional memory
	// can be freed here.
	yul::YulStringRepository::reset();
	solidityAllocations.clear();
	languageServer.reset();
	languageServerTransport.reset();
}
}
