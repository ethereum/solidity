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
/** @file AssemblyMutation.cpp
 * @author Danil Nemirovsky <danil.nemirovsky@gmail.com>
 * @date 2016
 */

#include <libevmasm/AssemblyMutation.h>

using namespace dev;
using namespace dev::eth;
using namespace std;

AssemblyItem AssemblyMutation::newTag()
{
    // TODO Iterate over mutants, return AssemblyItem from ordinary
    return m_ordinary.newTag();
}

AssemblyItem AssemblyMutation::newPushTag()
{
    // TODO Iterate over mutants, return AssemblyItem from ordinary
    return m_ordinary.newPushTag();
}

AssemblyItem AssemblyMutation::newData(bytes const& _data)
{
    // TODO Iterate over mutants, return AssemblyItem from ordinary
    return m_ordinary.newData(_data);
}

AssemblyMutation const& AssemblyMutation::sub(size_t _sub) const
{
    // TODO Iterate over mutants, return AssemblyMutation created with subs
    AssemblyMutation* assembly = new AssemblyMutation();
    assembly->ordinary(m_ordinary.sub(_sub));
    return *assembly;
}

AssemblyItem const& AssemblyMutation::append(AssemblyItem const& _i)
{
    // TODO Iterate over mutants, return AssemblyItem from ordinary
    return m_ordinary.append(_i);
}

AssemblyItem const& AssemblyMutation::append(string const& _data)
{
    // TODO Iterate over mutants, return AssemblyItem from ordinary
    return m_ordinary.append(_data);
}

AssemblyItem const& AssemblyMutation::append(bytes const& _data)
{
    // TODO Iterate over mutants, return AssemblyItem from ordinary
    return m_ordinary.append(_data);
}

AssemblyItem AssemblyMutation::appendSubSize(Assembly const& _a)
{
    // TODO Iterate over mutants, return AssemblyItem from ordinary
    return m_ordinary.appendSubSize(_a);
}

void AssemblyMutation::appendProgramSize()
{
    // TODO iterate over mutants
    m_ordinary.appendProgramSize();
}

void AssemblyMutation::appendLibraryAddress(string const& _identifier)
{
    // TODO iterate over mutants
    m_ordinary.appendLibraryAddress(_identifier);
}

AssemblyItem AssemblyMutation::appendJump()
{
    // TODO Iterate over mutants, return AssemblyItem from ordinary
    return m_ordinary.appendJump();
}

AssemblyItem AssemblyMutation::appendJumpI() 
{
    // TODO Iterate over mutants, return AssemblyItem from ordinary
    return m_ordinary.appendJumpI();
}

AssemblyItem AssemblyMutation::appendJump(AssemblyItem const& _tag)
{
    // TODO Iterate over mutants, return AssemblyItem from ordinary
    return m_ordinary.appendJump(_tag);
}

AssemblyItem AssemblyMutation::appendJumpI(AssemblyItem const& _tag)
{
    // TODO Iterate over mutants, return AssemblyItem from ordinary
    return m_ordinary.appendJumpI(_tag);
}

AssemblyItem AssemblyMutation::errorTag() 
{
    // TODO Iterate over mutants, return AssemblyItem from ordinary
    return m_ordinary.errorTag();
}

AssemblyItems const& AssemblyMutation::items() const
{
    // Returns items only from ordinary since it used only on gas estimation
    // and source mapping. Mutation should not influence it.
    return m_ordinary.items();
}

int AssemblyMutation::deposit() const
{
    return m_ordinary.deposit();
}

void AssemblyMutation::adjustDeposit(int _adjustment)
{
    // TODO iterate over mutants
    m_ordinary.adjustDeposit(_adjustment);
}

void AssemblyMutation::setDeposit(int _deposit) 
{
    // TODO iterate over mutants
    m_ordinary.setDeposit(_deposit);
}

void AssemblyMutation::setSourceLocation(SourceLocation const& _location)
{
    // TODO iterate over mutants
    m_ordinary.setSourceLocation(_location);
}

LinkerMutation const& AssemblyMutation::assemble() const
{
    LinkerMutation& mutation = m_assembledMutation;

    // TODO iterate over mutants
    mutation.ordinary(m_ordinary.assemble());

    return mutation;
}

AssemblyMutation& AssemblyMutation::optimise(bool _enable, bool _isCreation, size_t _runs)
{
    // TODO iterate over mutants
    m_ordinary.optimise(_enable, _isCreation, _runs);

    return *this;
}

Json::Value AssemblyMutation::stream(
		std::ostream& _out,
		std::string const& _prefix,
		const StringMap &_sourceCodes,
		bool _inJsonFormat
	) const
{
    // TODO iterate over mutants
    return m_ordinary.stream(_out, _prefix, _sourceCodes, _inJsonFormat);   
}

void AssemblyMutation::injectVersionStampIntoSub(size_t _subIndex, bytes _version)
{
    // TODO iterate over mutants
    eth::Assembly& sub = m_ordinary.sub(_subIndex);
	sub.injectStart(Instruction::POP);
	sub.injectStart(fromBigEndian<u256>(_version));
}

Assembly const& AssemblyMutation::ordinary() const
{   
    return m_ordinary;
}

void AssemblyMutation::ordinary(eth::Assembly const& _ordinary)
{
    m_ordinary = _ordinary;
}
