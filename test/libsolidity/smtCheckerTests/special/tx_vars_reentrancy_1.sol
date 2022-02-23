interface I {
	function f() external;
}

contract C {
	function g(I _i) public payable {
		uint x = address(this).balance;
		_i.f();
		assert(x == address(this).balance); // should fail
	}
}
// ====
// SMTEngine: all
// SMTIgnoreOS: macos
// ----
// Warning 6328: (135-169): CHC: Assertion violation happens here.\nCounterexample:\n\n_i = 0\nx = 2997\n\nTransaction trace:\nC.constructor()\nC.g(0){ msg.value: 2803 }\n    _i.f() -- untrusted external call, synthesized as:\n        C.g(0){ msg.value: 2446 } -- reentrant call\n            _i.f() -- untrusted external call
