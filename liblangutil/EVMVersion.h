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
#include <boost/operators.hpp>

namespace langutil
{

/**
 * A version specifier of the EVM we want to compile to.
 * Defaults to the latest version deployed on Ethereum mainnet at the time of compiler release.
 */
class EVMVersion:
	boost::less_than_comparable<EVMVersion>,
	boost::equality_comparable<EVMVersion>
{
public:
	EVMVersion() = default;

	static EVMVersion homestead() { return {Version::Homestead}; }
	static EVMVersion tangerineWhistle() { return {Version::TangerineWhistle}; }
	static EVMVersion spuriousDragon() { return {Version::SpuriousDragon}; }
	static EVMVersion byzantium() { return {Version::Byzantium}; }
	static EVMVersion constantinople() { return {Version::Constantinople}; }
	static EVMVersion petersburg() { return {Version::Petersburg}; }

	static boost::optional<EVMVersion> fromString(std::string const& _version)
	{
		for (auto const& v: {homestead(), tangerineWhistle(), spuriousDragon(), byzantium(), constantinople(), petersburg()})
			if (_version == v.name())
				return v;
		return {};
	}

	bool operator==(EVMVersion const& _other) const { return m_version == _other.m_version; }
	bool operator<(EVMVersion const& _other) const { return m_version < _other.m_version; }

	std::string name() const
	{
		switch (m_version)
		{
		case Version::Homestead: return "homestead";
		case Version::TangerineWhistle: return "tangerineWhistle";
		case Version::SpuriousDragon: return "spuriousDragon";
		case Version::Byzantium: return "byzantium";
		case Version::Constantinople: return "constantinople";
		case Version::Petersburg: return "petersburg";
		}
		return "INVALID";
	}

	/// Has the RETURNDATACOPY and RETURNDATASIZE opcodes.
	bool supportsReturndata() const { return *this >= byzantium(); }
	bool hasStaticCall() const { return *this >= byzantium(); }
	bool hasBitwiseShifting() const { return *this >= constantinople(); }
	bool hasCreate2() const { return *this >= constantinople(); }
	bool hasExtCodeHash() const { return *this >= constantinople(); }

	/// Whether we have to retain the costs for the call opcode itself (false),
	/// or whether we can just forward easily all remaining gas (true).
	bool canOverchargeGasForCall() const { return *this >= tangerineWhistle(); }

private:
	enum class Version { Homestead, TangerineWhistle, SpuriousDragon, Byzantium, Constantinople, Petersburg };

	EVMVersion(Version _version): m_version(_version) {}

	Version m_version = Version::Byzantium;
};


}
