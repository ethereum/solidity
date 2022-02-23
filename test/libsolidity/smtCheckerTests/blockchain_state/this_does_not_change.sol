contract C {
	address t;
	constructor() {
		t = address(this);
	}
	function inv() public view {
		assert(address(this) == t);
	}
}
// ====
// SMTEngine: all
// SMTIgnoreOS: macos
// ----
// Info 1180: Contract invariant(s) for :C:\n(((address(this) + ((- 1) * t)) <= 0) && ((address(this) + ((- 1) * t)) >= 0))\n
