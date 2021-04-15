library L
{
	enum D { Left, Right }
}

contract C
{
	enum E { Left, Right }
	function f(E _d) public pure {
		_d = E.Right;
		assert(_d == E.Left);
	}
}
// ====
// SMTEngine: all
// ----
// Warning 6328: (126-146): CHC: Assertion violation happens here.\nCounterexample:\n\n_d = 1\n\nTransaction trace:\nC.constructor()\nC.f(0)
