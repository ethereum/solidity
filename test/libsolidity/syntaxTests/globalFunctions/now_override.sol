contract C {
	function f() public view {
		uint now = block.timestamp;
		now;
	}
}
// ----
// Warning 2319: (43-51): This declaration shadows a builtin symbol.
