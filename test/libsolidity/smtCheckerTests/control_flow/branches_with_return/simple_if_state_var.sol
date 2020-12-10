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
// Warning 6328: (132-146): CHC: Assertion violation happens here.\nCounterexample:\nx = 0\n\n\n\nTransaction trace:\nconstructor()\nState: x = 0\ncheck()
