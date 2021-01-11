pragma experimental SMTChecker;

contract C {

	uint x;

	function check() public {
		require(x == 0);
		conditional_increment();
		assert(x == 1); // should fail;
		assert(x == 0); // should hold;
	}

	function conditional_increment() internal {
		if (x == 0) {
			return;
		}
		x = 1;
	}
}
// ----
// Warning 6328: (132-146): CHC: Assertion violation happens here.\nCounterexample:\nx = 0\n\nTransaction trace:\nC.constructor()\nState: x = 0\nC.check()\n    C.conditional_increment() -- internal call
