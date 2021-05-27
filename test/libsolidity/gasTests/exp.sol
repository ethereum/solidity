pragma abicoder               v2;

contract C {
	function exp_neg_one(uint exponent) public returns(int) {
		unchecked { return (-1)**exponent; }
	}
	function exp_two(uint exponent) public returns(uint) {
		unchecked { return 2**exponent; }
	}
	function exp_zero(uint exponent) public returns(uint) {
		unchecked { return 0**exponent; }
	}
	function exp_one(uint exponent) public returns(uint) {
		unchecked { return 1**exponent; }
	}
}
// ====
// optimize: false
// optimize-yul: false
// ----
// creation:
//   codeDepositCost: 120600
//   executionCost: 172
//   totalCost: 120772
// external:
//   exp_neg_one(uint256): 2254
//   exp_one(uint256): infinite
//   exp_two(uint256): infinite
//   exp_zero(uint256): infinite
