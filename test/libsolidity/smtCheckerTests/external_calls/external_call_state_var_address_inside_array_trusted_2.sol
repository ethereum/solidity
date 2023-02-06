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
// Warning 6328: (183-208): CHC: Assertion violation happens here.\nCounterexample:\nds = [0x25]\n\nTransaction trace:\nC.constructor()\nState: ds = [0x25]\nC.f()
