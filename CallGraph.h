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
#include <functional>
#include <boost/range/iterator_range.hpp>
#include <libsolidity/ASTVisitor.h>

namespace dev
{
namespace solidity
{

/**
 * Can be used to compute the graph of calls (or rather references) between functions of the same
 * contract. Current functionality is limited to computing all functions that are directly
 * or indirectly called by some functions.
 */
class CallGraph: private ASTConstVisitor
{
public:
	using OverrideResolver = std::function<FunctionDefinition const*(std::string const&)>;

	CallGraph(OverrideResolver const& _overrideResolver): m_overrideResolver(&_overrideResolver) {}

	void addNode(ASTNode const& _node);

	std::set<FunctionDefinition const*> const& getCalls();

private:
	virtual bool visit(FunctionDefinition const& _function) override;
	virtual bool visit(Identifier const& _identifier) override;
	virtual bool visit(MemberAccess const& _memberAccess) override;

	void computeCallGraph();
	void addFunction(FunctionDefinition const& _function);

	OverrideResolver const* m_overrideResolver;
	std::set<FunctionDefinition const*> m_functionsSeen;
	std::queue<FunctionDefinition const*> m_workQueue;
};

}
}
