contract D {
	uint public x;
	function setD(uint _x) public { x = _x; }
}

contract C {
	address[] ds;
	constructor() {
		ds.push(address(new D()));
		assert(D(ds[0]).x() == 0); // should hold
	}
	function f() public view {
		assert(D(ds[0]).x() == 0); // should fail
	}
}
// ====
// SMTEngine: chc
// SMTExtCalls: trusted
// SMTTargets: assert
// ----
// Warning 6328: (226-251): CHC: Assertion violation happens here.\nCounterexample:\nds = [0x0]\n\nTransaction trace:\nC.constructor()\nState: ds = [0x0]\nC.f()
