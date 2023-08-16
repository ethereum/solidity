contract c {
	uint x;
	function f() internal returns (uint) {
		x = x + 1;
		return x;
	}
	function g() public returns (bool) {
		x = 0;
		bool b = (f() > 0) || (f() > 0);
		assert(x == 1);
		assert(!b);
		return b;
	}
}
// ====
// SMTEngine: all
// SMTIgnoreCex: no
// ----
// Warning 6328: (192-202): CHC: Assertion violation happens here.\nCounterexample:\nx = 1\n = false\nb = true\n\nTransaction trace:\nc.constructor()\nState: x = 0\nc.g()\n    c.f() -- internal call\n    c.f() -- internal call
// Info 1391: CHC: 2 verification condition(s) proved safe! Enable the model checker option "show proved safe" to see all of them.
