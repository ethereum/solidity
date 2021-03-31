contract C
{
	function f() public pure {
		if (true) {
		} else {
			address a = g();
			assert(a == address(0));
		}
	}
	function g() public pure returns (address) {
		address x;
		x = address(0);
		return x;
	}
}
// ====
// SMTEngine: all
// ----
