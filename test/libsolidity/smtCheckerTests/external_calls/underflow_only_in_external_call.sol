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
// SMTTargets: underflow
// ----
// Warning 3944: (109-112): CHC: Underflow (resulting value less than 0) happens here.
