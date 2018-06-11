contract C {
	function f() public pure returns (uint, uint, uint, uint) {
		(uint a, uint b,) = f();
		a; b;
	}
}
// ----
// Warning: (76-99): Different number of components on the left hand side (3) than on the right hand side (4).
