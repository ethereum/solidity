pragma experimental SMTChecker;

contract C {

	uint x;

	modifier check() {
		require(x == 0);
		_;
		assert(x == 1); // should fail;
		assert(x == 0); // should hold;
	}

	modifier inc() {
		if (x == 0) {
			return;
		}
		x = x + 1;
		_;
	}

	function test() check inc public {
	}
}
// ----
// Warning 6328: (103-117): CHC: Assertion violation happens here.\nCounterexample:\nx = 0\n\n\n\nTransaction trace:\nconstructor()\nState: x = 0\ntest()
