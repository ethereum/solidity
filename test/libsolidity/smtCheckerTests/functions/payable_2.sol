contract C {
	function g() external {
		f();
		h();
		i();
	}

	function g2() external payable {
		i();
	}

	function f() internal {
		require(msg.value == 0);
	}

	function h() internal {
		assert(msg.value == 0); // should hold
	}

	function i() internal {
		assert(msg.value == 0); // should fail
	}
}
// ====
// SMTEngine: all
// SMTIgnoreOS: macos
// ----
// Warning 6328: (261-283): CHC: Assertion violation happens here.\nCounterexample:\n\n\nTransaction trace:\nC.constructor()\nC.g2(){ msg.value: 3529 }\n    C.i() -- internal call
