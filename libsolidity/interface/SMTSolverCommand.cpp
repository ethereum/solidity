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
#include <libsolidity/interface/SMTSolverCommand.h>

#include <liblangutil/Exceptions.h>

#include <boost/algorithm/string/join.hpp>
#include <boost/process.hpp>

namespace solidity::frontend
{

void SMTSolverCommand::setEldarica(std::optional<unsigned int> timeoutInMilliseconds, bool computeInvariants)
{
	m_arguments.clear();
	m_solverCmd = "eld";
	m_arguments.emplace_back("-hsmt"); // Tell Eldarica to expect input in SMT2 format
	m_arguments.emplace_back("-in"); // Tell Eldarica to read from standard input
	if (timeoutInMilliseconds)
	{
		unsigned int timeoutInSeconds = timeoutInMilliseconds.value() / 1000u;
		timeoutInSeconds = timeoutInSeconds == 0 ? 1 : timeoutInSeconds;
		m_arguments.push_back("-t:" + std::to_string(timeoutInSeconds));
	}
	if (computeInvariants)
		m_arguments.emplace_back("-ssol"); // Tell Eldarica to produce model (invariant)
}

void SMTSolverCommand::setCvc5(std::optional<unsigned int> timeoutInMilliseconds)
{
	m_arguments.clear();
	m_solverCmd = "cvc5";
	if (timeoutInMilliseconds)
	{
		m_arguments.emplace_back("--tlimit-per");
		m_arguments.push_back(std::to_string(timeoutInMilliseconds.value()));
	}
	else
	{
		m_arguments.emplace_back("--rlimit"); // Set resource limit cvc5 can spend on a query
		m_arguments.push_back(std::to_string(12000));
	}
}

void SMTSolverCommand::setZ3(std::optional<unsigned int> timeoutInMilliseconds, bool _preprocessing, bool _computeInvariants)
{
	constexpr int Z3ResourceLimit = 2000000;
	m_arguments.clear();
	m_solverCmd = "z3";
	m_arguments.emplace_back("-in"); // Read from standard input
	m_arguments.emplace_back("-smt2"); // Expect input in SMT-LIB2 format
	if (_computeInvariants)
		m_arguments.emplace_back("-model"); // Output model automatically after check-sat
	if (timeoutInMilliseconds)
		m_arguments.emplace_back("-t:" + std::to_string(timeoutInMilliseconds.value()));
	else
		m_arguments.emplace_back("rlimit=" + std::to_string(Z3ResourceLimit));

	// These options have been empirically established to be helpful
	m_arguments.emplace_back("rewriter.pull_cheap_ite=true");
	m_arguments.emplace_back("fp.spacer.q3.use_qgen=true");
	m_arguments.emplace_back("fp.spacer.mbqi=false");
	m_arguments.emplace_back("fp.spacer.ground_pobs=false");

	// Spacer optimization should be
	// - enabled for better solving (default)
	// - disable for counterexample generation
	std::string preprocessingArg = _preprocessing ? "true" : "false";
	m_arguments.emplace_back("fp.xform.slice=" + preprocessingArg);
	m_arguments.emplace_back("fp.xform.inline_linear=" + preprocessingArg);
	m_arguments.emplace_back("fp.xform.inline_eager=" + preprocessingArg);
}

ReadCallback::Result SMTSolverCommand::solve(std::string const& _kind, std::string const& _query) const
{
	try
	{
		if (_kind != ReadCallback::kindString(ReadCallback::Kind::SMTQuery))
			solAssert(false, "SMTQuery callback used as callback kind " + _kind);

		if (m_solverCmd.empty())
			return ReadCallback::Result{false, "No solver set."};

		auto solverBin = boost::process::search_path(m_solverCmd);

		if (solverBin.empty())
			return ReadCallback::Result{false, m_solverCmd + " binary not found."};

		auto args = m_arguments;

		boost::process::opstream in;  // input to subprocess written to by the main process
		boost::process::ipstream out; // output from subprocess read by the main process
		boost::process::child solverProcess(
			solverBin,
			args,
			boost::process::std_out > out,
			boost::process::std_in < in,
			boost::process::std_err > boost::process::null
		);

		in << _query << std::flush;
		in.pipe().close();
		in.close();

		std::vector<std::string> data;
		std::string line;
		while (!(out.fail() || out.eof()) && std::getline(out, line))
			if (!line.empty())
				data.push_back(line);

		solverProcess.wait();

		return ReadCallback::Result{true, boost::join(data, "\n")};
	}
	catch (...)
	{
		return ReadCallback::Result{false, "Exception in SMTQuery callback: " + boost::current_exception_diagnostic_information()};
	}
}

}
