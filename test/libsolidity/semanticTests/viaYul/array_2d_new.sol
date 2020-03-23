contract C {
	function f(uint n) public pure returns (uint) {
		uint[][] memory a = new uint[][](2);
		for (uint i = 0; i < 2; ++i)
			a[i] = new uint[](3);
		return a[0][0] = n;
	}
}
// ====
// compileViaYul: also
// ----
// f(uint256): 42 -> 42
