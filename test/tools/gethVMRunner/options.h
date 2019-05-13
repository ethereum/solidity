#pragma once

class Options
{
private:
	bool trace, cmp_success, cmp_trace, cmp_stack, cmp_gas, cmp_msize, diversity_guided, validate_code, no_prestate;
	std::string preset;
	size_t gas, max_gas, balance, max_balance, min_code_size;
	std::string from_statetest_in, from_statetest_out;
	std::string to_statetest_out;
	size_t blocknum;
	bool no_geth, no_parity, no_cpp;

	/* Geth */
	bool merge;
	std::string geth_inst_type;
	std::string geth_symcov_in, geth_symcov_out;
	std::string geth_minimize_in, geth_minimize_out;

public:
	void Parse(int argc, char** argv, const std::vector<std::string> preset_names);

	bool get_trace(void) const
	{ return this->trace; }

	bool get_cmp_success(void) const
	{ return this->cmp_success; }

	bool get_cmp_trace(void) const
	{ return this->cmp_trace; }

	bool get_cmp_stack(void) const
	{ return this->cmp_stack; }

	bool get_cmp_gas(void) const
	{ return this->cmp_gas; }

	bool get_cmp_msize(void) const
	{ return this->cmp_msize; }

	bool get_cmp(void) const
	{
		return this->cmp_success || this->cmp_trace || this->cmp_stack || this->cmp_gas || this->cmp_msize;
	}

	bool get_diversity_guided(void) const
	{ return this->diversity_guided; }

	bool get_validate_code(void) const
	{ return this->validate_code; }

	bool get_no_prestate(void) const
	{ return this->no_prestate; }

	std::string get_preset(void) const
	{ return this->preset; }

	size_t get_gas(void) const
	{ return this->gas; }

	size_t get_max_gas(void) const
	{ return this->max_gas; }

	size_t get_balance(void) const
	{ return this->balance; }

	size_t get_max_balance(void) const
	{ return this->max_balance; }

	size_t get_min_code_size(void) const
	{ return this->min_code_size; }

	bool convert_from_statetest(void) const
	{
		return this->from_statetest_in.size() && this->from_statetest_out.size();
	}

	bool convert_to_statetest(void) const
	{
		return this->to_statetest_out.size();
	}

	std::pair<std::string, std::string> get_from_statetest(void) const
	{
		return std::pair<std::string, std::string>(this->from_statetest_in, this->from_statetest_out);
	}

	std::string get_to_statetest(void) const
	{
		return this->to_statetest_out;
	}

	bool get_no_geth(void) const
	{ return this->no_geth; }

	bool get_no_parity(void) const
	{ return this->no_parity; }

	bool get_no_cpp(void) const
	{ return this->no_cpp; }

	size_t get_num_vms(void) const
	{
		size_t num_vms = 0;
		if (get_no_geth() == false) num_vms++;
		if (get_no_parity() == false) num_vms++;
		if (get_no_cpp() == false) num_vms++;
		return num_vms;
	}

	bool get_merge(void) const
	{ return this->merge; }

	std::string get_geth_inst_type(void) const
	{ return this->geth_inst_type; }

	std::string get_geth_symcov_in(void) const
	{ return this->geth_symcov_in; }

	std::string get_geth_symcov_out(void) const
	{ return this->geth_symcov_out; }

	std::string get_geth_minimize_in(void) const
	{ return this->geth_minimize_in; }

	std::string get_geth_minimize_out(void) const
	{ return this->geth_minimize_out; }

	size_t get_blocknum(void) const
	{ return this->blocknum; }
};