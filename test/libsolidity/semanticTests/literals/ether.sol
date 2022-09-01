contract C {
	uint constant x = 1 ether;

	function f() public view returns(uint) { return x; }
}
// ====
// compileToEwasm: also
// ----
// f() -> 1000000000000000000
