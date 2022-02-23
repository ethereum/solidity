contract C
{
	function f(address a) public pure {
		require(a != address(0));
		assert(a != address(0));
	}
}
// ====
// SMTEngine: all
// ----
