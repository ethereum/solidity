abstract contract D {
	function d() external virtual returns (uint);
}

contract C {

	D d;
	uint v;
	bool guard = true;

	function inc() public {
		++v;
	}

	function dec() public {
		if (guard) return;
		--v;
	}

	function f() public returns (uint) {
		guard = false;
		uint ret = v > 0 ? d.d() : 0;
		guard = true;
		return ret;
	}
}
// ====
// SMTEngine: chc
// SMTTargets: underflow
// ----
// Warning 3944: (206-209): CHC: Underflow (resulting value less than 0) happens here.
