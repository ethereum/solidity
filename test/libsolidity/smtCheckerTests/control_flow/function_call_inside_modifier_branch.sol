contract C
{
	modifier m(address a) {
		if (true) {
			a = g();
			_;
			assert(a == address(0));
		}
	}

	function f(address a) m(a) public pure {
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
