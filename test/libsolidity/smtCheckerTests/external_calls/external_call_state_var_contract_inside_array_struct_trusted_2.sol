contract D {
	uint public x;
	function setD(uint _x) public { x = _x; }
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
		assert(ss[0].d.x() == 0); // should fail
	}
}
// ====
// SMTEngine: chc
// SMTExtCalls: trusted
// SMTTargets: assert
// ----
// Warning 6328: (235-259): CHC: Assertion violation happens here.\nCounterexample:\nss = [{d: 3}]\n\nTransaction trace:\nC.constructor()\nState: ss = [{d: 3}]\nC.f()
