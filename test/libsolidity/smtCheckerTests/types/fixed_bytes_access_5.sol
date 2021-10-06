contract C {
	function f() public pure {
		bytes4 x = 0x01020304;
		bytes1 b = 0x02;
		assert(x[0] == b); // fails
		assert(x[1] == b);
		assert(x[2] == b); // fails
		assert(x[3] == b); // fails
	}
}
// ====
// SMTEngine: all
// ----
// Warning 6328: (87-104): CHC: Assertion violation happens here.\nCounterexample:\n\nx = 0x01020304\nb = 0x02\n\nTransaction trace:\nC.constructor()\nC.f()
// Warning 6328: (138-155): CHC: Assertion violation happens here.\nCounterexample:\n\nx = 0x01020304\nb = 0x02\n\nTransaction trace:\nC.constructor()\nC.f()
// Warning 6328: (168-185): CHC: Assertion violation happens here.\nCounterexample:\n\nx = 0x01020304\nb = 0x02\n\nTransaction trace:\nC.constructor()\nC.f()
