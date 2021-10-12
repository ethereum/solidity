contract D {
	uint public x;
}

contract C {
	struct S {
		D d;
	}
	struct T {
		S s;
	}
	T t;
	constructor() {
		t.s.d = new D();
		assert(t.s.d.x() == 0); // should hold
	}
	function f() public view {
		assert(t.s.d.x() == 0); // should hold, but fails because we havoc the state
	}
}
// ====
// SMTEngine: chc
// SMTExtCalls: trusted
// SMTTargets: assert
// SMTIgnoreCex: yes
// ----
// Warning 6328: (205-227): CHC: Assertion violation happens here.
