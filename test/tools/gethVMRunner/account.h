#pragma once

#include <stdint.h>
#include <string>
#include <vector>

class Account
{
private:
	std::vector<uint8_t> address;
	std::vector<uint8_t> code;
	uint64_t balance;

	struct
	{
		bool address;
		bool code;
		bool balance;
	} isSet;
public:
	Account(void);

	std::string GetAddressHexString(void) const;

	std::vector<uint8_t> GetAddress(void) const;

	uint8_t* GetAddressPtr(void);

	void SetAddress(const uint8_t* _address);

	void SetAddress(std::string hexString);

	std::vector<uint8_t> GetCode() const;

	uint8_t* GetCodePtr(void);

	size_t GetCodeSize(void) const;

	std::string GetCodeString(void) const;

	void SetCode(const std::vector<uint8_t> _code);

	void SetCode(std::string codeHex);

	void SetBalance(uint64_t _balance);

	void SetBalance(const std::string _balance);

	uint64_t GetBalance(void) const;

	std::string GetBalanceHexString(void) const;

	std::string GetBalanceString(void) const;

	bool IsValid(void) const;
};