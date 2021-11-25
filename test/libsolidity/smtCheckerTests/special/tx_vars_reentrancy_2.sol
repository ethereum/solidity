interface I {
	function f() external payable;
}

contract C {
	function g(I _i) public payable {
		uint x = address(this).balance;
		_i.f{ value: 100 }();
		assert(x == address(this).balance); // should fail
	}
}
// ====
// SMTEngine: all
// SMTIgnoreOS: macos
// ----
// Warning 6328: (157-191): CHC: Assertion violation happens here.\nCounterexample:\n\n_i = 0\nx = 101\n\nTransaction trace:\nC.constructor()\nC.g(0){ msg.value: 69 }\n    _i.f{ value: 100 }() -- untrusted external call
