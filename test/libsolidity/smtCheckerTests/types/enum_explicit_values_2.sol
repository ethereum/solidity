pragma experimental SMTChecker;

contract C
{
	enum D { Left, Right }
	D d;
	function f(D _a) public {
		require(_a == D.Left);
		d = D.Left;
		assert(d != _a);
	}
}
// ----
// Warning 6328: (144-159): CHC: Assertion violation happens here.\nCounterexample:\nd = 0\n_a = 0\n\n\nTransaction trace:\nconstructor()\nState: d = 0\nf(0)
