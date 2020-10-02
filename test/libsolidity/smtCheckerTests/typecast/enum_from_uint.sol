pragma experimental SMTChecker;

contract C
{
	enum D { Left, Right }
	function f(uint x) public pure {
		require(x == 0);
		D _a = D(x);
		assert(_a == D.Left);
	}
}
// ----
// Warning 8364: (132-133): Assertion checker does not yet implement type type(enum C.D)
