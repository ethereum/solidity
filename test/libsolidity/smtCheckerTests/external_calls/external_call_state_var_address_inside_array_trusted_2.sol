contract D {
	uint public x;
}

contract C {
	address[] ds;
	constructor() {
		ds.push(address(new D()));
		assert(D(ds[0]).x() == 0); // should hold
	}
	function f() public view {
		assert(D(ds[0]).x() == 0); // should hold, but fails because we havoc the state
	}
}
// ====
// SMTEngine: chc
// SMTExtCalls: trusted
// SMTTargets: assert
// ----
// Warning 6328: (183-208): CHC: Assertion violation happens here.
// Info 1391: CHC: 1 verification condition(s) proved safe! Enable the model checker option "show proved safe" to see all of them.
