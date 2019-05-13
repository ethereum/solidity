#include "account.h"
#include <string.h>
#include <stdexcept>
#include "util.h"

void Account::SetAddress(const uint8_t* _address)
{
	memcpy(address.data(), _address, 20);

	isSet.address = true;
}

Account::Account(void) :
		balance(0)
{
	address.resize(20);
	isSet.address = false;
	isSet.code = false;
	isSet.balance = false;
}

std::string Account::GetAddressHexString(void) const
{
	return dataToHexString(address);
}

std::vector<uint8_t> Account::GetAddress(void) const
{
	return address;
}

uint8_t* Account::GetAddressPtr(void)
{
	return address.data();
}

void Account::SetAddress(std::string hexString)
{
	if (hexString.size() != 40 && hexString.size() != 42) {
		throw std::runtime_error("Invalid input to Account::SetAddress");
	}

	const auto data = hexToData(hexString);

	if (data.size() != 20) {
		/* Shouldn't happen */
		abort();
	}

	memcpy(address.data(), data.data(), 20);

	isSet.address = true;
}

void Account::SetBalance(uint64_t _balance)
{
	balance = _balance;

	isSet.balance = true;
}

void Account::SetBalance(const std::string _balance)
{
	balance = uint64FromHexString(_balance);

	isSet.balance = true;
}

uint64_t Account::GetBalance(void) const
{
	return balance;
}

std::string Account::GetBalanceString(void) const
{
	return stringFromUint64(balance);
}

std::string Account::GetBalanceHexString(void) const
{
	return hexStringFromUint64(balance);
}

std::vector<uint8_t> Account::GetCode(void) const
{
	return code;
}

uint8_t* Account::GetCodePtr(void)
{
	return code.data();
}

size_t Account::GetCodeSize(void) const
{
	return code.size();
}

std::string Account::GetCodeString(void) const
{
	return dataToHexString(code);
}

void Account::SetCode(const std::vector<uint8_t> _code)
{
	code = _code;

	isSet.code = true;
}

void Account::SetCode(std::string codeHex)
{
	if (codeHex.size() >= 2) {
		if (codeHex[0] == '0' && codeHex[1] == 'x') {
			codeHex = codeHex.substr(2);
		}
	}

	const auto data = hexToData(codeHex);
	SetCode(data);
}

bool Account::IsValid(void) const
{
	if (isSet.address == false) {
		return false;
	}

	if (isSet.code == false) {
		return false;
	}

	if (isSet.balance == false) {
		return false;
	}

	return true;
}