contract D {
	uint x;
}

contract C {
	function f() public {
		D d1 = new D();
		D d2 = new D();

		assert(d1 != d2); // should hold in ext calls trusted mode
		assert(address(this) != address(d1)); // should hold in ext calls trusted mode
		assert(address(this) != address(d2)); // should hold in ext calls trusted mode
	}
}
// ====
// SMTEngine: all
// SMTExtCalls: trusted
// ----
// Info 1391: CHC: 3 verification condition(s) proved safe! Enable the model checker option "show proved safe" to see all of them.
