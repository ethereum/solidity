#pragma once

#include "account.h"
#include <vector>
#include <stdexcept>

class Prestate
{
private:
	Account* execAccount;
	std::vector<Account*> accounts;
	std::vector<uint8_t> input;

	bool hasValidAddress(Account* account) const;

	struct
	{
		bool execAccount;
		bool input;
	} isSet;
public:
	Prestate(void);

	~Prestate();

	void Add(Account* _account, const bool isExec);

	template<class Callback>
	void ForEach(Callback CB) const
	{
		for (const auto& account : accounts) {
			CB(account);
		}
	}

	Account* GetExecAccount(void) const;

	size_t Size(void) const;

	Account* GetAccountByIdx(const size_t idx) const;

	bool IsValid(void) const;

	std::vector<uint8_t> GetInput(void) const;

	uint8_t* GetInputPtr(void);

	size_t GetInputSize(void) const;

	std::string GetInputString(void) const;

	void SetInput(const std::vector<uint8_t> _input);

	bool hasAccount(Account* account) const;
};