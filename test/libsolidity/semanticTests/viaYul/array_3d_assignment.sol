contract C {
	function f(uint n) public pure returns (uint) {
		uint[][][] memory a = new uint[][][](2);
		for (uint i = 0; i < 2; ++i)
		{
			a[i] = new uint[][](3);
			for (uint j = 0; j < 3; ++j)
				a[i][j] = new uint[](4);
		}
		a[1][1][1] = n;
		uint[][] memory b = a[1];
		uint[] memory c = b[1];
		return c[1];
	}
}
// ====
// compileViaYul: also
// ----
// f(uint256): 42 -> 42
