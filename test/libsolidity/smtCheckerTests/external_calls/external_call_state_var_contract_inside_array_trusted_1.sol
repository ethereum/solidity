contract D {
	uint public x;
	function setD(uint _x) public { x = _x; }
}

contract C {
	D[] ds;
	constructor() {
		ds.push(new D());
		assert(ds[0].x() == 0); // should hold
	}
	function f() public view {
		assert(ds[0].x() == 0); // should fail
	}
}
// ====
// SMTEngine: chc
// SMTExtCalls: trusted
// SMTTargets: assert
// ----
// Warning 6328: (208-230): CHC: Assertion violation happens here.\nCounterexample:\nds = [37]\n\nTransaction trace:\nC.constructor()\nState: ds = [37]\nC.f()
