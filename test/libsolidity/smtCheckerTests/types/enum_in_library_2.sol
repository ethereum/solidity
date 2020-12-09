pragma experimental SMTChecker;

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
// ----
// Warning 6328: (159-179): CHC: Assertion violation happens here.\nCounterexample:\n\n_d = 1\n\n\nTransaction trace:\nconstructor()\nf(0)
