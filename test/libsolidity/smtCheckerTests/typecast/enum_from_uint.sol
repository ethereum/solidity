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
// Warning: (132-136): Type conversion is not yet fully supported and might yield false positives.
// Warning: (140-160): Assertion violation happens here
