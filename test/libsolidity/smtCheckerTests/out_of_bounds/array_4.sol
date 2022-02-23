contract C {
	uint[] a;
	function p() public { a.push(); }
	function r(uint i) public view returns (uint) {
		require(i < a.length);
		return a[i]; // safe access
	}
}
// ====
// SMTEngine: all
// ----
