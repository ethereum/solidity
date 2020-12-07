pragma experimental SMTChecker;
contract Simp {
	function f3() public pure returns (byte) {
		bytes memory y = "def";
		assert(y[0] ^ "e" != byte(0)); // should hold
		assert(y[1] ^ "e" != byte(0)); // should fail
		return y[0];
	}
}
// ----
// Warning 6328: (168-197): CHC: Assertion violation happens here.
