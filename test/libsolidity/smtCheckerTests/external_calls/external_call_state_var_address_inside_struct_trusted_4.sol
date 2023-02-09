contract D {
	uint public x;
	function setD(uint _x) public { x = _x; }
}

contract C {
	struct S {
		address d;
	}
	struct T {
		S s;
	}
	T t;
	constructor() {
		t.s.d = address(new D());
		assert(D(t.s.d).x() == 0); // should hold
	}
	function f() public view {
		assert(D(t.s.d).x() == 0); // should fail
	}
}
// ====
// SMTEngine: chc
// SMTExtCalls: trusted
// SMTTargets: assert
// ----
// Warning 6328: (266-291): CHC: Assertion violation happens here.
// Info 1391: CHC: 1 verification condition(s) proved safe! Enable the model checker option "show proved safe" to see all of them.
