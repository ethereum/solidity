contract C {
	error test();
	function f() public {
		revert test();
	}
}
// ----
// f() -> FAILURE, hex"f8a8fd6d"
