#include "prestate.h"
#include "util.h"
#include <string.h>
#include <set>
#include <iostream>

Prestate::Prestate(void) :
		execAccount(nullptr)
{
	isSet.execAccount = false;
	isSet.input = false;
}

Prestate::~Prestate()
{
	for (const auto& account : accounts) {
		delete account;
	}
}

/* TODO return bool; false if address exists */
void Prestate::Add(Account* _account, const bool isExec)
{
	if (isExec == true) {
		if (execAccount != nullptr) {
			throw std::runtime_error("Tried adding multiple execAccounts");
		}
		execAccount = _account;

		isSet.execAccount = true;
		// execAccount should be in the prestate too.
		accounts.push_back(_account);
	} else {
		accounts.push_back(_account);
	}
}

Account* Prestate::GetExecAccount(void) const
{
	if (execAccount == nullptr) {
		throw std::runtime_error("No execAccount defined");
	}

	return execAccount;
}

size_t Prestate::Size(void) const
{
	return accounts.size();
}

Account* Prestate::GetAccountByIdx(const size_t idx) const
{
	if (idx >= Size()) {
		throw std::runtime_error("Invalid account index requested");
	}

	return accounts[idx];
}

bool Prestate::hasValidAddress(Account* account) const
{
	/* Reject addresses of precompiled contracts */

	static uint8_t precompile_addr_19[] = {
			0x00, 0x00, 0x00, 0x00, 0x00,
			0x00, 0x00, 0x00, 0x00, 0x00,
			0x00, 0x00, 0x00, 0x00, 0x00,
			0x00, 0x00, 0x00, 0x00};

	const auto address = account->GetAddressPtr();

	if (memcmp(address, precompile_addr_19, 19) == 0) {
		switch (address[19]) {
			case 0x01:
			case 0x02:
			case 0x03:
			case 0x04:
				return false;
				break;
			default:
				break;

		}
	}

	return true;
}

bool Prestate::hasAccount(Account* account) const
{
	/* Return true if prestate contains the account */
	for (const auto curAccount : accounts) {
		if (account->GetAddress() == curAccount->GetAddress())
			return true;
	}

	return false;
}

bool Prestate::IsValid(void) const
{
	if (isSet.execAccount == false) {
		return false;
	}

	std::vector<Account*> allAccounts;

	for (const auto curAccount : accounts) {
		allAccounts.push_back(curAccount);
	}

	{
		/* Account self-validation via Account::IsValid() */
		for (const auto curAccount : allAccounts) {
			if (curAccount->IsValid() == false) {
				return false;
			}
		}
	}

	{
		/* Test if addresses are valid */
		/* TODO move to Account */
		for (const auto curAccount : allAccounts) {
			if (hasValidAddress(curAccount) == false) {
				return false;
			}
		}
	}

	{
		std::set<std::vector<uint8_t>> addresses;

		size_t prevSize = 0;
		for (const auto curAccount : allAccounts) {
			if (addresses.count(curAccount->GetAddress())) {
				std::cout << "account " << curAccount->GetAddressHexString()
				          << " was already processed in the prestate!" << std::endl;
				return false;
			} else {
				addresses.insert(curAccount->GetAddress());
				size_t newSize = addresses.size();
				if (prevSize == newSize) {
					return false;
				}
				prevSize = newSize;
			}
		}
	}

	/* TODO: check balances <= 2**63 */

	return true;
}

std::vector<uint8_t> Prestate::GetInput(void) const
{
	return input;
}

uint8_t* Prestate::GetInputPtr(void)
{
	return input.data();
}

size_t Prestate::GetInputSize(void) const
{
	return input.size();
}

std::string Prestate::GetInputString(void) const
{
	return dataToHexString(input);
}

void Prestate::SetInput(const std::vector<uint8_t> _input)
{
	input = _input;

	isSet.input = true;
}
