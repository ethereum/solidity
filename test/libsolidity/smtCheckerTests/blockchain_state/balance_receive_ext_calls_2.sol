contract C {
	function f(address _a) public {
		uint x = address(this).balance;
		_a.call("");
		assert(address(this).balance == x); // should fail
		assert(address(this).balance >= x); // should hold
	}
}
// ====
// SMTEngine: all
// ----
// Warning 9302: (82-93): Return value of low-level calls not used.
// Warning 6328: (97-131): CHC: Assertion violation happens here.\nCounterexample:\n\n_a = 0\nx = 5921\n\nTransaction trace:\nC.constructor()\nC.f(0)\n    _a.call("") -- untrusted external call, synthesized as:\n        C.f(0) -- reentrant call\n            _a.call("") -- untrusted external call
