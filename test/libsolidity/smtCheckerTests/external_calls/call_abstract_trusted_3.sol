contract D {
	constructor(uint _x) { x = _x; }
	function setD(uint _x) public { x = _x; }
	uint public x;
}

contract C {
	uint x;

	function f() public {
		x = 666;
		address d = address(new D(42));
		assert(D(d).x() == 42); // should hold
		assert(D(d).x() == 21); // should fail
		d.call(abi.encodeCall(D.setD, (21)));
		assert(D(d).x() == 21); // should hold, but false positive cus low level calls are not handled precisely
		assert(D(d).x() == 42); // should fail
		assert(x == 666); // should hold, C's storage should not have been havoced
	}
}
// ====
// SMTEngine: chc
// SMTExtCalls: trusted
// SMTIgnoreCex: yes
// ----
// Warning 9302: (284-320): Return value of low-level calls not used.
// Warning 6031: (306-312): Internal error: Expression undefined for SMT solver.
// Warning 6328: (243-265): CHC: Assertion violation happens here.
// Warning 6328: (324-346): CHC: Assertion violation happens here.
// Warning 6328: (431-453): CHC: Assertion violation happens here.
// Info 1180: Reentrancy property(ies) for :C:\n!(<errorCode> = 1)\n((((x' + ((- 1) * x)) = 0) || !(x' <= 665)) && (!(x' >= 667) || ((x' + ((- 1) * x)) = 0)) && !(<errorCode> >= 6))\n<errorCode> = 0 -> no errors\n<errorCode> = 1 -> Assertion failed at assert(D(d).x() == 42)\n<errorCode> = 2 -> Assertion failed at assert(D(d).x() == 21)\n<errorCode> = 4 -> Assertion failed at assert(D(d).x() == 21)\n<errorCode> = 5 -> Assertion failed at assert(D(d).x() == 42)\n<errorCode> = 6 -> Assertion failed at assert(x == 666)\n
