contract C {
	address t;
	constructor() {
		t = address(this);
	}
	function f() public view {
		g(address(this));
	}
	function g(address a) internal view {
		assert(a == t);
		assert(a == address(this));
	}
}
// ====
// SMTEngine: all
// ----
// Info 1180: Contract invariant(s) for :C:\n(((t + ((- 1) * address(this))) >= 0) && ((t + ((- 1) * address(this))) <= 0))\n
