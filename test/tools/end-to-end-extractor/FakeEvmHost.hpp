#pragma once

#include <test/EVMHost.h>

namespace solidity::test
{

class FakeEvmHost : public evmc::MockedHost
{
  public:
	void reset() {}
	void newBlock() {}

	static Address convertFromEVMC(evmc::address const &_addr)
	{
		(void) _addr;
		return Address();
	}

	static evmc::address convertToEVMC(Address const &_addr)
	{
		(void) _addr;
		return evmc::address();
	}

	static util::h256 convertFromEVMC(evmc::bytes32 const &_data)
	{
		(void) _data;
		return util::h256();
	}

	static evmc::bytes32 convertToEVMC(util::h256 const &_data)
	{
		(void) _data;
		return evmc::bytes32();
	}
};

} // namespace solidity::test
