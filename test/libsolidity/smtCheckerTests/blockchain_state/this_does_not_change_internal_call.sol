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
// Info 1391: CHC: 2 verification condition(s) proved safe! Enable the model checker option "show proved safe" to see all of them.
