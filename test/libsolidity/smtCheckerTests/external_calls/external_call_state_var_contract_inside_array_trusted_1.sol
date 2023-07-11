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
// Warning 6328: (208-230): CHC: Assertion violation happens here.
// Info 1391: CHC: 1 verification condition(s) proved safe! Enable the model checker option "show proved safe" to see all of them.
