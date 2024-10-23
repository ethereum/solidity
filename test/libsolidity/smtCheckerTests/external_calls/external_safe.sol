abstract contract D {
	function d() external virtual;
}

contract C {
	uint x;
	D d;
	function f() public {
		if (x < 5)
			++x;
	}
	function g() public {
		d.d();
		assert(x < 6);
	}
}
// ====
// SMTEngine: all
// SMTTargets: assert
// SMTIgnoreOS: linux
// ----
// Info 1391: CHC: 1 verification condition(s) proved safe! Enable the model checker option "show proved safe" to see all of them.
