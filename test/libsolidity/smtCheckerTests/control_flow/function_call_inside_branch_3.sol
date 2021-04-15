contract C
{
	function f() public pure {
		if (true) {
			address a = g();
			assert(a == address(0));
		}
		if (true) {
			address a = g();
			assert(a == address(0));
		}

	}
	function g() public pure returns (address) {
		address a;
		a = address(0);
		return a;
	}
}
// ====
// SMTEngine: all
// ----
