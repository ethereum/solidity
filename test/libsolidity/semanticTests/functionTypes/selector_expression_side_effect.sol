contract C {
	uint x;
	function f() public returns (uint256) {
		h().f.selector;
		return x;
	}
	function h() public returns (C) {
		x = 42;
		return this;
	}
}
// ----
// f() -> 42
