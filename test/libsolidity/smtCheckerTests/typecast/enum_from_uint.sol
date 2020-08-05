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
// Warning 6328: (140-160): Assertion violation happens here
// Warning 8364: (132-133): Assertion checker does not yet implement type type(enum C.D)
// Warning 5084: (132-136): Type conversion is not yet fully supported and might yield false positives.
