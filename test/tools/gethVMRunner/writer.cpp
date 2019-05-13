#include "writer.h"

Writer::Writer(const VMEnv* vmEnv) :
		vmEnv(vmEnv)
{
}

bool Writer::vectorToBytes(std::vector<uint8_t>& out, const std::vector<uint8_t>& in) const
{
	/* Size needs to fit in a uint16_t */
	if (in.size() > 65535) {
		return false;
	}

	/* Write vector size */
	{
		const uint16_t inSize = in.size();
		const uint8_t* inSizePtr = (uint8_t*) (&inSize);
		const std::vector<uint8_t> inSizeVec = std::vector<uint8_t>(inSizePtr, inSizePtr + sizeof(inSize));
		out.insert(std::end(out), std::begin(inSizeVec), std::end(inSizeVec));
	}

	/* Write vector */
	out.insert(std::end(out), std::begin(in), std::end(in));

	return true;
}

bool Writer::singleAccountToBytes(std::vector<uint8_t>& out, const Account* curAccount) const
{
	/* Retrieve and write address */
	{
		const std::vector<uint8_t> addressVec = curAccount->GetAddress();
		out.insert(std::end(out), std::begin(addressVec), std::end(addressVec));
	}

	/* Retrieve and write balance */
	{
		const uint64_t balance = curAccount->GetBalance();
		const uint8_t* balancePtr = (uint8_t*) (&balance);
		const std::vector<uint8_t> balanceVec = std::vector<uint8_t>(balancePtr, balancePtr + sizeof(balance));
		out.insert(std::end(out), std::begin(balanceVec), std::end(balanceVec));
	}

	/* Retrieve and write code */
	{
		const std::vector<uint8_t> codeVec = curAccount->GetCode();

		if (vectorToBytes(out, codeVec) == false) {
			return false;
		}
	}

	return true;
}

bool Writer::accountsToBytes(std::vector<uint8_t>& out) const
{
	const auto prestate = vmEnv->GetPrestate();

	/* Write execution account */
	if (singleAccountToBytes(out, prestate->GetExecAccount()) == false) {
		return false;
	}

	/* Write other accounts in prestate */
	{
		bool failure = false;
		prestate->ForEach([&](Account* curAccount) {
			if (failure == false && (curAccount != prestate->GetExecAccount()) &&
			    singleAccountToBytes(out, curAccount) == false) {
				failure = true;
			}
		});

		if (failure == true) {
			return false;
		}
	}


	return true;
}

bool Writer::ToBytes(std::vector<uint8_t>& out) const
{
	out.clear();

	if (vmEnv->IsValid() == false) {
		return false;
	}

	{
		struct
		{
			uint64_t gas;
			uint64_t timestamp;
			uint64_t gaslimit;
			uint64_t difficulty;
			uint64_t gasprice;
		} envVars;

		envVars.gas = vmEnv->GetGas();
		envVars.timestamp = vmEnv->GetTimestamp();
		envVars.gaslimit = vmEnv->GetGasLimit();
		envVars.difficulty = vmEnv->GetDifficulty();
		envVars.gasprice = vmEnv->GetGasPrice();

		const std::vector<uint8_t> envVarsVec((uint8_t*) (&envVars), ((uint8_t*) &envVars) + sizeof(envVars));

		out.insert(std::end(out), std::begin(envVarsVec), std::end(envVarsVec));
	}

	{
		const auto inputVec = vmEnv->GetPrestate()->GetInput();
		if (vectorToBytes(out, inputVec) == false) {
			return false;
		}
	}

	if (accountsToBytes(out) == false) {
		return false;
	}

	return true;
}
