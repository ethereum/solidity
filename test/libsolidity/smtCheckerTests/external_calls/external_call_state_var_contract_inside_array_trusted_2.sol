contract D {
	uint public x;
}

contract C {
	D[] ds;
	constructor() {
		ds.push(new D());
		assert(ds[0].x() == 0); // should hold
	}
	function f() public view {
		assert(ds[0].x() == 0); // should hold, but fails because we havoc the state
	}
}
// ====
// SMTEngine: chc
// SMTExtCalls: trusted
// SMTTargets: assert
// SMTIgnoreCex: yes
// ----
// Warning 6328: (165-187): CHC: Assertion violation happens here.
