abstract contract D {
	function d() virtual public {}
}

contract A {
	int x = 0;

	function f() virtual public view {
		assert(x == 0); // should hold
		assert(x == 1); // should fail
	}
}
contract C is A {
	constructor() {
		x = 1;
	}

	function call(D d) public {
		d.d();
	}

	function f() public view override {
		assert(x == 1); // should hold
		assert(x == 0); // should fail
	}
}
// ====
// SMTEngine: chc
// SMTIgnoreCex: yes
// SMTIgnoreInv: yes
// SMTSolvers: eld
// ----
// Warning 6328: (154-168): CHC: Assertion violation happens here.
// Warning 6328: (352-366): CHC: Assertion violation happens here.
// Info 1391: CHC: 2 verification condition(s) proved safe! Enable the model checker option "show proved safe" to see all of them.
