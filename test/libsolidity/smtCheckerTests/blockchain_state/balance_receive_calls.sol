contract C {
	bool once;
	function f() public payable {
		require(!once);
		once = true;
		require(msg.value == 10);
		assert(address(this).balance >= 10); // should hold
		assert(address(this).balance >= 20); // should fail
		g();
	}
	function g() internal view {
		assert(address(this).balance >= 10); // should hold
		assert(address(this).balance >= 20); // should fail
		h();
	}
	function h() internal view {
		assert(address(this).balance >= 10); // should hold
		assert(address(this).balance >= 20); // should fail
	}
}
// ====
// SMTEngine: all
// SMTIgnoreOS: macos
// ----
// Warning 6328: (173-208): CHC: Assertion violation happens here.\nCounterexample:\nonce = true\n\nTransaction trace:\nC.constructor()\nState: once = false\nC.f(){ msg.value: 10 }
// Warning 6328: (321-356): CHC: Assertion violation happens here.\nCounterexample:\nonce = true\n\nTransaction trace:\nC.constructor()\nState: once = false\nC.f(){ msg.value: 10 }\n    C.g() -- internal call
// Warning 6328: (469-504): CHC: Assertion violation happens here.\nCounterexample:\nonce = true\n\nTransaction trace:\nC.constructor()\nState: once = false\nC.f(){ msg.value: 10 }\n    C.g() -- internal call\n        C.h() -- internal call
// Info 1180: Contract invariant(s) for :C:\n((:var 1).balances[address(this)] >= 0)\nonce\n
