pragma experimental SMTChecker;
contract c {
	function f() public pure returns (uint) {
		uint x = 8e130%9;
		assert(x == 8);
		assert(x != 8);
	}
}
// ----
// Warning: (128-142): Assertion violation happens here
