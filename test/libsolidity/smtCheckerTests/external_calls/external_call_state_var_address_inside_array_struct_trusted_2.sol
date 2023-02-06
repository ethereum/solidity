contract D {
	uint public x;
	function setD(uint _x) public { x = _x; }
}

contract C {
	struct S {
		address d;
	}
	S[] ss;
	constructor() {
		ss.push(S(address(new D())));
		assert(D(ss[0].d).x() == 0); // should hold
	}
	function f() public view {
		assert(D(ss[0].d).x() == 0); // should fail
	}
}
// ====
// SMTEngine: chc
// SMTExtCalls: trusted
// SMTTargets: assert
// ----
// Warning 6328: (253-280): CHC: Assertion violation happens here.\nCounterexample:\nss = [{d: 0x4706}]\n\nTransaction trace:\nC.constructor()\nState: ss = [{d: 0x4706}]\nC.f()
