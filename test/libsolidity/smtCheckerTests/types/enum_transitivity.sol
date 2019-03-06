pragma experimental SMTChecker;

contract C
{
	enum D { Left, Right }
	D d;
	function f(D _a, D _b) public view {
		require(_a == _b);
		require(_a == d);
		assert(d == _b);
	}
}
