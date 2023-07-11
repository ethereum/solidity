contract C {
	uint[][] a;
	function p() public { a.push(); }
	function q(uint i) public {
		require(i < a.length);
		a[i].push();
	}
	function r(uint i, uint j) public view returns (uint) {
		require(i < a.length);
		return a[i][j]; // unsafe access
	}
}
// ====
// SMTEngine: all
// ----
// Warning 6368: (224-231): CHC: Out of bounds access happens here.
// Info 1391: CHC: 2 verification condition(s) proved safe! Enable the model checker option "show proved safe" to see all of them.
