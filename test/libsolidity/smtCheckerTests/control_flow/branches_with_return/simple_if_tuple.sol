pragma experimental SMTChecker;

contract C {

	uint x;
	uint y;

	function check() public {
		require(x == 0);
		require(y == 0);
		conditional_increment();
		assert(x == 0); // should fail;
		assert(x == 1); // should fail;
		assert(x == 2); // should hold;
	}

	function conditional_increment() internal {
		if (x == 0) {
			(x,y) = (2,2);
			return;
		}
		(x,y) = (1,1);
	}
}
// ----
// Warning 6328: (160-174): CHC: Assertion violation happens here.\nCounterexample:\nx = 2, y = 2\n\n\n\nTransaction trace:\nconstructor()\nState: x = 0, y = 0\ncheck()
// Warning 6328: (194-208): CHC: Assertion violation happens here.\nCounterexample:\nx = 2, y = 2\n\n\n\nTransaction trace:\nconstructor()\nState: x = 0, y = 0\ncheck()
