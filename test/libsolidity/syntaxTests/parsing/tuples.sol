contract C {
	function f() {
		uint a = (1);
		(uint b,) = (1,);
		(uint c, uint d) = (1, 2 + a);
		(uint e,) = (1, 2, b);
		(a) = 3;
	}
}
// ----
// Warning: (47-63): Different number of components on the left hand side (2) than on the right hand side (1).
// Warning: (100-121): Different number of components on the left hand side (2) than on the right hand side (3).
// Warning: (14-136): No visibility specified. Defaulting to "public". 
// Warning: (68-74): Unused local variable.
// Warning: (76-82): Unused local variable.
// Warning: (101-107): Unused local variable.
// Warning: (14-136): Function state mutability can be restricted to pure
