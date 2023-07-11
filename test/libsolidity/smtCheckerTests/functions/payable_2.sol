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
// Warning 6328: (261-283): CHC: Assertion violation happens here.
// Info 1391: CHC: 1 verification condition(s) proved safe! Enable the model checker option "show proved safe" to see all of them.
