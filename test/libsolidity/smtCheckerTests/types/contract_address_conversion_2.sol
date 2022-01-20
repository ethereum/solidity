contract C
{
	function f(C c, C d) public pure {
		assert(address(c) == address(c));
		address a = address(c);
		require(address(c) == address(d));
		assert(a == address(d));
	}
}
// ====
// SMTEngine: all
// ----
