contract C {
	string x;

	function s() public {
		x = "abc";
		bytes(x).push("a");
		assert(bytes(x).length == 4); // should hold
		assert(bytes(x).length == 3); // should fail
	}
}
// ====
// SMTEngine: all
// ----
// Warning 6328: (132-160): CHC: Assertion violation happens here.\nCounterexample:\n\n\nTransaction trace:\nC.constructor()\nState: x = []\nC.s()
