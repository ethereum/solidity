uint constant x = 42;

contract C {
	function f() public pure {
		uint z = x;
		assert(z == 41);
	}
}
// ====
// SMTEngine: all
// ----
// Warning 6328: (80-95): CHC: Assertion violation happens here.\nCounterexample:\n\nz = 42\n\nTransaction trace:\nC.constructor()\nC.f()
