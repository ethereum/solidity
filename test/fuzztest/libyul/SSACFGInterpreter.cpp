#include <test/fuzztest/libyul/SSACFGInterpreter.h>

#include "libsolutil/Visitor.h"

#include <libyul/backends/evm/ssa/ControlFlowGraphs.h>

using namespace solidity;
using namespace solidity::yul::ssa;
using namespace solidity::yul::test;


SSACFGInterpreter::SSACFGInterpreter(ControlFlowGraphs const& _cfgs): m_cfgs(_cfgs){}

std::vector<solidity::u256> SSACFGInterpreter::runFunction(SSACFG const& _function, std::vector<u256> const& _arguments)
{
	// todo assert m_cfgs.functionGraphs contains _function
	yulAssert(_function.arguments.size() == _arguments.size(), "Arguments must have the same size");
	Frame frame(_function.numInsts());
	for (std::size_t i = 0; i < _arguments.size(); ++i)
	{
		InstId const& arg = _function.arguments[i];
		yulAssert(arg.hasValue());
		yulAssert(_function.isFunctionArg(arg), "Graph argument is not a FunctionArg instruction.");
		frame.setValue(arg, _arguments[i]);
	}

	BlockId currentBlockId = _function.entry;
	yulAssert(currentBlockId.hasValue());

	while (currentBlockId.hasValue())
	{
		SSACFG::BasicBlock const& block = _function.block(currentBlockId);
		// todo execute instructions

		std::optional<std::vector<u256>> result;
		std::visit(util::GenericVisitor{
			[&](SSACFG::BasicBlock::MainExit const&) { result = std::vector<u256>{}; },
			[&](SSACFG::BasicBlock::Jump const& _jump) { currentBlockId = _jump.target; },
			[&](SSACFG::BasicBlock::ConditionalJump const& _jump) {
				currentBlockId = frame.value(_jump.condition) != 0 ? _jump.nonZero : _jump.zero;
			},
			[&](SSACFG::BasicBlock::FunctionReturn const& _return) {
				std::vector<u256> values;
				for (InstId const id: _return.returnValues)
					values.push_back(frame.value(id));
				result = std::move(values);
			},
			[&](SSACFG::BasicBlock::Terminated const&) {
				yulAssert(
					false,
					fmt::format(
						"Reached the Terminated exit of block #{} without execution having been terminated.",
						currentBlockId.value
					)
				);
			}
		}, block.exit);

		if (result.has_value())
		{
			return *result;
		}
	}

	return {};
}
u256 const& SSACFGInterpreter::Frame::value(InstId _id) const
{
	yulAssert(_id.hasValue(), "Use of an empty InstId.");
	yulAssert(_id.value < m_values.size(), fmt::format("InstId {} out of range.", _id.value));
	auto const& slot = m_values[_id.value];
	yulAssert(slot.has_value(), fmt::format("Use of value v{} before it was defined.", _id.value));
	return *slot;
}
void SSACFGInterpreter::Frame::setValue(InstId _id, u256 const& _value)
{
	yulAssert(_id.value < m_values.size(), fmt::format("InstId {} out of range.", _id.value));
	m_values[_id.value] = _value;
}
