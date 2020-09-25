pragma experimental SMTChecker;
contract c {
	function f() public pure returns (uint) {
		uint x = 8e130%9;
		assert(x == 8);
		assert(x != 8);
	}
}
// ----
// Warning 6328: (128-142): CHC: Assertion violation happens here.
