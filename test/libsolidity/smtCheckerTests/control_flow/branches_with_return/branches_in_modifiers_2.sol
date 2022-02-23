contract C {

	uint x;

	function reset_if_overflow() internal postinc {
		if (x < 10)
			return;
		x = 0;
	}

	modifier postinc() {
		if (x == 0) {
			return;
		}
		_;
		x = x + 1;
	}

	function test() public {
		if (x == 0) {
			reset_if_overflow();
			assert(x == 1); // should fail;
			assert(x == 0); // should hold;
			return;
		}
		if (x < 10) {
			uint oldx = x;
			reset_if_overflow();
			// Disabled because of nondeterminism in Spacer Z3 4.8.9
			//assert(oldx + 1 == x); // should hold;
			assert(oldx == x);     // should fail;
			return;
		}
		reset_if_overflow();
		assert(x == 1); // should hold;
		assert(x == 0); // should fail;
	}

	function set(uint _x) public {
		x = _x;
	}
}
// ====
// SMTEngine: all
// SMTIgnoreOS: macos
// ----
// Warning 6328: (255-269): CHC: Assertion violation happens here.\nCounterexample:\nx = 0\n\nTransaction trace:\nC.constructor()\nState: x = 0\nC.test()\n    C.reset_if_overflow() -- internal call
// Warning 6328: (502-519): CHC: Assertion violation happens here.
// Warning 6328: (615-629): CHC: Assertion violation happens here.\nCounterexample:\nx = 1\n\nTransaction trace:\nC.constructor()\nState: x = 0\nC.set(10)\nState: x = 10\nC.test()\n    C.reset_if_overflow() -- internal call
