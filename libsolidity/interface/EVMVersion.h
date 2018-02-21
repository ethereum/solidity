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
 * EVM versioning.
 */

#pragma once

#include <string>

#include <boost/optional.hpp>

namespace dev
{
namespace solidity
{

/**
 * A version specifier of the EVM we want to compile to.
 * Defaults to the latest version.
 */
class EVMVersion
{
public:
	EVMVersion() {}

	static EVMVersion homestead() { return {Version::Homestead}; }
	static EVMVersion byzantium() { return {Version::Byzantium}; }

	static boost::optional<EVMVersion> fromString(std::string const& _version)
	{
		if (_version == "homestead")
			return homestead();
		else if (_version == "byzantium")
			return byzantium();
		else
			return {};
	}

	bool operator==(EVMVersion const& _other) const { return m_version == _other.m_version; }
	bool operator!=(EVMVersion const& _other) const { return !this->operator==(_other); }

	std::string name() const { return m_version == Version::Byzantium ? "byzantium" : "homestead"; }

	/// Has the RETURNDATACOPY and RETURNDATASIZE opcodes.
	bool supportsReturndata() const { return *this >= byzantium(); }
	bool hasStaticCall() const { return *this >= byzantium(); }

	/// Whether we have to retain the costs for the call opcode itself (false),
	/// or whether we can just forward easily all remaining gas (true).
	bool canOverchargeGasForCall() const
	{
		// @TODO when exactly was this introduced? Was together with the call stack fix.
		return m_version == Version::Byzantium;
	}

private:
	enum class Version { Homestead, Byzantium };

	EVMVersion(Version _version): m_version(_version) {}

	Version m_version = Version::Byzantium;
};


}
}
