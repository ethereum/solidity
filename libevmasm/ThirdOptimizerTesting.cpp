#include <libevmasm/ThirdOptimizerTesting.h>
#include "ThirdOptimizerTesting.h"
#include <algorithm>


bool dev::solidity::Pattern::matches(std::vector<AssemblyItem> _items) const
{
	vector<Pattern> stack;
	for (auto const& item : _items)
	{
		switch (item.type())
		{
			case AssemblyItemType::Push:
				stack.emplace_back(item.data());
				break;
			case AssemblyItemType::Operation:
			{
				assertThrow(stack.size() >= unsigned(item.arguments()), OptimizerException, "invalid operation on Pattern::matches");
				vector<Pattern> arguments(stack.rbegin(), stack.rbegin() + item.arguments());

				{
					int i = item.arguments();
					while (i--) stack.pop_back();
				}

				if (item.returnValues() == 1 && !(isSwapInstruction(item.instruction()) || isDupInstruction(item.instruction())))
					stack.emplace_back(item.instruction(), arguments);
				else
				{
					int i = item.returnValues();
					while (i--) stack.emplace_back(AssemblyItemType::UndefinedItem);
				}
				break;
			}
			default:
			{
				assertThrow(stack.size() >= unsigned(item.arguments()), OptimizerException, "invalid operation on Pattern::matches");
				{
					int i = item.arguments();
					while (i--) stack.pop_back();
				}
				{
					int i = item.returnValues();
					while (i--) stack.emplace_back(AssemblyItemType::UndefinedItem);
				}
			}
		}
	}

	assertThrow(stack.size() == 1, OptimizerException, "operand stream did not leave stack with one item");

	auto otherPattern = stack[0];
	return this->matches(otherPattern);
}

bool dev::solidity::Pattern::matches(Pattern _other) const
{
	switch (kind())
	{
		case Kind::Constant:
		{
			if (_other.kind() != Kind::Constant) return false;
			if (!m_hasConstant) return true;
			if (!_other.m_hasConstant) return false;

			return constant() == _other.constant();
		}
		case Kind::Operation:
		{
			if (_other.kind() != Kind::Operation) return false;
			if (!m_hasOperationValues) return true;
			if (!_other.m_hasOperationValues) return false;

			auto otherArguments = _other.operands();
			if (otherArguments.size() != m_operands.size()) return false;

			{
				unsigned n = 0;
				for (auto const& operand : m_operands)
				{
					if (!operand.matches(otherArguments[n])) return false;
					n++;
				}
			}

			return true;
		}
		case Kind::Any:
		{
			if (m_hasAssemblyType)
			{
				if (!_other.m_hasAssemblyType) return false;
				if (assemblyItemType() == AssemblyItemType::UndefinedItem) return true;
				return assemblyItemType() == _other.assemblyItemType();
			}
			else
			{
				// We were given the type Any
				return true;
			}
		}
		default:
			assertThrow(false, OptimizerException, "invalid pattern kind");
	}
}
