pragma experimental SMTChecker;

library L
{
	enum D { Left, Right }
}

contract C
{
	enum E { Left, Right }
	function f(E _d) public pure {
		_d = E.Left;
		assert(_d == E.Left);
	}
}
