contract C {
	function f() public view {
		uint now = block.timestamp;
		now;
	}
}
// ----
// Warning: (43-51): This declaration shadows a builtin symbol.
