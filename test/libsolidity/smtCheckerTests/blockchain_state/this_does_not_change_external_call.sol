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
// SMTIgnoreOS: macos
// ----
// Info 1180: Contract invariant(s) for :C:\n(((address(this) + ((- 1) * t)) <= 0) && ((address(this) + ((- 1) * t)) >= 0))\nReentrancy property(ies) for :C:\n((!((t + ((- 1) * address(this))) = 0) || (<errorCode> <= 0)) && (!((t + ((- 1) * address(this))) <= 0) || ((t' + ((- 1) * address(this))) <= 0)) && (!((t + ((- 1) * address(this))) >= 0) || ((address(this) + ((- 1) * t')) <= 0)))\n((!(<errorCode> >= 2) || !((t + ((- 1) * address(this))) = 0)) && (!((t + ((- 1) * address(this))) <= 0) || ((t' + ((- 1) * address(this))) <= 0)) && (!((t + ((- 1) * address(this))) >= 0) || ((address(this) + ((- 1) * t')) <= 0)))\n<errorCode> = 0 -> no errors\n<errorCode> = 1 -> Assertion failed at assert(address(this) == t)\n<errorCode> = 2 -> Assertion failed at assert(a == t)\n
