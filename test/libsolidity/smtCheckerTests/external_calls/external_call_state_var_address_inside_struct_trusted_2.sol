contract D {
	uint public x;
}

contract C {
	struct S {
		address d;
	}
	S s;
	constructor() {
		s.d = address(new D());
		assert(D(s.d).x() == 0); // should hold
	}
	function f() public view {
		assert(D(s.d).x() == 0); // should hold, but fails because we havoc the state
	}
}
// ====
// SMTEngine: chc
// SMTExtCalls: trusted
// SMTTargets: assert
// ----
// Warning 6328: (197-220): CHC: Assertion violation happens here.
// Info 1391: CHC: 1 verification condition(s) proved safe! Enable the model checker option "show proved safe" to see all of them.
