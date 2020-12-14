pragma experimental SMTChecker;
contract Simp {
	function f3() public pure returns (bytes1) {
		bytes memory y = "def";
		assert(y[0] ^ "e" != bytes1(0)); // should hold
		assert(y[1] ^ "e" != bytes1(0)); // should fail
		return y[0];
	}
}
// ----
// Warning 6328: (172-203): CHC: Assertion violation happens here.\nCounterexample:\n\n\n = 0\n\nTransaction trace:\nconstructor()\nf3()
