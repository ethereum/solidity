contract C {
	uint[][] a;
	function p() public { a.push(); }
	function q(uint i) public {
		require(i < a.length);
		a[i].push();
	}
	function r() public view {
		for (uint i = 0; i < a.length + 10; ++i)
			for (uint j = 0; j < a[i].length + 20; ++j)
				a[i][j]; // oob access
	}
}
// ====
// SMTEngine: all
// ----
// Warning 4984: (184-197): CHC: Overflow (resulting value larger than 2**256 - 1) might happen here.
// Warning 4984: (199-202): CHC: Overflow (resulting value larger than 2**256 - 1) might happen here.
// Warning 6368: (228-232): CHC: Out of bounds access happens here.
// Warning 4984: (228-244): CHC: Overflow (resulting value larger than 2**256 - 1) might happen here.
// Warning 4984: (246-249): CHC: Overflow (resulting value larger than 2**256 - 1) might happen here.
// Warning 6368: (255-259): CHC: Out of bounds access happens here.
// Warning 6368: (255-262): CHC: Out of bounds access happens here.
// Info 1391: CHC: 1 verification condition(s) proved safe! Enable the model checker option "show proved safe" to see all of them.
// Warning 2661: (184-197): BMC: Overflow (resulting value larger than 2**256 - 1) happens here.
// Warning 2661: (228-244): BMC: Overflow (resulting value larger than 2**256 - 1) happens here.
// Info 6002: BMC: 2 verification condition(s) proved safe! Enable the model checker option "show proved safe" to see all of them.
