#include <test/fuzztest/libyul/SSACFGInterpreter.h>

#include "libsolutil/Visitor.h"

#include <libyul/backends/evm/ssa/ControlFlowGraphs.h>

using namespace solidity;
using namespace solidity::yul::ssa;
using namespace solidity::yul::test;


SSACFGInterpreter::SSACFGInterpreter(ControlFlowGraphs const& _cfgs): m_cfgs(_cfgs){}

std::vector<u256> SSACFGInterpreter::runFunction(SSACFG const& _function, std::vector<u256> const& _arguments)
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
u256 const& SSACFGInterpreter::Frame::value(InstId const _id) const
{
	yulAssert(_id.hasValue(), "Use of an empty InstId.");
	yulAssert(_id.value < m_values.size(), fmt::format("InstId {} out of range.", _id.value));
	auto const& slot = m_values[_id.value];
	yulAssert(slot.has_value(), fmt::format("Use of value v{} before it was defined.", _id.value));
	return *slot;
}

bool SSACFGInterpreter::Frame::hasValue(InstId const _id) const
{
	yulAssert(_id.hasValue(), "Use of an empty InstId.");
	yulAssert(_id.value < m_values.size(), fmt::format("InstId {} out of range.", _id.value));
	return m_values[_id.value].has_value();
}

void SSACFGInterpreter::Frame::setValue(InstId const _id, u256 const& _value)
{
	yulAssert(_id.value < m_values.size(), fmt::format("InstId {} out of range.", _id.value));
	m_values[_id.value] = _value;
}

void SSACFGInterpreter::executeInstruction(SSACFG const& _function, Frame& _frame, InstId const _instId)
{
	SSACFG::Inst const& inst = _function.inst(_instId);
	switch (inst.opcode)
	{
	case InstOpcode::Const:
		_frame.setValue(_instId, _function.literalPayload(_instId));
		break;
	case InstOpcode::Phi:
		// no-op, the phi values are populated by upsilons
		break;
	case InstOpcode::Upsilon:
		yulAssert(inst.inputs.size() == 1, "Upsilon with != 1 inputs.");
		_frame.setValue(_function.upsilonPhi(_instId), _frame.value(inst.inputs.front()));
		break;
	case InstOpcode::BuiltinCall:
		// todo
		break;
	case InstOpcode::Call:
		// todo
		break;
	case InstOpcode::Unreachable:
		yulAssert(false, fmt::format("Executed Unreachable instruction v{}.", _instId));
		break;
	case InstOpcode::FunctionArg:
		yulAssert(_frame.hasValue(_instId), fmt::format("Unbound function argument v{}.", _instId));
		break;
	case InstOpcode::Projection:
	{
		yulAssert(inst.inputs.size() == 1, "Projection with != 1 inputs.");
		InstId const producer = inst.inputs.front();
		std::size_t const index = _function.projectionIndex(_instId);
		yulAssert(_instId.value == producer.value + index);
		yulAssert(_frame.hasValue(_instId), "Called projection inst without the generating parent instruction");
		break;
	}
	case InstOpcode::Identity:
		yulAssert(inst.inputs.size() == 1, "Identity with != 1 inputs.");
		_frame.setValue(_instId, _frame.value(inst.inputs.front()));
		break;
	case InstOpcode::Nop:
		break;
	case InstOpcode::MemoryGuard:
		yulAssert(m_cfgs.memoryGuard.has_value(), "MemoryGuard instruction without a memoryguard value.");
		_frame.setValue(_instId, *m_cfgs.memoryGuard);
		break;
	case InstOpcode::Tombstone:
		yulAssert(false, fmt::format("Executed Tombstone slot {}.", _instId));
		break;
	}
}
