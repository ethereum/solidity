#pragma once

#include <stdint.h>
#include "account.h"
#include "prestate.h"

class VMEnv
{
private:
	bool reportTrace;
	bool printTrace;

	uint64_t gas;
	uint64_t blocknumber;
	uint64_t timestamp;
	uint64_t gaslimit;
	uint64_t difficulty;
	uint64_t gasprice;

	Prestate* prestate;

	struct
	{
		bool gas;
		bool blocknumber;
		bool timestamp;
		bool gaslimit;
		bool difficulty;
		bool gasprice;

		bool prestate;
	} isSet;

public:
	VMEnv(void);

	~VMEnv();

	void SetPrintTrace(const bool _printTrace);

	void SetReportTrace(const bool _reportTrace);

	bool GetPrintTrace(void) const;

	bool GetReportTrace(void) const;

	void SetGas(uint64_t _gas);

	void SetGas(const std::string _gas);

	uint64_t GetGas(void) const;

	std::string GetGasString(const bool prepend0x = false) const;

	void SetBlockNumber(uint64_t _blocknumber);

	void SetBlockNumber(const std::string _blocknumber);

	uint64_t GetBlockNumber(void) const;

	std::string GetBlockNumberString(const bool prepend0x = false) const;

	void SetTimestamp(uint64_t _timestamp);

	void SetTimestamp(const std::string _timestamp);

	uint64_t GetTimestamp(void) const;

	std::string GetTimestampString(const bool prepend0x = false) const;

	void SetGasLimit(uint64_t _gaslimit);

	void SetGasLimit(const std::string _gaslimit);

	uint64_t GetGasLimit(void) const;

	std::string GetGasLimitString(const bool prepend0x = false) const;

	void SetDifficulty(uint64_t _difficulty);

	void SetDifficulty(const std::string _difficulty);

	uint64_t GetDifficulty(void) const;

	std::string GetDifficultyString(const bool prepend0x = false) const;

	void SetGasPrice(uint64_t _gasprice);

	void SetGasPrice(const std::string _gasprice);

	uint64_t GetGasPrice(void) const;

	std::string GetGasPriceString(const bool prepend0x = false) const;

	void AddAccount(Account* account, const bool isExec);

	void SetPrestate(Prestate* _prestate);

	Prestate* GetPrestate(void) const;

	bool IsValid(void) const;
};