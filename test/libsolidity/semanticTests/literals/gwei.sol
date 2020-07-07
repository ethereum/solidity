contract C {
	uint constant gwei = 1 gwei;

	function f() public view returns(uint) { return gwei; }
}
// ====
// compileViaYul: also
// ----
// f() -> 1000000000

