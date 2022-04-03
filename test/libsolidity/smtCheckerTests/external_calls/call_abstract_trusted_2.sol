contract D {
	constructor(uint _x) { x = _x; }
	function setD(uint _x) public { x = _x; }
	uint public x;
}

contract C {
	function f() public {
		address d = address(new D(42));
		assert(D(d).x() == 42); // should hold
		assert(D(d).x() == 21); // should fail
		d.call(abi.encodeCall(D.setD, (21)));
		assert(D(d).x() == 21); // should hold, but false positive cus low level calls are not handled precisely
		assert(D(d).x() == 42); // should fail
	}
}
// ====
// SMTEngine: chc
// SMTExtCalls: trusted
// SMTIgnoreCex: yes
// ----
// Warning 9302: (263-299): Return value of low-level calls not used.
// Warning 6031: (285-291): Internal error: Expression undefined for SMT solver.
// Warning 6328: (222-244): CHC: Assertion violation happens here.
// Warning 6328: (303-325): CHC: Assertion violation happens here.
// Warning 6328: (410-432): CHC: Assertion violation happens here.
// Info 1180: Reentrancy property(ies) for :C:\n!(<errorCode> = 1)\n<errorCode> = 0 -> no errors\n<errorCode> = 1 -> Assertion failed at assert(D(d).x() == 42)\n<errorCode> = 2 -> Assertion failed at assert(D(d).x() == 21)\n<errorCode> = 4 -> Assertion failed at assert(D(d).x() == 21)\n<errorCode> = 5 -> Assertion failed at assert(D(d).x() == 42)\n
