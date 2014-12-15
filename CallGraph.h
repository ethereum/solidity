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
 * @author Christian <c@ethdev.com>
 * @date 2014
 * Callgraph of functions inside a contract.
 */

#include <set>
#include <queue>
#include <boost/range/iterator_range.hpp>
#include <libsolidity/ASTVisitor.h>

namespace dev {
namespace solidity {

/**
 * Can be used to compute the graph of calls (or rather references) between functions of the same
 * contract. Current functionality is limited to computing all functions that are directly
 * or indirectly called by some functions.
 */
class CallGraph: private ASTConstVisitor
{
public:
	void addFunction(FunctionDefinition const& _function);
	void computeCallGraph();

	std::set<FunctionDefinition const*> const& getCalls();

private:
	void addFunctionToQueue(FunctionDefinition const& _function);
	virtual bool visit(Identifier const& _identifier) override;

	std::set<FunctionDefinition const*> m_functionsSeen;
	std::queue<FunctionDefinition const*> m_workQueue;
};

}
}
