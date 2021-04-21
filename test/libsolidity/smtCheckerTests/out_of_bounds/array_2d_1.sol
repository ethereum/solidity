contract C {
	uint[][] a;
	function p() public { a.push(); }
	function q(uint i) public {
		require(i < a.length);
		a[i].push();
	}
	function r(uint i, uint j) public view returns (uint) {
		require(i < a.length);
		require(j < a[i].length);
		return a[i][j]; // safe access
	}
}
// ====
// SMTEngine: all
// ----
