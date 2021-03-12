contract C {
	uint[][] a;
	function p() public { a.push(); }
	function q(uint i) public {
		require(i < a.length);
		a[i].push();
	}
	function r() public view {
		for (uint i = 0; i < a.length; ++i)
			for (uint j = 0; j < a[i].length; ++j)
				a[i][j]; // safe access
	}
}
// ====
// SMTEngine: all
// ----
