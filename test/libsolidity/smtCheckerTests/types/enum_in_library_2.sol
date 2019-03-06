pragma experimental SMTChecker;

library L
{
	enum D { Left, Right }
}

contract C
{
	enum E { Left, Right }
	function f(E _d) internal pure {
		_d = E.Right;
		assert(_d == E.Left);
	}
}
// ----
// Warning: (161-181): Assertion violation happens here
