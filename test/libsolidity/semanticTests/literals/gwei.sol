contract C {
	uint constant x = 1 gwei;

	function f() public view returns(uint) { return x; }
}
// ====
// compileViaYul: also
// compileToEwasm: also
// ----
// f() -> 1000000000
