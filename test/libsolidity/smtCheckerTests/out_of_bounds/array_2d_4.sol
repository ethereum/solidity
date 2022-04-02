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
// Warning 1218: (228-232='a[i]'): CHC: Error trying to invoke SMT solver.
// Warning 1218: (255-259='a[i]'): CHC: Error trying to invoke SMT solver.
// Warning 1218: (255-262='a[i][j]'): CHC: Error trying to invoke SMT solver.
// Warning 4984: (184-197='a.length + 10'): CHC: Overflow (resulting value larger than 2**256 - 1) might happen here.
// Warning 4984: (199-202='++i'): CHC: Overflow (resulting value larger than 2**256 - 1) might happen here.
// Warning 6368: (228-232='a[i]'): CHC: Out of bounds access might happen here.
// Warning 4984: (228-244='a[i].length + 20'): CHC: Overflow (resulting value larger than 2**256 - 1) might happen here.
// Warning 4984: (246-249='++j'): CHC: Overflow (resulting value larger than 2**256 - 1) might happen here.
// Warning 6368: (255-259='a[i]'): CHC: Out of bounds access might happen here.
// Warning 6368: (255-262='a[i][j]'): CHC: Out of bounds access might happen here.
// Warning 2661: (184-197='a.length + 10'): BMC: Overflow (resulting value larger than 2**256 - 1) happens here.
// Warning 2661: (228-244='a[i].length + 20'): BMC: Overflow (resulting value larger than 2**256 - 1) happens here.
