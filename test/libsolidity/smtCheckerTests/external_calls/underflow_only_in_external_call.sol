contract C {

	uint v;
	bool guard = true;

	function dec() public returns (uint) {
		if (guard) return 0;
		--v;
		return v;
	}

	function f() public returns (uint) {
		guard = false;
		uint ret = this.dec();
		guard = true;
		return ret;
	}
}
// ====
// SMTEngine: chc
// SMTIgnoreCex: no
// SMTTargets: underflow
// ----
// Warning 3944: (109-112): CHC: Underflow (resulting value less than 0) happens here.\nCounterexample:\nv = 0, guard = false\n = 0\n\nTransaction trace:\nC.constructor()\nState: v = 0, guard = true\nC.f()\n    C.dec() -- trusted external call
