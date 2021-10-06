contract C {
	function g() external {
		f();
	}

	function h() external payable {
		f();
	}

	function f() internal {
		require(msg.value == 0);
	}
}
// ====
// SMTEngine: all
