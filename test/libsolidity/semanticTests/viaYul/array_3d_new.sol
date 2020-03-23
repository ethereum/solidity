contract C {
	function f(uint n) public pure returns (uint) {
		uint[][][] memory a = new uint[][][](2);
		for (uint i = 0; i < 2; ++i)
		{
			a[i] = new uint[][](3);
			for (uint j = 0; j < 3; ++j)
				a[i][j] = new uint[](4);
		}
		return a[1][1][1] = n;
	}
}
// ====
// compileViaYul: also
// ----
// f(uint256): 42 -> 42
