pragma experimental SMTChecker;

contract C {

	function test(uint256 a) public pure {
		assert(simple_if(a) == 1); // should fail for a == 0
	}

	function simple_if(uint256 a) internal pure returns (uint256) {
		if (a == 0) {
			return 0;
		}
		return 1;
	}
}
// ----
// Warning 6328: (89-114): CHC: Assertion violation happens here.\nCounterexample:\n\na = 0\n\nTransaction trace:\nC.constructor()\nC.test(0)\n    C.simple_if(0) -- internal call
