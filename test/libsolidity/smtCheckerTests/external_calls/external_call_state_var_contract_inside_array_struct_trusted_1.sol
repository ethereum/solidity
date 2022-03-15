contract D {
	uint public x;
}

contract C {
	struct S {
		D d;
	}
	S[] ss;
	constructor() {
		ss.push(S(new D()));
		assert(ss[0].d.x() == 0); // should hold
	}
	function f() public view {
		assert(ss[0].d.x() == 0); // should hold, but fails because we havoc the state
	}
}
// ====
// SMTEngine: chc
// SMTExtCalls: trusted
// SMTTargets: assert
// ----
// Warning 6328: (192-216): CHC: Assertion violation happens here.\nCounterexample:\nss = [{d: 3}]\n\nTransaction trace:\nC.constructor()\nState: ss = [{d: 3}]\nC.f()
