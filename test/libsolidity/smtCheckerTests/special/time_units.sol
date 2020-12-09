pragma experimental SMTChecker;
contract D {
	function f() public pure {
		assert(1 == 1 seconds);
		assert(2 == 1 seconds);
		assert(2 minutes == 120 seconds);
		assert(3 minutes == 120 seconds);
		assert(2 hours == 120 minutes);
		assert(3 hours == 120 minutes);
		assert(2 days == 48 hours);
		assert(4 days == 48 hours);
		assert(2 weeks == 14 days);
		assert(25 weeks == 14 days);
	}
}
// ----
// Warning 6328: (101-123): CHC: Assertion violation happens here.\nCounterexample:\n\n\n\n\nTransaction trace:\nconstructor()\nf()
// Warning 6328: (163-195): CHC: Assertion violation happens here.\nCounterexample:\n\n\n\n\nTransaction trace:\nconstructor()\nf()
// Warning 6328: (233-263): CHC: Assertion violation happens here.\nCounterexample:\n\n\n\n\nTransaction trace:\nconstructor()\nf()
// Warning 6328: (297-323): CHC: Assertion violation happens here.\nCounterexample:\n\n\n\n\nTransaction trace:\nconstructor()\nf()
// Warning 6328: (357-384): CHC: Assertion violation happens here.\nCounterexample:\n\n\n\n\nTransaction trace:\nconstructor()\nf()
