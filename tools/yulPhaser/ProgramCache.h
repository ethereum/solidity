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

#include <tools/yulPhaser/Program.h>

#include <map>
#include <string>

namespace solidity::phaser
{

/**
 * Structure used by @a ProgramCache to store intermediate programs and metadata associated
 * with them.
 */
struct CacheEntry
{
	Program program;
	size_t roundNumber;

	CacheEntry(Program _program, size_t _roundNumber):
		program(std::move(_program)),
		roundNumber(_roundNumber) {}
};

/**
 * Stores statistics about current cache usage.
 */
struct CacheStats
{
	size_t hits;
	size_t misses;
	size_t totalCodeSize;
	std::map<size_t, size_t> roundEntryCounts;

	CacheStats& operator+=(CacheStats const& _other);
	CacheStats operator+(CacheStats const& _other) const { return CacheStats(*this) += _other; }

	bool operator==(CacheStats const& _other) const;
	bool operator!=(CacheStats const& _other) const { return !(*this == _other); }
};

/**
 * Class that optimises programs one step at a time which allows it to store and later reuse the
 * results of the intermediate steps.
 *
 * The cache keeps track of the current round number and associates newly created entries with it.
 * @a startRound() must be called at the beginning of a round so that entries that are too old
 * can be purged. The current strategy is to store programs corresponding to all possible prefixes
 * encountered in the current and the previous rounds. Entries older than that get removed to
 * conserve memory.
 *
 * @a gatherStats() allows getting statistics useful for determining cache effectiveness.
 *
 * The current strategy does speed things up (about 4:1 hit:miss ratio observed in my limited
 * experiments) but there's room for improvement. We could fit more useful programs in
 * the cache by being more picky about which ones we choose.
 *
 * There is currently no way to purge entries without starting a new round. Since the programs
 * take a lot of memory, this may lead to the cache eating up all the available RAM if sequences are
 * long and programs large. A limiter based on entry count or total program size would be useful.
 */
class ProgramCache
{
public:
	explicit ProgramCache(Program _program):
		m_program(std::move(_program)) {}

	Program optimiseProgram(
		std::string const& _abbreviatedOptimisationSteps,
		size_t _repetitionCount = 1
	);
	void startRound(size_t _nextRoundNumber);
	void clear();

	size_t size() const { return m_entries.size(); }
	Program const* find(std::string const& _abbreviatedOptimisationSteps) const;
	bool contains(std::string const& _abbreviatedOptimisationSteps) const { return find(_abbreviatedOptimisationSteps) != nullptr; }

	CacheStats gatherStats() const;

	std::map<std::string, CacheEntry> const& entries() const { return m_entries; };
	Program const& program() const { return m_program; }
	size_t currentRound() const { return m_currentRound; }

private:
	size_t calculateTotalCachedCodeSize() const;
	std::map<size_t, size_t> countRoundEntries() const;

	// The best matching data structure here would be a trie of chromosome prefixes but since
	// the programs are orders of magnitude larger than the prefixes, it does not really matter.
	// A map should be good enough.
	std::map<std::string, CacheEntry> m_entries;

	Program m_program;
	size_t m_currentRound = 0;
	size_t m_hits = 0;
	size_t m_misses = 0;
};

}
