#pragma once
#include "test/tools/yulInterpreter/EVMInstructionInterpreter.h"
#include "test/tools/yulInterpreter/Interpreter.h"


#include <libsolutil/Numeric.h>

#include <optional>
#include <vector>

namespace solidity::yul::ssa
{
class SSACFG;
struct InstId;
struct ControlFlowGraphs;
}

namespace solidity::yul::test
{

class SSACFGInterpreter
{
public:
	SSACFGInterpreter(ssa::ControlFlowGraphs const& _cfgs, InterpreterState _state, bool _disableMemoryTrace);

	std::vector<u256> runFunction(ssa::SSACFG const& _function, std::vector<u256> const& _arguments);
private:
	class Frame
	{
	public:
		explicit Frame(std::size_t const _numValues): m_values(_numValues, std::nullopt) {}

		u256 const& value(ssa::InstId _id) const;
		bool hasValue(ssa::InstId _id) const;
		void setValue(ssa::InstId _id, u256 const& _value);

	private:
		std::vector<std::optional<u256>> m_values;
	};

	void executeInstruction(ssa::SSACFG const& _function, Frame& _frame, ssa::InstId _instId);
	void executeBuiltinCall(ssa::SSACFG const& _function, Frame& _frame, ssa::InstId _id);
	void executeCall(ssa::SSACFG const& _function, Frame& _frame, ssa::InstId _id);

	ssa::ControlFlowGraphs const& m_cfgs;
	InterpreterState m_state;
	EVMInstructionInterpreter m_evmInstructionInterpreter;
};

}
