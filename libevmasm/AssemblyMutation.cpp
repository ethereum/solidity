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
	if (m_mutate)
		for (auto& mutant : m_mutants)
			mutant.newTag();

	return m_ordinary.newTag();
}

AssemblyItem AssemblyMutation::newPushTag()
{
	if (m_mutate)
		for (auto& mutant : m_mutants)
			mutant.newPushTag();

	return m_ordinary.newPushTag();
}

AssemblyItem AssemblyMutation::newData(bytes const& _data)
{
	if (m_mutate)
		for (auto& mutant : m_mutants)
			mutant.newData(_data);

	return m_ordinary.newData(_data);
}

AssemblyMutation const& AssemblyMutation::sub(size_t _sub) const
{
	AssemblyMutation* mutation = new AssemblyMutation(m_mutate);

	if (m_mutate)
		for (auto& mutant : m_mutants)
			mutation->addMutant(mutant.subMutant(_sub));

	mutation->ordinary(m_ordinary.sub(_sub));
	return *mutation;
}

AssemblyItem const& AssemblyMutation::append(AssemblyItem const& _i)
{
	if (m_mutate)
		for (auto& mutant : m_mutants)
			mutant.append(_i);

	return m_ordinary.append(_i);
}

AssemblyItem const& AssemblyMutation::append(string const& _data)
{
	if (m_mutate)
		for (auto& mutant : m_mutants)
			mutant.append(_data);

	return m_ordinary.append(_data);
}

AssemblyItem const& AssemblyMutation::append(bytes const& _data)
{
	if (m_mutate)
		for (auto& mutant : m_mutants)
			mutant.append(_data);

	return m_ordinary.append(_data);
}

AssemblyItem AssemblyMutation::appendSubSize(AssemblyMutation const& _a)
{
	Assembly const& ordinarySubroutine = _a.ordinary();

	if (m_mutate) {
		for (auto& mutant : m_mutants)
			mutant.appendSubSize(ordinarySubroutine);

		for (auto& mutant : _a.m_mutants)
		{
			Assembly bud = m_ordinary;
			bud.appendSubSize(mutant);

			AssemblyMutant* updated
				= new AssemblyMutant(bud, mutant.description(), mutant.genLocation());

			addMutant(*updated);
		}
	}

	// return AsseblyItem from ordinary since our mutations do not affect offset.
	return m_ordinary.appendSubSize(ordinarySubroutine);
}

void AssemblyMutation::appendProgramSize()
{
	if (m_mutate)
		for (auto& mutant : m_mutants)
			mutant.appendProgramSize();

	m_ordinary.appendProgramSize();
}

void AssemblyMutation::appendLibraryAddress(string const& _identifier)
{
	if (m_mutate)
		for (auto& mutant : m_mutants)
			mutant.appendLibraryAddress(_identifier);

	m_ordinary.appendLibraryAddress(_identifier);
}

AssemblyItem AssemblyMutation::appendJump()
{
	if (m_mutate)
		for (auto& mutant : m_mutants)
			mutant.appendJump();

	return m_ordinary.appendJump();
}

AssemblyItem AssemblyMutation::appendJumpI()
{
	if (m_mutate)
		for (auto& mutant : m_mutants)
			mutant.appendJumpI();

	return m_ordinary.appendJumpI();
}

AssemblyItem AssemblyMutation::appendJump(AssemblyItem const& _tag)
{
	if (m_mutate)
		for (auto& mutant : m_mutants)
			mutant.appendJump(_tag);

	return m_ordinary.appendJump(_tag);
}

AssemblyItem AssemblyMutation::appendJumpI(AssemblyItem const& _tag)
{
	if (m_mutate)
		for (auto& mutant : m_mutants)
			mutant.appendJumpI(_tag);

	return m_ordinary.appendJumpI(_tag);
}

AssemblyItem AssemblyMutation::errorTag()
{
	if (m_mutate)
		for (auto& mutant : m_mutants)
			mutant.errorTag();

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
	// Returns desposit from ordinary since mutation should not influence it.
	return m_ordinary.deposit();
}

void AssemblyMutation::adjustDeposit(int _adjustment)
{
	if (m_mutate)
		for (auto& mutant : m_mutants)
			mutant.adjustDeposit(_adjustment);

	m_ordinary.adjustDeposit(_adjustment);
}

void AssemblyMutation::setDeposit(int _deposit)
{
	if (m_mutate)
		for (auto& mutant : m_mutants)
			mutant.setDeposit(_deposit);

	m_ordinary.setDeposit(_deposit);
}

void AssemblyMutation::setSourceLocation(SourceLocation const& _location)
{
	if (m_mutate)
		for (auto& mutant : m_mutants)
			mutant.setSourceLocation(_location);

	m_ordinary.setSourceLocation(_location);
}

LinkerMutation const& AssemblyMutation::assemble() const
{
	LinkerMutation* mutation = new LinkerMutation();

	if (m_mutate)
		for (auto& mutant : m_mutants)
			mutation->addMutant(mutant.assembleMutant());

	mutation->ordinary(m_ordinary.assemble());
	return *mutation;
}

AssemblyMutation& AssemblyMutation::optimise(bool _enable, bool _isCreation, size_t _runs)
{
	if (m_mutate)
		for (auto& mutant : m_mutants)
			mutant.optimise(_enable, _isCreation, _runs);

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
	// We stream only ordinary to preserve current behaviour.
	return m_ordinary.stream(_out, _prefix, _sourceCodes, _inJsonFormat);
}

void AssemblyMutation::injectVersionStampIntoSub(size_t _subIndex, bytes _version)
{
	if (m_mutate)
		for (auto& mutant : m_mutants)
			sub(mutant, _subIndex, _version);

	sub(m_ordinary, _subIndex, _version);
}

Assembly const& AssemblyMutation::ordinary() const
{
	return m_ordinary;
}

void AssemblyMutation::ordinary(eth::Assembly const& _ordinary)
{
	m_ordinary = _ordinary;
}

void AssemblyMutation::addMutant(eth::AssemblyMutant const& _mutant)
{
	m_mutants.push_back(_mutant);
}

void AssemblyMutation::sub(eth::Assembly& _assembly, size_t _subIndex, bytes _version) const
{
	eth::Assembly& sub = _assembly.sub(_subIndex);
	sub.injectStart(Instruction::POP);
	sub.injectStart(fromBigEndian<u256>(_version));
}
