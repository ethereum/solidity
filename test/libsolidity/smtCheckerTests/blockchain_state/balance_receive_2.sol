contract C {
	uint x;
	bool once;
	constructor() payable {
		x = address(this).balance;
	}
	function f() public payable {
		require(!once);
		once = true;
		require(msg.value > 0);
		assert(address(this).balance > x); // should hold
		assert(address(this).balance > x + 10); // should fail
	}
}
// ====
// SMTEngine: all
// ----
// Warning 4984: (266-272): CHC: Overflow (resulting value larger than 2**256 - 1) happens here.\nCounterexample:\nx = 115792089237316195423570985008687907853269984665640564039457584007913129639926, once = true\n\nTransaction trace:\nC.constructor(){ msg.value: 28100 }\nState: x = 115792089237316195423570985008687907853269984665640564039457584007913129639926, once = false\nC.f(){ msg.value: 8 }
// Warning 6328: (235-273): CHC: Assertion violation happens here.\nCounterexample:\nx = 0, once = true\n\nTransaction trace:\nC.constructor(){ msg.value: 0 }\nState: x = 0, once = false\nC.f(){ msg.value: 8 }
// Info 1180: Contract invariant(s) for :C:\nonce\n
