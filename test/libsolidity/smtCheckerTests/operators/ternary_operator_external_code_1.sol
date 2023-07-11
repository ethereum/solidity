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
		guard = true;
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
// Info 1391: CHC: 1 verification condition(s) proved safe! Enable the model checker option "show proved safe" to see all of them.
