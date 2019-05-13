#include "vmenv.h"
#include "util.h"

VMEnv::VMEnv(void) :
		reportTrace(false), printTrace(false),
		gas(0), blocknumber(0), timestamp(0), gaslimit(0), difficulty(0), gasprice(0), prestate(nullptr)
{
	isSet.gas = false;
	isSet.blocknumber = false;
	isSet.timestamp = false;
	isSet.gaslimit = false;
	isSet.difficulty = false;
	isSet.gasprice = false;
	isSet.prestate = false;
}

VMEnv::~VMEnv()
{
	delete prestate;
}

void VMEnv::SetPrintTrace(const bool _printTrace)
{
	printTrace = _printTrace;
}

void VMEnv::SetReportTrace(const bool _reportTrace)
{
	reportTrace = _reportTrace;
}

bool VMEnv::GetPrintTrace(void) const
{
	return printTrace;
}

bool VMEnv::GetReportTrace(void) const
{
	return reportTrace;
}

void VMEnv::SetGas(uint64_t _gas)
{
	gas = _gas;

	isSet.gas = true;
}

void VMEnv::SetGas(const std::string _gas)
{
	gas = uint64FromHexString(_gas);

	isSet.gas = true;
}

uint64_t VMEnv::GetGas(void) const
{
	return gas;
}

std::string VMEnv::GetGasString(const bool prepend0x) const
{
	std::string ret;
	if (prepend0x) {
		ret += "0x";
	}
	ret += hexStringFromUint64(gas);
	return ret;
}


void VMEnv::SetBlockNumber(uint64_t _blocknumber)
{
	blocknumber = _blocknumber;

	isSet.blocknumber = true;
}

void VMEnv::SetBlockNumber(const std::string _blocknumber)
{
	blocknumber = uint64FromHexString(_blocknumber);

	isSet.blocknumber = true;
}

uint64_t VMEnv::GetBlockNumber(void) const
{
	return blocknumber;
}

std::string VMEnv::GetBlockNumberString(const bool prepend0x) const
{
	std::string ret;
	if (prepend0x) {
		ret += "0x";
	}
	ret += hexStringFromUint64(blocknumber);
	return ret;
}


void VMEnv::SetTimestamp(uint64_t _timestamp)
{
	timestamp = _timestamp;

	isSet.timestamp = true;
}

void VMEnv::SetTimestamp(const std::string _timestamp)
{
	timestamp = uint64FromHexString(_timestamp);

	isSet.timestamp = true;
}

uint64_t VMEnv::GetTimestamp(void) const
{
	return timestamp;
}

std::string VMEnv::GetTimestampString(const bool prepend0x) const
{
	std::string ret;
	if (prepend0x) {
		ret += "0x";
	}
	ret += hexStringFromUint64(timestamp);
	return ret;
}


void VMEnv::SetGasLimit(uint64_t _gaslimit)
{
	gaslimit = _gaslimit;

	isSet.gaslimit = true;
}

void VMEnv::SetGasLimit(const std::string _gaslimit)
{
	gaslimit = uint64FromHexString(_gaslimit);

	isSet.gaslimit = true;
}

uint64_t VMEnv::GetGasLimit(void) const
{
	return gaslimit;
}

std::string VMEnv::GetGasLimitString(const bool prepend0x) const
{
	std::string ret;
	if (prepend0x) {
		ret += "0x";
	}
	ret += hexStringFromUint64(gaslimit);
	return ret;
}


void VMEnv::SetDifficulty(uint64_t _difficulty)
{
	difficulty = _difficulty;

	isSet.difficulty = true;
}

void VMEnv::SetDifficulty(const std::string _difficulty)
{
	difficulty = uint64FromHexString(_difficulty);

	isSet.difficulty = true;
}

uint64_t VMEnv::GetDifficulty(void) const
{
	return difficulty;
}

std::string VMEnv::GetDifficultyString(const bool prepend0x) const
{
	std::string ret;
	if (prepend0x) {
		ret += "0x";
	}
	ret += hexStringFromUint64(difficulty);
	return ret;
}


void VMEnv::SetGasPrice(uint64_t _gasprice)
{
	gasprice = _gasprice;

	isSet.gasprice = true;
}

void VMEnv::SetGasPrice(const std::string _gasprice)
{
	gasprice = uint64FromHexString(_gasprice);

	isSet.gasprice = true;
}

uint64_t VMEnv::GetGasPrice(void) const
{
	return gasprice;
}

std::string VMEnv::GetGasPriceString(const bool prepend0x) const
{
	std::string ret;
	if (prepend0x) {
		ret += "0x";
	}
	ret += hexStringFromUint64(gasprice);
	return ret;
}


void VMEnv::AddAccount(Account* _account, const bool isExec)
{
	prestate->Add(_account, isExec);
}

void VMEnv::SetPrestate(Prestate* _prestate)
{
	if (prestate != nullptr) {
		delete prestate;
	}

	prestate = _prestate;

	isSet.prestate = true;
}

Prestate* VMEnv::GetPrestate(void) const
{
	return prestate;
}

bool VMEnv::IsValid(void) const
{
	if (isSet.gas == false) {
		return false;
	}
	if (isSet.blocknumber == false) {
		return false;
	}
	if (isSet.timestamp == false) {
		return false;
	}
	if (isSet.gaslimit == false) {
		return false;
	}
	if (isSet.difficulty == false) {
		return false;
	}
	if (isSet.gasprice == false) {
		return false;
	}
	if (isSet.prestate == false) {
		return false;
	}

	if (GetPrestate()->IsValid() == false) {
		return false;
	}

	return true;
}
