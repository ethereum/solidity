pragma experimental SMTChecker;

contract C
{
	enum D { Left, Right }
	D d;
	function f(D _d) public {
		d = _d;
		assert(d != _d);
	}
}
// ----
// Warning 6328: (115-130): CHC: Assertion violation happens here.\nCounterexample:\nd = 0\n_d = 0\n\n\nTransaction trace:\nconstructor()\nState: d = 0\nf(0)
