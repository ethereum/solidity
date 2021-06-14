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
// ----
// Warning 0: (57-239): Contract invariants for :C:\n((!((address(this) + ((- 1) * t)) = 0) || (<errorCode> <= 0)) && (!((address(this) + ((- 1) * t)) >= 0) || ((t' + ((- 1) * address(this))) <= 0)) && (!((address(this) + ((- 1) * t)) <= 0) || ((address(this) + ((- 1) * t')) <= 0)))\n((!(<errorCode> >= 2) || !((address(this) + ((- 1) * t)) = 0)) && (!((address(this) + ((- 1) * t)) >= 0) || ((t' + ((- 1) * address(this))) <= 0)) && (((address(this) + ((- 1) * t')) <= 0) || !((address(this) + ((- 1) * t)) <= 0)))\n(((t + ((- 1) * address(this))) >= 0) && ((t + ((- 1) * address(this))) <= 0))\n
