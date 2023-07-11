contract D {
	constructor(uint _x) { x = _x; }
	uint public x;
}

contract E {
	constructor() { x = 2; }
	uint public x;
}

contract C {
	function f() public {
		address d = address(new D(42));
		assert(D(d).x() == 42); // should hold
		assert(D(d).x() == 43); // should fail
		uint y = E(d).x();
		assert(y == 2); // should fail, it would still call D.x() == 42
		assert(y == 42); // should hold, but fails due to false positive
	}
}
// ====
// SMTEngine: chc
// SMTExtCalls: trusted
// SMTIgnoreCex: yes
// ----
// Warning 6328: (237-259): CHC: Assertion violation happens here.
// Warning 6328: (299-313): CHC: Assertion violation happens here.
// Warning 6328: (365-380): CHC: Assertion violation happens here.
// Info 1391: CHC: 1 verification condition(s) proved safe! Enable the model checker option "show proved safe" to see all of them.
