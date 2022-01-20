contract C
{
	function f(C c, C d, C e) public pure {
		require(address(c) == address(d));
		require(address(d) == address(e));
		assert(address(c) == address(e));
	}
}
// ====
// SMTEngine: all
// ----
