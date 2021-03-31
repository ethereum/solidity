contract C
{
	function f() public pure {
		uint a = 1;
		uint b = 3;
		a += ((((b))));
		assert(a == 3);
	}
}
// ====
// SMTEngine: all
// ----
// Warning 6328: (89-103): CHC: Assertion violation happens here.\nCounterexample:\n\na = 4\nb = 3\n\nTransaction trace:\nC.constructor()\nC.f()
