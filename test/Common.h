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

#pragma once

#include <libsolutil/Exceptions.h>
#include <liblangutil/EVMVersion.h>

#include <boost/filesystem/path.hpp>
#include <boost/noncopyable.hpp>
#include <boost/program_options.hpp>

namespace solidity::test
{

#ifdef _WIN32
static constexpr auto evmoneFilename = "evmone.dll";
static constexpr auto evmoneDownloadLink = "https://github.com/ethereum/evmone/releases/download/v0.4.1/evmone-0.4.1-windows-amd64.zip";
#elif defined(__APPLE__)
static constexpr auto evmoneFilename = "libevmone.dylib";
static constexpr auto evmoneDownloadLink = "https://github.com/ethereum/evmone/releases/download/v0.4.1/evmone-0.4.1-darwin-x86_64.tar.gz";
#else
static constexpr auto evmoneFilename = "libevmone.so";
static constexpr auto evmoneDownloadLink = "https://github.com/ethereum/evmone/releases/download/v0.4.1/evmone-0.4.1-linux-x86_64.tar.gz";
#endif


struct ConfigException : public util::Exception {};

struct CommonOptions: boost::noncopyable
{
	boost::filesystem::path evmonePath;
	std::vector<boost::filesystem::path> vmPaths;
	boost::filesystem::path testPath;
	bool optimize = false;
	bool enforceViaYul = false;
	bool disableSMT = false;
	bool useABIEncoderV2 = false;
	bool showMessages = false;
	bool showMetadata = false;

	langutil::EVMVersion evmVersion() const;

	virtual bool parse(int argc, char const* const* argv);
	// Throws a ConfigException on error
	virtual void validate() const;

	static CommonOptions const& get();
	static void setSingleton(std::unique_ptr<CommonOptions const>&& _instance);

	CommonOptions(std::string caption = "");
	virtual ~CommonOptions() {};
protected:

	boost::program_options::options_description options;

private:
	std::string evmVersionString;
	static std::unique_ptr<CommonOptions const> m_singleton;
};

}
