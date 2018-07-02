contract C {
	function f() public {
		uint a = (1);
		(uint b,) = (1,);
		(uint c, uint d) = (1, 2 + a);
		(uint e,) = (1, 2, b);
		(a) = 3;
	}
}
// ----
// Warning: (54-70): Different number of components on the left hand side (2) than on the right hand side (1).
// Warning: (107-128): Different number of components on the left hand side (2) than on the right hand side (3).
// Warning: (75-81): Unused local variable.
// Warning: (83-89): Unused local variable.
// Warning: (108-114): Unused local variable.
// Warning: (14-143): Function state mutability can be restricted to pure
