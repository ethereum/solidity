contract C
{
	enum D { Left, Right }
	D d;
	function f(D _d) public {
		d = _d;
		assert(d != _d);
	}
}
// ====
// SMTEngine: all
// ----
// Warning 6328: (82-97): CHC: Assertion violation happens here.\nCounterexample:\nd = 0\n_d = 0\n\nTransaction trace:\nC.constructor()\nState: d = 0\nC.f(0)
