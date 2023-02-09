abstract contract D {
	function d() external virtual;
}

contract C {
	address t;
	constructor() {
		t = address(this);
	}
	function f(D d) public {
		address a = address(this);
		d.d();
		assert(address(this) == t);
		assert(a == t);
	}
}
// ====
// SMTEngine: all
// SMTIgnoreInv: yes
// SMTIgnoreOS: macos
// ----
// Info 1391: CHC: 2 verification condition(s) proved safe! Enable the model checker option "show proved safe" to see all of them.
