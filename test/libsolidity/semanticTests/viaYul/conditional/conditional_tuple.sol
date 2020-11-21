contract A {
	function f(bool cond) public pure returns (uint, uint) {
		(uint a, uint b) = cond ? (1, 2) : (3, 4);
		return (a, b);
	}
}
// ====
// compileViaYul: also
// compileToEwasm: also
// ----
// f(bool): true -> 1, 2
// f(bool): false -> 3, 4
