contract C
{
	function f(bytes memory b) public pure returns (bytes memory) {
		bytes memory c = b;
		return b;
	}
}
// ====
// SMTEngine: all
// ----
// Warning 2072: (80-94): Unused local variable.
