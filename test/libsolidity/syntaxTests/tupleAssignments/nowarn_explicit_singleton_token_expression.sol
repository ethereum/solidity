contract C {
	function f() public pure {
		uint a;
		(a,) = (uint(1),);
	}
}
// ----
// Warning: (53-70): Different number of components on the left hand side (2) than on the right hand side (1).
