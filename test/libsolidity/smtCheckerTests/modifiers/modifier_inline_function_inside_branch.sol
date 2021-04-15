contract C
{
	address owner;
	modifier m {
		if (true)
			owner = g();
		_;
	}
	function f() m public {
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
