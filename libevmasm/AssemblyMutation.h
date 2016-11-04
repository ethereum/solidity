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
/**
 * @author Danil Nemirovsky <danil.nemirovsky@gmail.com>
 * @date 2016
 * Assembles bytecode for contract and its mutants.
 */

#pragma once

#include <libevmasm/Assembly.h>
#include <libevmasm/LinkerMutation.h>

namespace dev
{

namespace eth
{

class AssemblyMutation
{
public:
	AssemblyMutation() {}

	AssemblyItem newTag();
	AssemblyItem newPushTag();
	AssemblyItem newData(bytes const& _data);

	AssemblyMutation const& sub(size_t _sub) const;

	AssemblyItem const& append(AssemblyItem const& _i);
	AssemblyItem const& append(std::string const& _data);
	AssemblyItem const& append(bytes const& _data);
	AssemblyItem appendSubSize(Assembly const& _a);
	void appendProgramSize();
	void appendLibraryAddress(std::string const& _identifier);

	AssemblyItem appendJump();
	AssemblyItem appendJumpI();
	AssemblyItem appendJump(AssemblyItem const& _tag);
	AssemblyItem appendJumpI(AssemblyItem const& _tag);
	AssemblyItem errorTag();

	template <class T> AssemblyMutation& operator<<(T const& _d) { append(_d); return *this; }

	AssemblyItems const& items() const;

	int deposit() const;
	void adjustDeposit(int _adjustment);
	void setDeposit(int _deposit);

	void setSourceLocation(SourceLocation const& _location);

	LinkerMutation const& assemble() const;

	AssemblyMutation& optimise(bool _enable, bool _isCreation = true, size_t _runs = 200);
	Json::Value stream(
		std::ostream& _out,
		std::string const& _prefix = "",
		const StringMap &_sourceCodes = StringMap(),
		bool _inJsonFormat = false
	) const;

	void injectVersionStampIntoSub(size_t _subIndex, bytes _version);

	eth::Assembly const& ordinary() const;
	void ordinary(eth::Assembly const& _ordinary);

private:
	eth::Assembly m_ordinary;
	std::map<std::string const, eth::Assembly> m_mutants;

	mutable LinkerMutation m_assembledMutation;
};

}
}
