contract C
{
	enum D { Left, Right }
	function f(uint x) public pure {
		require(x == 0);
		D _a = D(x);
		assert(_a == D.Left);
	}
}
// ====
// SMTEngine: all
// ----
