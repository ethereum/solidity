contract C {
	function f() public pure returns (uint, uint, uint, uint) {
		// Can later be replaced by (uint a, uint b,) = f();
		var (a,b,) = f();
		a; b;
	}
}
// ----
// Warning: (136-137): Use of the "var" keyword is deprecated.
// Warning: (138-139): Use of the "var" keyword is deprecated.
// Warning: (131-147): Different number of components on the left hand side (3) than on the right hand side (4).
